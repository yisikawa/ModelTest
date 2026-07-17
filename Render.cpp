

//======================================================================
// INCLUDE
//======================================================================
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>

#include "WinMain.h"
#include "Dx.h"
#include "Render.h"
#include "Model.h"
#include <DirectXTex.h>
#include <wincodec.h>
#pragma comment(lib, "DirectXTex.lib")

//======================================================================
// PROTOTYPE
//======================================================================
void	Inventory_init(void);
DWORD	ConvertStr2Dno2( char* DataName );
//======================================================================
// DEFINE
//======================================================================
#define SAFE_RELEASE(p)		if ( (p) != NULL ) { (p)->Release(); (p) = NULL; }
#define SAFE_DELETES(p)		if ( (p) != NULL ) { delete [] (p); (p) = NULL; }
#define SAFE_DELETE(p)		if ( (p) != NULL ) { delete (p); (p) = NULL; }
#define PAI					(3.1415926535897932384626433832795f)
#define PAI2				(PAI*2.0f)

inline DWORD FtoDW( FLOAT f ) { return *((DWORD*)&f); }

//======================================================================
// GLOBAL
//======================================================================
		bool		g_mPCFlag			=	true;
		float		g_mTime				=	0.;
		CPC			*pPC				=	NULL;
		CNPC		*pNPC				=	NULL;
		int			g_mMotionSpeed		=	3000;
		bool		g_mAnimPlaying		=	true;
		float		g_mAnimStep			=	100.0f;
extern	HWND		hDlg2;
float				g_mFov			= PAI / 4.f;
float				g_mAspect		= 16.f/9.f;
float				g_mNear_z		= 0.1f;
float				g_mFar_z		= 400.0f;
LIGHTDATA			g_mLight, g_mLightbase;
static	float		fTime		= 0;
extern	bool		g_mDispToon,g_mDispToon2,g_mDispBone;

D3DXMATRIX			g_mProjection, g_mView, g_mEyeMat;
float				g_mEyeScale=1.f, g_mEyeAlph = 0.f, g_mEyeBeta = 0.f;
float				g_mLightAlph = 0.f, g_mLightBeta = 0.f;
D3DXVECTOR3			g_mEye, g_mEyebase( 0.0f, 0.f, 3.0f );
D3DXVECTOR3			g_mAt(  0.0f, 1.1f, 0.0f );
D3DXVECTOR3			g_mUp(  0.0f, 1.0f, 0.0f );
float				g_mLightDist = 1.5f;
D3DXVECTOR3			g_mLightPosition(0.f,0.f,0.f);
D3DXMATRIX			g_mViewLight;
D3DXMATRIX			g_mProjLight;

extern	long		g_mScreenWidth;
extern	long		g_mScreenHeight;
//-------------------------------------------------------------
// デバッグ用に表示するテクスチャ用の構造体
//-------------------------------------------------------------
typedef struct {
    FLOAT       p[4];
    FLOAT       tu, tv;
} TVERTEX;

//======================================================================
//
//		レンダリング
//
//======================================================================
//

CModel* GetActiveModel() {
	return g_mPCFlag ? (CModel*)pPC : (CModel*)pNPC;
}

void Rendering( void )
{
	D3DXVECTOR3		Pos;
	static unsigned long OldTime = timeGetTime();
	unsigned long NowTime = timeGetTime();

	fTime = (float)(NowTime - OldTime) / 1000.0f;
	OldTime = NowTime;
	// アニメーション時間
	g_mTime += fTime*g_mMotionSpeed;

	// 変換適用（引数はアニメ時間

	//-----------------------------------------------
	// レンダリング
	//-----------------------------------------------
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
	// Pass3: シャドウキャッチャー床（半透明。CBPerFrameは直前のモデル描画で更新済み）
	RenderFloorShadow();
	AdDrawPolygons( poly );
}

