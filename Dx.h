
#pragma once

//======================================================================
// INCLUDE
//======================================================================
#include <d3d11.h>
#include <dxgi.h>
#include <d3dcompiler.h>
#include "DxMath.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")


//======================================================================
// 定数バッファ レイアウト
//======================================================================
struct CBPerFrame
{
	XMFLOAT4X4 matView;
	XMFLOAT4X4 matProj;
	XMFLOAT4   lightDir;      // ワールド空間ライト方向（正規化済み、光源へ向かう方向）
	XMFLOAT4   lightDiffuse;
	XMFLOAT4   lightAmbient;
	XMFLOAT4   lightSpecular;
	XMFLOAT4   eyePos;        // ワールド空間カメラ位置
	XMFLOAT4X4 matLightView;  // ライトビュー行列（シャドウマップ用）
	XMFLOAT4X4 matLightProj;  // ライト射影行列（シャドウマップ用）
};

struct CBPerObject
{
	XMFLOAT4X4 matWorld;
	XMFLOAT4X4 boneMatrices[128];
};


//======================================================================
// PROTOTYPE
//======================================================================
bool InitD3D( void );
void ReleaseD3D( void );
bool InitRenderTarget( void );
bool InitShaders( void );

ID3D11Device*           GetDevice( void );
ID3D11DeviceContext*    GetContext( void );
IDXGISwapChain*         GetSwapChain( void );
ID3D11RenderTargetView* GetRenderTargetView( void );
ID3D11DepthStencilView* GetDepthStencilView( void );
ID3D11InputLayout*      GetInputLayout( void );
ID3D11VertexShader*     GetVertexShader( void );
ID3D11PixelShader*      GetPixelShader( void );
ID3D11Buffer*           GetCBPerFrame( void );
ID3D11Buffer*           GetCBPerObject( void );
ID3D11SamplerState*     GetLinearSampler( void );

ID3D11DepthStencilView*   GetShadowDSV( void );
ID3D11ShaderResourceView* GetShadowSRV( void );
ID3D11VertexShader*       GetShadowVS( void );
ID3D11SamplerState*       GetShadowSampler( void );

void BeginShadowPass( void );
void EndShadowPass( void );

HRESULT CreateVB( ID3D11Buffer** ppVB, UINT byteWidth, bool dynamic = false );
HRESULT CreateIB( ID3D11Buffer** ppIB, UINT byteWidth, bool dynamic = false );
