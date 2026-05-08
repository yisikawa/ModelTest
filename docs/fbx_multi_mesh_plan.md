# FBX マルチメッシュ出力 実装企画書

## 方針まとめ

| 軸 | 方針 |
|---|---|
| OptimizeVertices | CMesh 単位のローカルコンテキストに分離 |
| ボーン | createFBXBoneNodes を1回だけ実行し、スキン付けはメッシュごとに attachFBXSkinToMesh |
| メッシュ-マテリアル | 全マテリアルを各 FbxNode に同順で AddMaterial → `pStream->m_texNo` をそのまま polygon material index として使用 |

---

## Phase 1 — MeshContext 構造体の定義

**対象ファイル:** `Model.h` または `SaveFile.cpp` 先頭

```cpp
struct MeshContext {
    std::vector<int>           vertexRemap;    // [localVertexIdx] -> weldedIdx (0-based per mesh)
    std::vector<WELDED_VERTEX> weldedVertices; // このメッシュ専用のユニーク頂点
};
```

クラスメンバの `m_vertexRemap` / `m_weldedVertices` は .x 形式出力（`saveX` → `outputVertex` → `OptimizeVertices(false)`）で引き続き使うので **削除しない**。

---

## Phase 2 — OptimizeVerticesForMesh の実装

**現状の問題:** `SaveFile.cpp:54-216` の `OptimizeVertices` は全 CMesh をループして `m_vertexRemap`（全メッシュ連番）に書く。

**新関数:**

```cpp
void CModel::OptimizeVerticesForMesh(CMesh* pMesh, MeshContext& ctx, bool enable);
```

現行の Phase 1 / 2 / 3 をそのまま使うが、スコープを以下に変更:
- `ctx.vertexRemap.assign(pMesh->m_NumVertices, -1)` — vCnt 不要、0-based
- `ctx.weldedVertices` に書く
- Phase 2 の面ループも `pMesh->m_Streams` のみ（vCnt = 0）

既存の `OptimizeVertices` は **変更しない**。

---

## Phase 3 — per-mesh 頂点・面出力関数

**参考:** `SaveFile.cpp:218` / `SaveFile.cpp:249` を元に新規追加

```cpp
bool CModel::outputFBXVertexForMesh(FbxMesh* pfbxMesh, const MeshContext& ctx);
```

- 既存 `outputFBXVertex` と同じ変換行列・法線/UV設定
- ControlPoints のソースを `ctx.weldedVertices` に変更
- `OptimizeVerticesForMesh` の呼び出しは行わない（saveFBX 側が担う）

```cpp
bool CModel::outputFBXFaceForMesh(FbxMesh* pfbxMesh, CMesh* pMesh,
                                   FbxLayerElementMaterial* pMatElem,
                                   const MeshContext& ctx);
```

- 対象は `pMesh` の `m_Streams` のみ
- `ctx.vertexRemap[i]`（vCnt = 0）でインデックス変換
- `pStream->m_texNo` はそのまま `pMatElem->GetIndexArray().Add()` に渡す

---

## Phase 4 — ボーン関連の分割

**現状:** `SaveFile.cpp:393-452` の `outputFBXBone` がボーンノード生成・BindPose・FbxSkin・FbxCluster を一体で行っている。

### createFBXBoneNodes（ボーン生成 — 1回だけ）

```cpp
bool CModel::createFBXBoneNodes(FbxNode* pRootNode, FbxScene* pScene,
                                 FbxNode*& outRootBoneNode,
                                 std::vector<FbxNode*>& outBoneNodes,
                                 FbxPose* bindPose);
```

- 現行 `outputFBXBone` の FbxSkeleton・階層・BindPose追加部分を抽出
- `FbxSkin` / `FbxCluster` は作らない
- `pScene->AddPose(bindPose)` は **呼ばない**（saveFBX がメッシュループ後に1回だけ呼ぶ）
- `outRootBoneNode` = "Scene_Root" ノード（`attachFBXSkinToMesh` で `rMat` 取得に必要）

### countBone2VerForMesh（頂点カウント — per mesh）

```cpp
int CModel::countBone2VerForMesh(int boneNo, const MeshContext& ctx);
```

現行の `countBone2Ver`（`SaveFile.cpp:1594`）の `m_weldedVertices` を `ctx.weldedVertices` に差し替えるだけ。

### SetFBXBone2VerNoForMesh（ボーン-頂点マッピング — per mesh）

```cpp
bool CModel::SetFBXBone2VerNoForMesh(FbxCluster* pCluster, int boneNo,
                                      CMesh* pMesh, const MeshContext& ctx);
```

現行の `SetFBXBone2VerNo`（`SaveFile.cpp:455`）との差分:
- 全メッシュループを `pMesh` 1つに限定
- `m_vertexRemap[i + vCnt]` → `ctx.vertexRemap[i]`（vCnt 不要）
- `addedMap` サイズは `ctx.vertexRemap.size()`

### attachFBXSkinToMesh（スキン付け — per mesh）

