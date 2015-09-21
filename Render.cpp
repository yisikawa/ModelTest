

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
		int			g_mMotionSpeed		=	3000; // 2300
extern	HWND		hDlg2;
float				g_mFov			= PAI / 4.f;		// FOV : 60度
float				g_mAspect		= 1.4f;		// 画面のアスペクト比
float				g_mNear_z		= 0.1f;			// 最近接距離
float				g_mFar_z		= 400.0f;		// 最遠方距離
D3DLIGHT9			g_mLight,g_mLightbase;
static	float		fTime		= 0;
extern	unsigned long	VertexShaderVersion;
extern	int				MaxVertexShaderConst; // 頂点シェーダー　MAX　Matrix
extern	bool		g_mDispWire,g_mDispIdl,g_mDispBone;

D3DXMATRIX			g_mProjection, g_mView,g_mEyeMat;
float				g_mEyeScale=1.f,g_mEyeAlph = 0.f,g_mEyeBeta = 0.f;
float				g_mLightAlph = 0.f,g_mLightBeta = 0.f;
D3DXVECTOR3			g_mEye,g_mEyebase( 0.0f,	 0.f, 3.0f);
D3DXVECTOR3			g_mAt(	0.0f,	 1.1f,	0.0f);
D3DXVECTOR3			g_mUp(	0.0f,	 1.0f,	0.0f);
LPDIRECT3DSURFACE9	g_pBackBuffer;					// バックバッファ
LPDIRECT3DSURFACE9	g_pZBuffer;						// Zバッファ
float				g_mLightDist = 1.5f;
D3DXVECTOR3			g_mLightPosition(0.f,0.f,0.f);
D3DXMATRIX			g_mViewLight;					// ライトから見た場合のビューマトリックス

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
	g_mLightPosition = g_mAt+g_mLightDist*-(D3DXVECTOR3)g_mLight.Direction;
	D3DXMatrixLookAtLH( &g_mViewLight,&g_mLightPosition,&g_mAt,&g_mUp);

	pPC->GetWorldPosition( Pos );
	if (g_mPCFlag) {
		pPC->AddTime(fTime*g_mMotionSpeed);
		pPC->DynamicTransform();
		//pPC->DynamicTransform2();
		if (g_mDispIdl) {
			pPC->BoneRendering();
		}
		else {
			poly += pPC->Rendering();
		}
	}
	else {
		pNPC->AddTime( fTime*g_mMotionSpeed );
		pNPC->DynamicTransform();
		//pNPC->DynamicTransform2();
		if (g_mDispIdl) {
			pNPC->BoneRendering();
		}
		else {
			poly += pNPC->Rendering();
		}
	}
	AdDrawPolygons( poly );
}

//======================================================================
//
//		3D空間の生成
//
//======================================================================
bool Create3DSpace( void )
{
	HRESULT	hr;
	//===========================================================
	// バックバッファ取得
	//===========================================================
	hr = GetDevice()->GetRenderTarget( 0,&g_pBackBuffer );
	if FAILED( hr ) return false;

	//===========================================================
	// Zバッファ生成
	//===========================================================
	hr = GetDevice()->GetDepthStencilSurface( &g_pZBuffer );
	if FAILED( hr ) return false;

	//===========================================================
	// プロジェクション行列の設定
	//===========================================================
	// 行列生成
	D3DXMatrixPerspectiveFovLH( &g_mProjection, g_mFov, g_mAspect, g_mNear_z, g_mFar_z );

	//===========================================================
	// デフォルトのカメラの設定
	//===========================================================

	D3DXMatrixLookAtLH( &g_mView, &g_mEye, &g_mAt, &g_mUp );
	//=================================================
	// レンダリングステート
	//=================================================
	GetDevice()->SetRenderState( D3DRS_DITHERENABLE,		TRUE );
	GetDevice()->SetRenderState( D3DRS_ZENABLE,				TRUE );
	GetDevice()->SetRenderState( D3DRS_ZWRITEENABLE,		TRUE );
	//=================================================
	// ライト
	//=================================================
	memset( &g_mLight, 0x00, sizeof(D3DLIGHT9) );
	memset( &g_mLightbase, 0x00, sizeof(D3DLIGHT9) );
	g_mLight.Type			= D3DLIGHT_DIRECTIONAL;
	g_mLight.Diffuse.a		= 1.0f;
	g_mLight.Diffuse.r		= 0.8f;
	g_mLight.Diffuse.g		= 0.8f;
	g_mLight.Diffuse.b		= 0.8f;
	g_mLight.Ambient.a		= 1.0f;
	g_mLight.Ambient.r		= 0.5f;
	g_mLight.Ambient.g		= 0.5f;
	g_mLight.Ambient.b		= 0.5f;
	g_mLight.Specular.a	= 1.0f;
	g_mLight.Specular.r	= 0.5f;
	g_mLight.Specular.g	= 0.5f;
	g_mLight.Specular.b	= 0.5f;
	D3DXVec3Normalize( (D3DXVECTOR3*)&g_mLightbase.Direction, &D3DXVECTOR3( 0.3f, -1.0f, 0.3f) );
	g_mLight.Direction = g_mLightbase.Direction;
	GetDevice()->SetLight( 0, &g_mLight );
	GetDevice()->LightEnable( 0, TRUE );

	//===========================================================
	// ライト方向のカメラの設定
	//===========================================================

	g_mLightPosition = g_mAt+g_mLightDist*-(D3DXVECTOR3)g_mLight.Direction;
	D3DXMatrixLookAtLH( &g_mViewLight,&g_mLightPosition,&g_mAt,&g_mUp);
	return true;
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

