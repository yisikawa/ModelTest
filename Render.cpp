

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
float				g_mFov			= PAI / 4.f;		// FOV : 60�x
float				g_mAspect		= 1.4f;		// ��ʂ̃A�X�y�N�g��
float				g_mNear_z		= 0.1f;			// �ŋߐڋ���
float				g_mFar_z		= 400.0f;		// �ŉ�������
D3DLIGHT9			g_mLight,g_mLightbase;
static	float		fTime		= 0;
extern	unsigned long	VertexShaderVersion;
extern	int				MaxVertexShaderConst; // ���_�V�F�[�_�[�@MAX�@Matrix
extern	bool		g_mDispWire,g_mDispIdl,g_mDispBone;

D3DXMATRIX			g_mProjection, g_mView,g_mEyeMat;
float				g_mEyeScale=1.f,g_mEyeAlph = 0.f,g_mEyeBeta = 0.f;
float				g_mLightAlph = 0.f,g_mLightBeta = 0.f;
D3DXVECTOR3			g_mEye,g_mEyebase( 0.0f,	 0.f, 3.0f);
D3DXVECTOR3			g_mAt(	0.0f,	 1.1f,	0.0f);
D3DXVECTOR3			g_mUp(	0.0f,	 1.0f,	0.0f);
LPDIRECT3DSURFACE9	g_pBackBuffer;					// �o�b�N�o�b�t�@
LPDIRECT3DSURFACE9	g_pZBuffer;						// Z�o�b�t�@
float				g_mLightDist = 1.5f;
D3DXVECTOR3			g_mLightPosition(0.f,0.f,0.f);
D3DXMATRIX			g_mViewLight;					// ���C�g���猩���ꍇ�̃r���[�}�g���b�N�X

extern	long		g_mScreenWidth;
extern	long		g_mScreenHeight;
//-------------------------------------------------------------
// �f�o�b�O�p�ɕ\������e�N�X�`���p�̍\����
//-------------------------------------------------------------
typedef struct {
    FLOAT       p[4];
    FLOAT       tu, tv;
} TVERTEX;

//======================================================================
//
//		�����_�����O
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
	// �A�j���[�V��������
	g_mTime += fTime*g_mMotionSpeed;

	// �ϊ��K�p�i�����̓A�j������

	//-----------------------------------------------
	// �����_�����O
	//-----------------------------------------------
	unsigned long poly = 0;
	//	���C�g�ʒu�̌v�Z
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
//		3D��Ԃ̐���
//
//======================================================================
bool Create3DSpace( void )
{
	HRESULT	hr;
	//===========================================================
	// �o�b�N�o�b�t�@�擾
	//===========================================================
	hr = GetDevice()->GetRenderTarget( 0,&g_pBackBuffer );
	if FAILED( hr ) return false;

	//===========================================================
	// Z�o�b�t�@����
	//===========================================================
	hr = GetDevice()->GetDepthStencilSurface( &g_pZBuffer );
	if FAILED( hr ) return false;

	//===========================================================
	// �v���W�F�N�V�����s��̐ݒ�
	//===========================================================
	// �s�񐶐�
	D3DXMatrixPerspectiveFovLH( &g_mProjection, g_mFov, g_mAspect, g_mNear_z, g_mFar_z );

	//===========================================================
	// �f�t�H���g�̃J�����̐ݒ�
	//===========================================================

	D3DXMatrixLookAtLH( &g_mView, &g_mEye, &g_mAt, &g_mUp );
	//=================================================
	// �����_�����O�X�e�[�g
	//=================================================
	GetDevice()->SetRenderState( D3DRS_DITHERENABLE,		TRUE );
	GetDevice()->SetRenderState( D3DRS_ZENABLE,				TRUE );
	GetDevice()->SetRenderState( D3DRS_ZWRITEENABLE,		TRUE );
	//=================================================
	// ���C�g
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
	// ���C�g�����̃J�����̐ݒ�
	//===========================================================

	g_mLightPosition = g_mAt+g_mLightDist*-(D3DXVECTOR3)g_mLight.Direction;
	D3DXMatrixLookAtLH( &g_mViewLight,&g_mLightPosition,&g_mAt,&g_mUp);
	return true;
}

//======================================================================
//
//		������
//
//======================================================================
bool InitRender( void )
{
	D3DXVECTOR3		Pos,Post;
	char			ComboString[128];
	//--------------------------------------------------
	// �����ݒ�
	//--------------------------------------------------
	if ( !Create3DSpace() )
	{
		MessageBox( NULL, "�����ݒ�Ɏ��s", "Error", MB_OK );
		return false;
	}
	//--------------------------------------------------
	// ���f���f�[�^�ǂݍ��݁i���_�t�H�[�}�b�g���w��
	//--------------------------------------------------
	// unsigned long ModelFVF = (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1);
	// PC �ݒ�
	pPC = new CPC;
	Inventory_init();
	// NPC�@�����ݒ�
	pNPC = new CNPC;
	pPC->GetWorldPosition( Pos );
	// NPC�@�����̏����ݒ�
 	GetWindowText(GetDlgItem(hDlg2, IDC_COMBO31), ComboString, sizeof(ComboString));
	pNPC->SetBody(ConvertStr2Dno2( ComboString ));

	// �ϊ�������
	pPC->InitTransform();
	pNPC->InitTransform();
	//------------------------------------------------------
	// ��]
	//------------------------------------------------------
	// Y���łX�O�x��]�iFFXI�͍��W�n�������ɈႤ
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
//		�J��
//
//======================================================================
void UnInitRender( void )
{
	// ���f�����
	SAFE_DELETE( pPC );
}



//======================================================================
//
//		���f�����
//
//======================================================================
void ReleaseModel( CModel **ppPC )
{
	SAFE_DELETE( *ppPC );
}

