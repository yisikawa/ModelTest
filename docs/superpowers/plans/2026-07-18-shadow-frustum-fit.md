# シャドウ視錐台の動的フィット Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 大型キャラクター（翼を広げた大鳥など）でもシャドウキャッチャー床に影が欠けず落ちるよう、シャドウマップ用ライト視錐台をモデルの実寸に毎フレーム自動フィットさせる。

**Architecture:** モデルのボーンワールド行列の平行移動成分から AABB を毎フレーム算出し（GPU スキニングのため頂点ではなくボーン位置を使用。羽先など骨から離れる頂点はパディングで吸収）、その AABB を包含する境界球に合わせてライトビュー行列 `g_mViewLight` と正射影行列 `g_mProjLight` を再計算する。従来の固定 4×4 単位・距離 1.5 の視錐台は、モデル未取得時のフォールバックとしてのみ残す。併せて床クアッドを大型モデルの影がはみ出さないサイズに拡大する。

**Tech Stack:** C++ / DirectX 11 / DirectXMath（D3DX互換ラッパー DxMath.h）/ MSBuild

## Global Constraints

- ビルド検証は必ず msbuild を使用する（ユーザー指示）: `msbuild ModelTest.sln -p:Configuration=Release -p:Platform=x64 -m`
- テストフレームワークは存在しない。各タスクの検証は「ビルド成功」＋「アプリ起動しての目視確認」で行う。
- コメント・コミットメッセージは既存コードに合わせて日本語で書く。
- `min`/`max` は windows.h のマクロと衝突しうるため使わず、三項演算子で書く（既存コード Model.cpp:664 と同スタイル）。
- コミット末尾に `Co-Authored-By: Claude Fable 5 <noreply@anthropic.com>` を付ける。

## 背景（調査済みの根本原因）

- `Render.cpp:172-174` — シャドウ用正射影が固定 `XMMatrixOrthographicLH(4.0f, 4.0f, 0.1f, 10.0f)`（±2単位）。翼端がはみ出すとシャドウパスでクリップされ深度が書き込まれない。
- `Render.cpp:59` / `Render.cpp:103` — ライト視点が `g_mAt` の上方 1.5 単位固定。高い部位が near 面より手前に出る。
- `Dx.cpp:659` + `floor_ps.hlsl` — 書き込まれなかったテクセルはクリア値 1.0 のまま「影なし」と評価されるため、クリップが「影が消える」として現れる。

---

### Task 1: CModel::GetBoneWorldAABB — ボーンワールドAABB算出

**Files:**
- Modify: `Model.h` (CModel クラス宣言、`DynamicTransform` 宣言の直後 = 560行付近)
- Modify: `Model.cpp` (`DynamicTransform` 実装の直後 = 648行付近)

**Interfaces:**
- Consumes: `m_nBone` (int), `m_Bones[i].m_mWorld` (D3DXMATRIX) — `DynamicTransform()` 実行後に有効なボーンのワールド行列。平行移動成分は `_41,_42,_43`。
- Produces: `virtual bool CModel::GetBoneWorldAABB( D3DXVECTOR3 &vMin, D3DXVECTOR3 &vMax )` — 全ボーンのワールド位置を包含する AABB を返す。ボーンが無い場合は false。Task 2 が使用する。

- [ ] **Step 1: Model.h に宣言を追加**

`Model.h` の CModel クラス内、`virtual void DynamicTransform(void);` の行（560行付近）の直後に追加:

```cpp
	virtual bool	GetBoneWorldAABB( D3DXVECTOR3 &vMin, D3DXVECTOR3 &vMax );
```

- [ ] **Step 2: Model.cpp に実装を追加**

`Model.cpp` の `CModel::DynamicTransform` 実装の閉じ括弧（648行付近）の直後に追加:

