# DX9 → DX11 移行計画

## 現状の整理

| ファイル | DX9依存の内容 |
|---|---|
| Dx.h / Dx.cpp | デバイス初期化、VB/IB生成 |
| Render.h / Render.cpp | 描画ループ、ライト、カメラ行列 |
| Model.h | テクスチャ/バッファ型、D3DX数学型、FVF |
| Model_Mesh.cpp | 頂点/インデックスバッファ操作 |
| Model_Common.cpp | D3DX行列演算 |
| Model_Anim.cpp | D3DX行列/クォータニオン補間 |

---

## Phase 1 — デバイス層の置き換え（Dx.h / Dx.cpp）

最初に土台を作る。他のすべてがここに依存しているため、まずここを固める。

**変更内容:**
- `Direct3DCreate9` → `D3D11CreateDeviceAndSwapChain` + `IDXGISwapChain`
- `IDirect3DDevice9` → `ID3D11Device` + `ID3D11DeviceContext`
- `D3DPRESENT_PARAMETERS` → `DXGI_SWAP_CHAIN_DESC`
- `D3DPOOL_MANAGED` は廃止 → `D3D11_USAGE_DEFAULT` / `D3D11_USAGE_DYNAMIC` に分ける
- `CreateVB` / `CreateIB` → `ID3D11Buffer` を `D3D11_BIND_VERTEX_BUFFER` / `D3D11_BIND_INDEX_BUFFER` で生成
- バックバッファ/Zバッファは `ID3D11RenderTargetView` + `ID3D11DepthStencilView` に変更

---

## Phase 2 — 数学ライブラリの置き換え（全ファイル）

DX11ではD3DXライブラリが**完全廃止**。影響が広いため早期に一括対応する。

**変更内容:**

```
D3DXMATRIX       → XMMATRIX / XMFLOAT4X4
D3DXVECTOR3      → XMVECTOR / XMFLOAT3
D3DXQUATERNION   → XMVECTOR / XMFLOAT4
D3DXMatrixLookAtLH          → XMMatrixLookAtLH
D3DXMatrixPerspectiveFovLH  → XMMatrixPerspectiveFovLH
D3DXVec3Normalize           → XMVector3Normalize
D3DXMatrixMultiply          → XMMatrixMultiply
D3DXQuaternionSlerp         → XMQuaternionSlerp
```

- `#include <d3dx9.h>` → `#include <DirectXMath.h>` + `using namespace DirectX;`
- XMMATRIX はSIMD整列が必要なため、構造体メンバには `XMFLOAT4X4` を使いGetterで `XMLoadFloat4x4` する

---

## Phase 3 — 固定機能パイプラインの廃止（CMesh, CModel::Rendering）

DX11ではFVFと固定機能シェーダーが**完全削除**。最大の変更点。

### 3-1. 入力レイアウト（Input Layout）の作成

```cpp
// FVF_BLENDVERTEX の代替
D3D11_INPUT_ELEMENT_DESC layout[] = {
    {"POSITION",     0, DXGI_FORMAT_R32G32B32_FLOAT,   0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"BLENDWEIGHT",  0, DXGI_FORMAT_R32_FLOAT,          0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"BLENDINDICES", 0, DXGI_FORMAT_R8G8B8A8_UINT,     0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"NORMAL",       0, DXGI_FORMAT_R32G32B32_FLOAT,   0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"TEXCOORD",     0, DXGI_FORMAT_R32G32_FLOAT,      0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0},
};
```

### 3-2. HLSLシェーダーの新規作成

- `skinning_vs.hlsl` — スキニング頂点シェーダー（ボーン行列128本をconstant bufferで渡す）
- `basic_ps.hlsl` — テクスチャ+ライティングピクセルシェーダー
- DX9で使っていた `IDirect3DVertexShader9` を `ID3D11VertexShader` + `ID3D11PixelShader` に変更

### 3-3. Constant Bufferの作成

```cpp
struct CBPerFrame {
    XMFLOAT4X4 matView;
    XMFLOAT4X4 matProj;
    XMFLOAT4   lightDir;
    XMFLOAT4   lightDiffuse;
    XMFLOAT4   lightAmbient;
};

struct CBPerObject {
    XMFLOAT4X4 matWorld;
    XMFLOAT4X4 boneMatrices[128]; // 注意: 8192バイト → Structured Buffer検討
};
```

---

## Phase 4 — テクスチャの置き換え（CMaterial）

**変更内容:**
- `IDirect3DTexture9` → `ID3D11ShaderResourceView` + `ID3D11Texture2D`
- `D3DXCreateTextureFromFile` → DirectXTK: `CreateWICTextureFromFile`
- `SetTexture(stage, tex)` → `PSSetShaderResources(slot, 1, &srv)`
- `D3DMATERIAL9` はConstant Bufferの材質パラメータ（diffuse/ambient/specularのfloat4）に移行

---

## Phase 5 — レンダリングステートの置き換え（Render.cpp）

DX11ではレンダリングステートはイミュータブルなオブジェクト。

| DX9 | DX11 |
|---|---|
| `SetRenderState(D3DRS_ZENABLE, TRUE)` | `ID3D11DepthStencilState` |
| `SetRenderState(D3DRS_CULLMODE, ...)` | `ID3D11RasterizerState` |
| `SetRenderState(D3DRS_ALPHABLENDENABLE, ...)` | `ID3D11BlendState` |
| `SetLight` / `LightEnable` | 廃止 → シェーダー内で計算 |

---

## Phase 6 — 描画コール（CModel::Rendering）

**変更内容:**

| DX9 | DX11 |
|---|---|
| `SetStreamSource` / `SetFVF` / `SetIndices` | `IASetVertexBuffers` / `IASetIndexBuffer` / `IASetInputLayout` |
| `DrawIndexedPrimitive` | `DrawIndexed` |
| `D3DPT_TRIANGLELIST` | `IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST)` |

---

## 作業順序まとめ

```
Phase 1: Dx.cpp            — デバイス/SwapChain初期化
Phase 2: 全ファイル         — D3DX→DirectXMath 一括置換
Phase 3: Dx.cpp + 新規HLSL — VB/IB生成＋シェーダー作成
Phase 4: Model_Mesh.cpp    — テクスチャ読み込み
Phase 5: Render.cpp        — ステートオブジェクト
Phase 6: Model_Common.cpp  — 描画コール
```

---

## 注意点

- **DirectXTK**（DirectX Tool Kit）の導入を推奨。テクスチャ読み込み・スプライト等のユーティリティがそのまま使える
- ボーン行列128本はConstant Bufferの制限（128×64バイト = **8192バイト**）に注意。Structured Bufferまたは複数パスへの分割を検討すること
- `D3DPOOL_MANAGED` の廃止によりデバイスロスト処理は不要になる（DX11ではデバイスロストが大幅に簡素化）