//======================================================================
//
//		3D空間の生成
//
//======================================================================
bool Create3DSpace( void )
{
	//===========================================================
	// プロジェクション行列の設定
	//===========================================================
	D3DXMatrixPerspectiveFovLH( &g_mProjection, g_mFov, g_mAspect, g_mNear_z, g_mFar_z );

	//===========================================================
	// デフォルトのカメラの設定
	//===========================================================
	D3DXMatrixIdentity( &g_mEyeMat );
	D3DXVec3TransformNormal( &g_mEye, &g_mEyebase, &g_mEyeMat );
	g_mEye += g_mAt;
	D3DXMatrixLookAtLH( &g_mView, &g_mEye, &g_mAt, &g_mUp );

	//=================================================
	// ライト設定
	//=================================================
	memset( &g_mLight,     0x00, sizeof(g_mLight) );
	memset( &g_mLightbase, 0x00, sizeof(g_mLightbase) );
	g_mLight.Diffuse.r  = 0.8f; g_mLight.Diffuse.g  = 0.8f; g_mLight.Diffuse.b  = 0.8f; g_mLight.Diffuse.a  = 1.0f;
	g_mLight.Ambient.r  = 0.5f; g_mLight.Ambient.g  = 0.5f; g_mLight.Ambient.b  = 0.5f; g_mLight.Ambient.a  = 1.0f;
	g_mLight.Specular.r = 0.5f; g_mLight.Specular.g = 0.5f; g_mLight.Specular.b = 0.5f; g_mLight.Specular.a = 1.0f;
	D3DXVec3Normalize( &g_mLightbase.Direction, &D3DXVECTOR3( -0.3f, -1.0f, -0.3f ) );
	g_mLight.Direction = g_mLightbase.Direction;

	//===========================================================
	// ライト方向のカメラの設定
	//===========================================================

	g_mLightPosition = g_mAt + g_mLightDist * (-g_mLight.Direction);
	D3DXMatrixLookAtLH( &g_mViewLight, &g_mLightPosition, &g_mAt, &g_mUp );

	// シャドウマップ用ライト正射影行列（キャラクター範囲をカバーする 4x4 単位）
	XMStoreFloat4x4( (XMFLOAT4X4*)&g_mProjLight,
	                 XMMatrixOrthographicLH( 4.0f, 4.0f, 0.1f, 10.0f ) );
	return true;
}

//======================================================================
//
//		プロジェクション行列更新（ウィンドウリサイズ時に呼び出す）
//
//======================================================================
void UpdateProjection( void )
{
	g_mAspect = (float)g_mScreenWidth / (float)g_mScreenHeight;
	D3DXMatrixPerspectiveFovLH( &g_mProjection, g_mFov, g_mAspect, g_mNear_z, g_mFar_z );
}

//======================================================================
//
//		初期化
//
//======================================================================
bool InitRender( void )
{
	D3DXVECTOR3		Pos,Post;
	char			ComboString[128];
	//--------------------------------------------------
	// 初期設定
	//--------------------------------------------------
	if ( !Create3DSpace() )
	{
		MessageBox( NULL, "初期設定に失敗", "Error", MB_OK );
		return false;
	}
	//--------------------------------------------------
	// モデルデータ読み込み（頂点フォーマットを指定
	//--------------------------------------------------
	// unsigned long ModelFVF = (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1);
	// PC 設定
	pPC = new CPC;
	Inventory_init();
	// NPC　味方設定
	pNPC = new CNPC;
	pPC->GetWorldPosition( Pos );
	// NPC　味方の初期設定
 	GetWindowText(GetDlgItem(hDlg2, IDC_COMBO31), ComboString, sizeof(ComboString));
	pNPC->SetBody(ConvertStr2Dno2( ComboString ));

	// 変換初期化
	pPC->InitTransform();
	pNPC->InitTransform();
	//------------------------------------------------------
	// 回転
	//------------------------------------------------------
	// Y軸で９０度回転（FFXIは座標系が微妙に違う
	pPC->MirrorY();
	pPC->GetWorldPosition( Pos );Post = Pos;
	pPC->Translation( 0. ,Post.y - Pos.y , 0. );
	g_mAt.y += Post.y - Pos.y;
	g_mEye = g_mAt + g_mEyebase;
	// PC load
	if (!pPC->LoadPCParts()) return false;
	if (!pPC->LoadPCMotion()) return false;
	pPC->InitialTransform();
	pPC->ConvertMesh();
	pNPC->MirrorY();
	pNPC->Translation( 0. ,Post.y - Pos.y , 0. );
	D3DXMatrixLookAtLH( &g_mView, &g_mEye, &g_mAt, &g_mUp );
	// NPC load
	pNPC->LoadNPC();
	pNPC->InitialTransform();
	pNPC->ConvertMesh();
	return true;
}

//======================================================================
//
//		開放
//
//======================================================================
void UnInitRender( void )
{
	// モデル解体
	SAFE_DELETE( pPC );
}



//======================================================================
//
//		モデル解体
//
//======================================================================
void ReleaseModel( CModel **ppPC )
{
	SAFE_DELETE( *ppPC );
}