```cpp
//======================================================================
//
//		ボーンワールドAABB算出
//
//		DynamicTransform() 適用後のボーンワールド位置を包含する
//		AABBを返します。シャドウ視錐台のフィットに使用します。
//		頂点はGPUスキニングのためCPU側に無く、ボーン位置で代用します
//		（骨から離れた頂点分は呼び出し側でパディングを加えること）。
//
//	output
//		true: vMin/vMax 有効	false: ボーンなし
//
//======================================================================
bool CModel::GetBoneWorldAABB( D3DXVECTOR3 &vMin, D3DXVECTOR3 &vMax )
{
	if ( m_nBone <= 0 ) return false;

	vMin = D3DXVECTOR3(  FLT_MAX,  FLT_MAX,  FLT_MAX );
	vMax = D3DXVECTOR3( -FLT_MAX, -FLT_MAX, -FLT_MAX );
	for ( int i = 0; i < m_nBone; i++ ) {
		const D3DXMATRIX &m = m_Bones[i].m_mWorld;
		vMin.x = vMin.x < m._41 ? vMin.x : m._41;
		vMin.y = vMin.y < m._42 ? vMin.y : m._42;
		vMin.z = vMin.z < m._43 ? vMin.z : m._43;
		vMax.x = vMax.x > m._41 ? vMax.x : m._41;
		vMax.y = vMax.y > m._42 ? vMax.y : m._42;
		vMax.z = vMax.z > m._43 ? vMax.z : m._43;
	}
	return true;
}
```

`FLT_MAX` は `<cfloat>`（または windows 系ヘッダー経由）で定義済み。ビルドエラーになる場合のみ `Model.cpp` 冒頭の include 群に `#include <cfloat>` を追加する。

- [ ] **Step 3: ビルドして成功を確認**

Run: `msbuild ModelTest.sln -p:Configuration=Release -p:Platform=x64 -m`
Expected: `0 エラー` でビルド成功（警告は既存分のみ）。

- [ ] **Step 4: Commit**

```bash
git add Model.h Model.cpp
git commit -m "ボーンワールドAABB算出メソッドを追加（シャドウ視錐台フィット用）

Co-Authored-By: Claude Fable 5 <noreply@anthropic.com>"
```

---

### Task 2: シャドウ用ライト行列をモデルAABBに毎フレームフィット

**Files:**
- Modify: `Render.cpp:85-132`（`Rendering()` と、その直前に static 関数を追加）

**Interfaces:**
- Consumes: `CModel::GetBoneWorldAABB( D3DXVECTOR3&, D3DXVECTOR3& )` → bool（Task 1）、既存グローバル `g_mLight.Direction` / `g_mLightPosition` / `g_mViewLight` / `g_mProjLight` / `g_mLightDist` / `g_mAt` / `g_mUp`、既存関数 `CModel* GetActiveModel()`（Render.cpp:81）
- Produces: `static void UpdateShadowLightMatrices( CModel *pModel )` — `g_mLightPosition` / `g_mViewLight` / `g_mProjLight` を更新する。Render.cpp 内部専用。

- [ ] **Step 1: UpdateShadowLightMatrices を追加**

`Render.cpp` の `GetActiveModel()`（81-83行）と `void Rendering( void )`（85行）の間に追加:

