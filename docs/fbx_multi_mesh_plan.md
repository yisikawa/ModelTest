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

---

## 追加計画: m_PartsNo による CMesh 集約 簡易版

### 背景

Claude による前回実装は、`m_PartsNo` ごとに `Parts_Head` などの親ノードを作成し、既存の `CMesh` 単位の `FbxMesh` をその配下へ移動するだけだった。

そのため、実際のジオメトリは結合されておらず、Blender 上でも期待した「1 PartsNo = 1 Mesh オブジェクト」にはならない。

今回の目的は、まず簡易版として **同じ `m_PartsNo` を持つ複数 `CMesh` を 1 つの `FbxMesh` に単純連結すること** とする。CMesh 間の頂点溶接は行わない。

### 目標

- `m_PartsNo` ごとに 1 つの `FbxNode` / `FbxMesh` を作成する。
- Blender のアウトライナー上で `Parts_Head`, `Parts_Body`, `Parts_Hand` などが Mesh オブジェクト本体として表示される。
- 既存のマテリアル割り当て `pStream->m_texNo` を維持する。
- 既存のボーンノード生成とアニメーション出力は維持する。
- スキンウェイトは Parts 全体の control point index に変換して出力する。

### 非目標

- CMesh 間の頂点溶接は行わない。
- 法線や UV が異なる境界の統合は行わない。
- 左右手足の分割など、PartsNo 以外の追加分類は今回扱わない。
- 既存 `.x` / `.mqo` 出力処理は変更しない。

### 新規データ構造

`Model.h` に Parts 集約用の context を追加する。

```cpp
struct PartMeshSource {
    CMesh* mesh;
    MeshContext meshContext;
    int baseIndex;
};

struct PartMeshContext {
    std::vector<PartMeshSource> sources;
    std::vector<WELDED_VERTEX> weldedVertices;
};
```

簡易版では、各 `CMesh` に既存の `OptimizeVerticesForMesh` を適用し、その結果を `PartMeshContext::weldedVertices` にオフセット付きで連結する。

### 新規関数

`CModel` に以下を追加する。

```cpp
void OptimizeVerticesForPart(const std::vector<CMesh*>& meshes,
                             PartMeshContext& ctx,
                             bool enable);

bool outputFBXVertexForPart(FbxMesh* pfbxMesh,
                            const PartMeshContext& ctx);

bool outputFBXFaceForPart(FbxMesh* pfbxMesh,
                          FbxLayerElementMaterial* pMatElem,
                          const PartMeshContext& ctx);

int countBone2VerForPart(int boneNo,
                         const PartMeshContext& ctx);

bool SetFBXBone2VerNoForPart(FbxCluster* pCluster,
                             int boneNo,
                             const PartMeshContext& ctx);

bool attachFBXSkinToPart(FbxScene* pScene,
                         FbxMesh* pfbxMesh,
                         int partsNo,
                         FbxNode* rootBoneNode,
                         const std::vector<FbxNode*>& boneNodes,
                         const PartMeshContext& ctx);
```

### saveFBX の変更方針

現在の `saveFBX` は `CMesh` 単位で `FbxNode` / `FbxMesh` を作成している。

```text
for each CMesh:
    create Mesh000
    OptimizeVerticesForMesh
    outputFBXVertexForMesh
    outputFBXFaceForMesh
    attachFBXSkinToMesh
```

これを `m_PartsNo` 単位の処理に変更する。

```text
group CMesh by m_PartsNo

for each partsNo:
    create Parts_xxx node
    create Parts_xxx_Geo mesh
    OptimizeVerticesForPart
    outputFBXVertexForPart
    outputFBXFaceForPart
    attachFBXSkinToPart
```

### 頂点連結ルール

1. `OptimizeVerticesForPart` は、対象 PartsNo の `CMesh` 配列を順に処理する。
2. 各 `CMesh` に対して既存 `OptimizeVerticesForMesh` を呼び、メッシュ内の重複頂点だけを整理する。
3. `PartMeshContext::weldedVertices` に追加するとき、現在のサイズを `baseIndex` として保持する。
4. 面出力時は、各 `CMesh` の `meshContext.vertexRemap[localIndex] + baseIndex` を FBX polygon index として使う。

この方式では CMesh 間の頂点は共有されないが、1 つの `FbxMesh` 内に正しい control point index として連結される。

### 面出力ルール

`outputFBXFaceForPart` は `PartMeshContext::sources` を順に処理する。

- `source.mesh` から `CStream` と index buffer を読む。
- triangle strip / triangle list の展開ロジックは既存 `outputFBXFaceForMesh` を流用する。
- `pMatElem->GetIndexArray().Add(pStream->m_texNo)` は既存どおり維持する。
- 各 source の `baseIndex` を使って、local remap を Parts 全体の index に変換する。

### スキン出力ルール

`attachFBXSkinToPart` は Parts 単位で 1 つの `FbxSkin` を作成する。

- cluster 名は `BoneName_Skin_Parts%d` のように PartsNo を含めて一意にする。
- `SetFBXBone2VerNoForPart` は全 source を走査する。
- 各 source で `CMesh::m_pBoneTbl` を使って global bone index を判定する。
- control point index は `source.meshContext.vertexRemap[i] + source.baseIndex` を使う。
- 同じ control point への重複追加を避けるため、Parts 全体サイズの `addedMap` を使う。

### Parts 名

既存の名前配列を流用する。

```cpp
static const char* kPartsName[] = {
    "Race", "Face", "Head", "Body", "Hand",
    "Leg", "Foot", "RightWeapon", "LeftWeapon", "RemoteWeapon"
};
```

ノード名は以下とする。

```text
Parts_Face
Parts_Head
Parts_Body
Parts_Hand
Parts_Leg
Parts_Foot
Parts_RightWeapon
```

範囲外の PartsNo は `Parts_%d` とする。

### 実装手順

1. `Model.h` に `PartMeshSource` / `PartMeshContext` を追加する。
2. `Model.h` に Parts 集約用関数宣言を追加する。
3. `SaveFile.cpp` に `OptimizeVerticesForPart` を実装する。
4. `outputFBXVertexForPart` を実装する。
5. `outputFBXFaceForPart` を実装する。
6. `countBone2VerForPart` / `SetFBXBone2VerNoForPart` を実装する。
7. `attachFBXSkinToPart` を実装する。
8. `saveFBX` の `CMesh` ループを PartsNo グループループに置き換える。
9. Debug ビルドでコンパイル確認する。
10. FBX を出力し、Blender でアウトライナーと表示状態を確認する。

### 確認観点

- Blender のアウトライナーで `Parts_*` が Mesh オブジェクトとして表示されること。
- `Mesh000` などの CMesh 単位オブジェクトが出力されないこと。
- 表示位置、姿勢、スケールが従来 FBX と一致すること。
- テクスチャとマテリアル割り当てが崩れていないこと。
- Armature による変形が大きく破綻していないこと。
- アニメーション出力を有効にした場合も既存と同等に動くこと。

### リスク

- スキンウェイトの index オフセットを誤ると、Parts 内の一部頂点だけが別ボーンに引っ張られる。
- `addedMap` を source 単位で持つと、同じ Parts 内で control point index が衝突する可能性があるため、Parts 全体単位で管理する。
- `pStream->m_texNo` がマテリアル配列範囲外の場合、既存と同じ前提で壊れる可能性がある。
- CMesh 間の頂点溶接を行わないため、境界の法線は完全には滑らかにならない。ただし今回の目的である PartsNo 単位のオブジェクト集約には影響しない。