//======================================================================
//
//		4面図（正面・背面・左側面・右側面）をPNGとして保存
//
//		バインドポーズ（アニメなし）・正射影・シャドウなしで描画する
//
//======================================================================
bool SaveOrthographicViews( const char* outDir )
{
	CModel* pModel = GetActiveModel();
	if ( !pModel ) return false;

	auto* pDev = GetDevice();
	auto* pCtx = GetContext();

	const int W = 1024, H = 1024;

	//----------------------------------------------------------
	// オフスクリーン カラー RT 作成（RGBA8、PNG保存用）
	//----------------------------------------------------------
	D3D11_TEXTURE2D_DESC tdesc = {};
	tdesc.Width            = W;
	tdesc.Height           = H;
	tdesc.MipLevels        = 1;
	tdesc.ArraySize        = 1;
	tdesc.Format           = DXGI_FORMAT_R8G8B8A8_UNORM;
	tdesc.SampleDesc.Count = 1;
	tdesc.Usage            = D3D11_USAGE_DEFAULT;
	tdesc.BindFlags        = D3D11_BIND_RENDER_TARGET;

	ID3D11Texture2D*        pColorTex = nullptr;
	ID3D11RenderTargetView* pRTV      = nullptr;
	if ( FAILED( pDev->CreateTexture2D( &tdesc, nullptr, &pColorTex ) ) ) return false;
	if ( FAILED( pDev->CreateRenderTargetView( pColorTex, nullptr, &pRTV ) ) ) {
		pColorTex->Release(); return false;
	}

	//----------------------------------------------------------
	// 深度バッファ作成
	//----------------------------------------------------------
	D3D11_TEXTURE2D_DESC ddesc = tdesc;
	ddesc.Format    = DXGI_FORMAT_D32_FLOAT;
	ddesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

	ID3D11Texture2D*        pDepthTex = nullptr;
	ID3D11DepthStencilView* pDSV      = nullptr;
	if ( FAILED( pDev->CreateTexture2D( &ddesc, nullptr, &pDepthTex ) ) ) {
		pRTV->Release(); pColorTex->Release(); return false;
	}
	if ( FAILED( pDev->CreateDepthStencilView( pDepthTex, nullptr, &pDSV ) ) ) {
		pDepthTex->Release(); pRTV->Release(); pColorTex->Release(); return false;
	}

	//----------------------------------------------------------
	// 1×1 白テクスチャ SRV 作成（シャドウサンプラーに渡し影を無効化）
	// 深度値 1.0f（最遠）→ pixel_depth <= 1.0f が常に成立 → shadow = 1.0（影なし）
	//----------------------------------------------------------
	float whiteDepth = 1.0f;
	D3D11_TEXTURE2D_DESC wdesc = {};
	wdesc.Width            = 1;
	wdesc.Height           = 1;
	wdesc.MipLevels        = 1;
	wdesc.ArraySize        = 1;
	wdesc.Format           = DXGI_FORMAT_R32_FLOAT;
	wdesc.SampleDesc.Count = 1;
	wdesc.Usage            = D3D11_USAGE_IMMUTABLE;
	wdesc.BindFlags        = D3D11_BIND_SHADER_RESOURCE;

	D3D11_SUBRESOURCE_DATA wInit = { &whiteDepth, sizeof(float), 0 };
	ID3D11Texture2D*          pWhiteTex = nullptr;
	ID3D11ShaderResourceView* pWhiteSRV = nullptr;
	pDev->CreateTexture2D( &wdesc, &wInit, &pWhiteTex );
	pDev->CreateShaderResourceView( pWhiteTex, nullptr, &pWhiteSRV );
	pWhiteTex->Release();

	//----------------------------------------------------------
	// 現在の状態を退避
	//----------------------------------------------------------
	D3DXMATRIX  savedView = g_mView;
	D3DXMATRIX  savedProj = g_mProjection;
	D3DXVECTOR3 savedEye  = g_mEye;

	//----------------------------------------------------------
	// バインドポーズを設定（MirrorY適用済み）
	//----------------------------------------------------------
	pModel->BindPoseTransform();

	//----------------------------------------------------------
	// 正射影行列設定（キャラ全高をカバー）
	//----------------------------------------------------------
	const float orthoSize = 2.8f;
	XMStoreFloat4x4( (XMFLOAT4X4*)&g_mProjection,
	                 XMMatrixOrthographicLH( orthoSize, orthoSize, 0.1f, 20.f ) );

	//----------------------------------------------------------
	// ビューポートをオフスクリーンサイズに設定
	//----------------------------------------------------------
	D3D11_VIEWPORT vp = {};
	vp.Width    = (float)W;
	vp.Height   = (float)H;
	vp.MaxDepth = 1.0f;
	pCtx->RSSetViewports( 1, &vp );

	//----------------------------------------------------------
	// シャドウスロットに白テクスチャをバインド（影を無効化）
	//----------------------------------------------------------
	ID3D11SamplerState* pShadowSmp = GetShadowSampler();
	pCtx->PSSetShaderResources( 1, 1, &pWhiteSRV );
	pCtx->PSSetSamplers( 1, 1, &pShadowSmp );

	//----------------------------------------------------------
	// 4方向レンダリング＆PNG保存
	// BindPoseTransform で MirrorY が適用済みのため Y は正方向。
	// 水平反転のみ行い、旧コード（180度回転）と同じ向きを維持する。
	//----------------------------------------------------------
	const D3DXVECTOR3 orthoAt( g_mAt.x, g_mAt.y, g_mAt.z );

	struct ViewDef { D3DXVECTOR3 eye; const char* suffix; };
	const ViewDef views[] = {
		{ D3DXVECTOR3(  0.f, g_mAt.y,  5.f ), "front" },
		{ D3DXVECTOR3(  0.f, g_mAt.y, -5.f ), "back"  },
		{ D3DXVECTOR3(  5.f, g_mAt.y,  0.f ), "left"  },
		{ D3DXVECTOR3( -5.f, g_mAt.y,  0.f ), "right" },
	};

	bool result = true;
	for ( const auto& v : views ) {
		g_mEye = v.eye;
		D3DXMatrixLookAtLH( &g_mView, &g_mEye, &orthoAt, &g_mUp );

		pCtx->OMSetRenderTargets( 1, &pRTV, pDSV );
		const float clearColor[] = { 0.f, 0.f, 0.f, 0.f };
		pCtx->ClearRenderTargetView( pRTV, clearColor );
		pCtx->ClearDepthStencilView( pDSV, D3D11_CLEAR_DEPTH, 1.0f, 0 );

		pModel->Rendering();

		char path[MAX_PATH];
		sprintf( path, "%s\\%s.png", outDir, v.suffix );
		wchar_t wpath[MAX_PATH];
		MultiByteToWideChar( CP_ACP, 0, path, -1, wpath, MAX_PATH );

		DirectX::ScratchImage img;
		if ( SUCCEEDED( DirectX::CaptureTexture( pDev, pCtx, pColorTex, img ) ) ) {
			DirectX::ScratchImage flipped;
			if ( SUCCEEDED( DirectX::FlipRotate( *img.GetImage( 0, 0, 0 ),
			                                     DirectX::TEX_FR_FLIP_HORIZONTAL, flipped ) ) ) {
				DirectX::SaveToWICFile( *flipped.GetImage( 0, 0, 0 ),
				                        DirectX::WIC_FLAGS_NONE,
				                        GUID_ContainerFormatPng, wpath );
			} else {
				result = false;
			}
		} else {
			result = false;
		}
	}

	//----------------------------------------------------------
	// 状態を復元
	//----------------------------------------------------------
	g_mView       = savedView;
	g_mProjection = savedProj;
	g_mEye        = savedEye;

	pModel->DynamicTransform();

	ID3D11RenderTargetView* pMainRTV = GetRenderTargetView();
	ID3D11DepthStencilView* pMainDSV = GetDepthStencilView();
	pCtx->OMSetRenderTargets( 1, &pMainRTV, pMainDSV );

	D3D11_VIEWPORT mainVP = {};
	mainVP.Width    = (float)GetScreenWidth();
	mainVP.Height   = (float)GetScreenHeight();
	mainVP.MaxDepth = 1.0f;
	pCtx->RSSetViewports( 1, &mainVP );

	ID3D11ShaderResourceView* pShadowSRV = GetShadowSRV();
	pCtx->PSSetShaderResources( 1, 1, &pShadowSRV );
	pCtx->PSSetSamplers( 1, 1, &pShadowSmp );

	//----------------------------------------------------------
	// リソース解放
	//----------------------------------------------------------
	pWhiteSRV->Release();
	pDSV->Release();
	pDepthTex->Release();
	pRTV->Release();
	pColorTex->Release();

	return result;
}