```cpp
//======================================================================
//
//		シャドウ用ライト行列の更新
//
//		モデルのボーンAABBを包含する境界球にライトの視点位置と
//		正射影サイズを毎フレームフィットさせます。固定4x4視錐台では
//		大型モデル（翼を広げた大鳥など）の末端がクリップされ、
//		シャドウマップに深度が書かれず影が欠けるため。
//
//======================================================================
static void UpdateShadowLightMatrices( CModel *pModel )
{
	D3DXVECTOR3 vMin, vMax;

	if ( pModel == NULL || !pModel->GetBoneWorldAABB( vMin, vMax ) ) {
		// フォールバック: 従来の固定視錐台（4x4単位・距離1.5）
		g_mLightPosition = g_mAt + g_mLightDist * (-g_mLight.Direction);
		D3DXMatrixLookAtLH( &g_mViewLight, &g_mLightPosition, &g_mAt, &g_mUp );
		XMStoreFloat4x4( (XMFLOAT4X4*)&g_mProjLight,
		                 XMMatrixOrthographicLH( 4.0f, 4.0f, 0.1f, 10.0f ) );
		return;
	}

	// ボーン位置AABBは頂点（羽先・髪など）を含まないため余白を加える
	D3DXVECTOR3 ext = vMax - vMin;
	float maxExt = ext.x > ext.y ? ext.x : ext.y;
	maxExt = maxExt > ext.z ? maxExt : ext.z;
	float pad = maxExt * 0.35f;
	pad = pad > 0.5f ? pad : 0.5f;
	vMin -= D3DXVECTOR3( pad, pad, pad );
	vMax += D3DXVECTOR3( pad, pad, pad );

	// AABBを包含する境界球
	D3DXVECTOR3 center = ( vMin + vMax ) * 0.5f;
	D3DXVECTOR3 half   = ( vMax - vMin ) * 0.5f;
	float radius = D3DXVec3Length( &half );

	// ライト視点: 境界球の外側 (radius + 1.0) からモデル中心を見る
	float dist = radius + 1.0f;
	g_mLightPosition = center + dist * (-g_mLight.Direction);

	// ライトがほぼ真下向きだと up=(0,1,0) と視線が平行になり
	// LookAt が退化するため、そのときは Z 軸を up に使う
	D3DXVECTOR3 up = ( fabsf( g_mLight.Direction.y ) > 0.99f )
	               ? D3DXVECTOR3( 0.f, 0.f, 1.f ) : g_mUp;
	D3DXMatrixLookAtLH( &g_mViewLight, &g_mLightPosition, &center, &up );

	// far: 中心の先 radius 分に加え、床(y=0)まで届く距離と余白を確保
	float centerY = center.y > 0.f ? center.y : 0.f;
	float farZ = dist + radius + centerY + 2.0f;
	XMStoreFloat4x4( (XMFLOAT4X4*)&g_mProjLight,
	                 XMMatrixOrthographicLH( radius * 2.0f, radius * 2.0f, 0.1f, farZ ) );
}
```

- [ ] **Step 2: Rendering() をアクティブモデル共通処理に書き換え**

`Rendering()` 内の以下のブロック（102-128行）:

```cpp
	unsigned long poly = 0;
	//	ライト位置の計算
	g_mLightPosition = g_mAt + g_mLightDist * (-g_mLight.Direction);
	D3DXMatrixLookAtLH( &g_mViewLight, &g_mLightPosition, &g_mAt, &g_mUp );

	pPC->GetWorldPosition( Pos );
	if (g_mPCFlag) {
		if (g_mAnimPlaying) pPC->AddTime(fTime*g_mMotionSpeed);
		pPC->DynamicTransform();
		//pPC->DynamicTransform2();
		// Pass1: シャドウパス
		BeginShadowPass();
		pPC->ShadowRendering();
		EndShadowPass();
		// Pass2: メインパス
		poly += pPC->Rendering();
	}
	else {
		if (g_mAnimPlaying) pNPC->AddTime( fTime*g_mMotionSpeed );
		pNPC->DynamicTransform();
		//pNPC->DynamicTransform2();
		// Pass1: シャドウパス
		BeginShadowPass();
		pNPC->ShadowRendering();
		EndShadowPass();
		// Pass2: メインパス
		poly += pNPC->Rendering();
	}
```

を次に置き換える（PC/NPC 分岐は `GetActiveModel()` で統一。ライト行列更新は**ボーン行列確定後**＝`DynamicTransform()` の後に移動するのが本修正の要点）:

```cpp
	unsigned long poly = 0;

	pPC->GetWorldPosition( Pos );
	CModel *pModel = GetActiveModel();
	if (g_mAnimPlaying) pModel->AddTime( fTime*g_mMotionSpeed );
	pModel->DynamicTransform();
	//pModel->DynamicTransform2();
	// ライト行列更新（ボーン行列確定後にモデルAABBへ視錐台をフィット）
	UpdateShadowLightMatrices( pModel );
	// Pass1: シャドウパス
	BeginShadowPass();
	pModel->ShadowRendering();
	EndShadowPass();
	// Pass2: メインパス
	poly += pModel->Rendering();
```

注意: `Create3DSpace()`（Render.cpp:169-174）の初期化時の固定視錐台設定は**変更しない**。モデルロード前の初期値として残す（毎フレーム `UpdateShadowLightMatrices` が上書きする）。

- [ ] **Step 3: ビルドして成功を確認**

Run: `msbuild ModelTest.sln -p:Configuration=Release -p:Platform=x64 -m`
Expected: `0 エラー` でビルド成功。