```cpp
bool CModel::attachFBXSkinToMesh(FbxScene* pScene, FbxMesh* pfbxMesh,
                                   CMesh* srcMesh, int meshIdx,
                                   FbxNode* rootBoneNode,
                                   const std::vector<FbxNode*>& boneNodes,
                                   const MeshContext& ctx);
```

- `FbxSkin::Create` → `pfbxMesh->AddDeformer`
- ボーンごとに `countBone2VerForMesh` で頂点なしをスキップ
- `FbxCluster` 名: `"BoneXXX_Skin_M%03d"` （meshIdx 付きでユニーク化）
- `rMat = rootBoneNode->EvaluateLocalTransform()`
- `SetFBXBone2VerNoForMesh` で頂点番号設定

---

## Phase 5 — saveFBX のリファクタリング

**対象:** `SaveFile.cpp:670-830`

```
saveFBX
│
├── [既存] FbxManager / FbxScene / GlobalSettings 初期化
│
├── [変更] マテリアル作成ループ
│     → fbxMaterials: vector<FbxSurfacePhong*> に収集
│     → meshNode->AddMaterial は行わない（後でメッシュごとに追加）
│
├── [新規] ボーンノード生成（1回だけ）
│     FbxPose* bindPose = FbxPose::Create(...)
│     createFBXBoneNodes(rootNode, fbxScene, rootBoneNode, boneNodes, bindPose)
│
├── [新規] CMesh ループ（meshIdx = 0, 1, 2, ...）
│     ├── FbxNode::Create("Mesh%03d")
│     ├── FbxMesh::Create("Mesh%03d_Geo")
│     ├── 全マテリアルを AddMaterial（同順）
│     ├── FbxLayerElementMaterial 設定
│     ├── OptimizeVerticesForMesh(pMesh, ctx, true)
│     ├── outputFBXVertexForMesh(fbxMesh, ctx)
│     ├── outputFBXFaceForMesh(fbxMesh, pMesh, pMatElem, ctx)
│     └── attachFBXSkinToMesh(fbxScene, fbxMesh, pMesh, meshIdx,
│                              rootBoneNode, boneNodes, ctx)
│
├── [新規] fbxScene->AddPose(bindPose)  ← ループ後1回だけ
│
└── [既存] アニメーション出力 / FBXエクスポート（変更なし）
```

---

## Phase 6 — Model.h への宣言追加

```cpp
// Model.h に追加（CModel クラス定義の外）
struct MeshContext {
    std::vector<int>           vertexRemap;
    std::vector<WELDED_VERTEX> weldedVertices;
};

// CModel クラスに追加
void OptimizeVerticesForMesh(CMesh* pMesh, MeshContext& ctx, bool enable);
bool outputFBXVertexForMesh(FbxMesh* pfbxMesh, const MeshContext& ctx);
bool outputFBXFaceForMesh(FbxMesh* pfbxMesh, CMesh* pMesh,
                           FbxLayerElementMaterial* pMatElem, const MeshContext& ctx);
bool createFBXBoneNodes(FbxNode* pRootNode, FbxScene* pScene,
                         FbxNode*& outRootBoneNode,
                         std::vector<FbxNode*>& outBoneNodes,
                         FbxPose* bindPose);
bool attachFBXSkinToMesh(FbxScene* pScene, FbxMesh* pfbxMesh, CMesh* srcMesh,
                          int meshIdx, FbxNode* rootBoneNode,
                          const std::vector<FbxNode*>& boneNodes,
                          const MeshContext& ctx);
bool SetFBXBone2VerNoForMesh(FbxCluster* pCluster, int boneNo,
                              CMesh* pMesh, const MeshContext& ctx);
int  countBone2VerForMesh(int boneNo, const MeshContext& ctx);
```

---

## 既存関数との共存関係

| 既存関数 | 用途 | 存続 |
|---|---|---|
| `OptimizeVertices` | .x 形式 (`saveX`) | 残す |
| `outputFBXVertex` | 旧FBX（参照のみ） | 残す |
| `outputFBXFace` | 旧FBX（参照のみ） | 残す |
| `outputFBXBone` | 旧FBX（参照のみ） | 残す |
| `SetFBXBone2VerNo` | 旧FBX（参照のみ） | 残す |
| `countBone2Ver` | .x 形式 | 残す |

---

## 実装順序

1. `MeshContext` 構造体定義（Model.h）
2. `OptimizeVerticesForMesh` 実装・確認
3. `outputFBXVertexForMesh` / `outputFBXFaceForMesh` 実装
4. `createFBXBoneNodes` 実装（スキンなし、ボーンノード確認）
5. `countBone2VerForMesh` / `SetFBXBone2VerNoForMesh` 実装
6. `attachFBXSkinToMesh` 実装
7. `saveFBX` リファクタリング（全結合・動作確認）

---

## 注意点

- **FbxCluster 名の重複**: 複数メッシュで同ボーン名の Cluster を作るため、`meshIdx` をサフィックスに付与してユニーク化が必須
- **pStream->m_texNo の有効範囲**: 全マテリアルを全 FbxNode に同順で追加するため、texNo がマテリアル総数未満である前提は現状と同じ
- **アニメーション出力**: ボーンノードを参照するだけなので変更不要