- [ ] **Step 4: 目視確認**

`x64\Release\ModelTest.exe` を起動し、以下を確認する:
1. 通常サイズの PC（ヒューム♂など）で従来どおり足元に影が出る（劣化なし）。
2. NPC を `1-5-18,大鳥(朱雀)` に切り替え、翼を広げるモーション（at1 等）で**翼端・首まで影が欠けず**床に落ちる。
3. 影の輪郭が以前よりぼやける（シャドウマップ2048px を大きい範囲に広げるため解像度が相対的に下がる）のは想定内。極端に汚い場合は Step 1 の `pad` 係数 0.35f を 0.25f に下げて再確認。

- [ ] **Step 5: Commit**

```bash
git add Render.cpp
git commit -m "シャドウ視錐台をモデルのボーンAABBに毎フレームフィットさせる

固定4x4単位・ライト距離1.5の視錐台では大型モデル（大鳥など）の
翼端・首がクリップされシャドウマップに書き込まれず影が欠けていた。
ボーンワールドAABBの境界球にライト視点と正射影サイズを合わせて解決。

Co-Authored-By: Claude Fable 5 <noreply@anthropic.com>"
```

---

### Task 3: 床クアッドを大型モデルの影に合わせて拡大

**Files:**
- Modify: `Dx.cpp`（`InitShaders()` 内の床用頂点バッファ生成、`floorVerts` 定義箇所）

**Interfaces:**
- Consumes: なし（定数変更のみ）
- Produces: なし（後続タスクなし）

- [ ] **Step 1: 床クアッドのサイズを ±3 → ±10 に変更**

`Dx.cpp` の床用頂点バッファ生成内:

```cpp
		const float floorY = -0.001f;
		float floorVerts[4][3] = {
			{ -3.0f, floorY, -3.0f },
			{ -3.0f, floorY,  3.0f },
			{  3.0f, floorY, -3.0f },
			{  3.0f, floorY,  3.0f },
		};
```

を次に変更（影範囲外は alpha=0 で透明のため、床を広げても見た目のコストはない）:

```cpp
		const float floorY = -0.001f;
		// 大型モデル（翼を広げた大鳥など）の影が床からはみ出さないよう±10単位
		float floorVerts[4][3] = {
			{ -10.0f, floorY, -10.0f },
			{ -10.0f, floorY,  10.0f },
			{  10.0f, floorY, -10.0f },
			{  10.0f, floorY,  10.0f },
		};
```

- [ ] **Step 2: ビルドして成功を確認**

Run: `msbuild ModelTest.sln -p:Configuration=Release -p:Platform=x64 -m`
Expected: `0 エラー` でビルド成功。

- [ ] **Step 3: 目視確認**

`x64\Release\ModelTest.exe` を起動し、大鳥(朱雀) の翼の影が床クアッドの縁で四角く切れていないことを確認する。

- [ ] **Step 4: Commit**

```bash
git add Dx.cpp
git commit -m "床クアッドを±10単位に拡大（大型モデルの影のはみ出し対策）

Co-Authored-By: Claude Fable 5 <noreply@anthropic.com>"
```

---

## Self-Review 結果

- **Spec coverage:** 根本原因3点（正射影サイズ固定 → Task 2、ライト距離固定 → Task 2、モデルサイズ非追従 → Task 1+2）を網羅。派生問題（床クアッドが小さく大型モデルの影がはみ出す）→ Task 3。
- **Placeholder scan:** 全ステップに実コード・実コマンドを記載済み。
- **Type consistency:** `GetBoneWorldAABB( D3DXVECTOR3&, D3DXVECTOR3& ) → bool` は Task 1 の宣言と Task 2 の呼び出しで一致。`GetActiveModel()` は既存関数（Render.cpp:81）。
- **既知のトレードオフ:** (1) ボーンAABB＋パディングは頂点の正確な外接ではないため、骨から極端に離れた頂点を持つモデルでは pad 係数の調整が必要になる可能性がある（Task 2 Step 4 に対処を記載）。(2) 視錐台が広がるぶんシャドウマップの実効解像度が下がり影の輪郭が甘くなるが、これはフィット方式の本質的コストであり許容とする。
