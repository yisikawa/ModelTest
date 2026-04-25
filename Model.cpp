// INCLUDE
#define _CRT_SECURE_NO_WARNINGS
#include <vector>
#include <sstream>
#include <iostream>
#include <string>
#include <stdio.h>
#include "Dx.h"
#include "Model.h"

//Function
DWORD _GetFileNameFromIDsub(DWORD dwV,DWORD dwID);
BOOL GetFileNameFromDno(LPSTR filename,DWORD dwID);
BOOL GetFileNameFromDir(LPSTR filename,char *DataName );
DWORD	ConvertStr2Dno( char* DataName );
HRESULT CreateVB( LPDIRECT3DVERTEXBUFFER9 *lpVB, DWORD size, DWORD Usage, DWORD fvf );
HRESULT CreateIB( LPDIRECT3DINDEXBUFFER9 *lpIB, DWORD size, DWORD Usage );

// DEFINE
#define SAFE_RELEASE(p)		if ( (p) != NULL ) { (p)->Release(); (p) = NULL; }
#define SAFE_DELETES(p)		if ( (p) != NULL ) { delete [] (p); (p) = NULL; }
#define SAFE_DELETE(p)		if ( (p) != NULL ) { delete (p); (p) = NULL; }
#define PAI					(3.1415926535897932384626433832795f)
#define PAI2				(3.1415926535897932384626433832795f*2.0f)

// Model_Common.cpp で定義されている定数・変数の宣言
extern const D3DXMATRIX matrixMirrorX;
extern const D3DXMATRIX matrixMirrorY;
extern const D3DXMATRIX matrixMirrorZ;
extern int g_mBLCnvTbl[9][10];
extern D3DVERTEXELEMENT9 VS0Formats[];

// 他のファイルで定義されているグローバル変数を参照
extern bool g_mPCFlag;

// Model_Common.cpp で定義されているユーティリティ関数の宣言
extern int Trim(char *s);
extern bool convert_path(char *path, const char *target);
extern void convert_texture_path(char *path);
extern	CPC			*pPC;
extern	CNPC		*pNPC;
extern	bool		g_mPCFlag;
extern	BOOL		g_mIsUseSoftware;
extern	D3DLIGHT9	g_mLight,g_mLightbase;
extern	D3DXMATRIX	g_mProjection, g_mView;
extern	D3DXVECTOR3	g_mAt,g_mEye,g_mUp;
extern	float		g_mTime;
extern	float		g_mLightDist;
extern	D3DXVECTOR3	g_mLightPosition;
extern	D3DXMATRIX	g_mViewLight;					// ライトから見た場合のビューマトリックス
extern	bool		g_mDispWire,g_mDispIdl,g_mDispBone;
extern	int			g_mDispBoneNo,g_mShlBoneNoR,g_mShlBoneNoL;
extern	char		g_meshPath[];
extern	char		g_texPath[];

// Model_Common.cpp で定義されている変数・テーブルを extern で参照
extern int			g_mPCMotion;
extern int			g_mBaseMotionFileNo[];
extern int			g_mMotionBFileNo[];
extern int			g_mmotionBnum[];
extern int			g_msinobiINFO[];
extern int			g_mkatanaINFO[];
extern int			g_mMotionB2FileNo[];
extern int			g_mmotionB2num[];
extern int			g_mMotionEFileNo[];
extern int			g_mmotionEnum;
extern int			g_mMotionAFileNo[];
extern int			g_mmotionAnum;
extern int			g_mMotionBLFileNo[];
extern int			g_mmotionBLnum[];
extern int			g_mMotionBAFileNo[];
extern int			g_mmotionBAnum;
extern int			g_mMotionTuruFileNo[];
extern int			g_mmotionTurunum;

//------------------------------------------------------------------------------------------//
//																							//
//																							//
//									CModel													//
//																							//
//																							//
//------------------------------------------------------------------------------------------//

//======================================================================
//
//		コンストラクタ
//
//======================================================================
CModel::CModel()
{
	m_nMesh				= 0;
	m_nBone				= 0;
	for( int i=0 ; i<128 ; i++ ) m_Bones[i].InitData();
	for( int i=0 ; i<128 ; i++ ) m_MotionArray[i].InitData();
	m_Meshs.Init();
	m_Materials.Init();
	m_motions.Init();

	strcpy(m_MotionName,"idl");
	D3DXMatrixIdentity(&m_mRootTransform);

}

//======================================================================
//
//		デストラクタ
//
//======================================================================
CModel::~CModel()
{
	for( int i=0 ; i<128 ; i++ ) m_Bones[i].InitData();
	for( int i=0 ; i<128 ; i++ ) m_MotionArray[i].InitData();
	CMotionFrame *pAnimFrame = (CMotionFrame*)m_motions.Top();
	while( pAnimFrame ) {
		pAnimFrame->~CMotionFrame();
		pAnimFrame = (CMotionFrame*)pAnimFrame->Next;
	}
	m_motions.Release();
	CMesh *pMesh = (CMesh*)m_Meshs.Top();
	while( pMesh ) {
		pMesh->~CMesh();
		pMesh = (CMesh*)pMesh->Next;
	}
	m_Meshs.Release();
	CMaterial *pMaterial = (CMaterial*)m_Materials.Top();
	while( pMaterial ) {
		pMaterial->~CMaterial();
		pMaterial = (CMaterial*)pMaterial->Next;
	}
	m_Materials.Release();
}

void	CModel::ClearMotion( void )
 {
	for( int i=0 ; i<128 ; i++ ) m_MotionArray[i].InitData();
	m_Time = 0.f;
}

//======================================================================
//
//		データの初期化
//
//======================================================================
void	CModel::InitPartsData(void)
{
	for( int i=0 ; i<128 ; i++ )
		m_Bones[i].InitData();
	CMesh *pMesh = (CMesh*)m_Meshs.Top();
	while( pMesh ) {
		pMesh->~CMesh();
		pMesh = (CMesh*)pMesh->Next;
	}
	m_Meshs.Release();
	CMaterial *pMaterial = (CMaterial*)m_Materials.Top();
	while( pMaterial ) {
		pMaterial->~CMaterial();
		pMaterial = (CMaterial*)pMaterial->Next;
	}
	m_Materials.Release();
}

//======================================================================
//
//		データの初期化
//
//======================================================================
void	CModel::InitMotionData(void)
{
	for( int i=0 ; i<128 ; i++ ) m_MotionArray[i].InitData();
	CMotionFrame *pAnimFrame = (CMotionFrame*)m_motions.Top();
	while( pAnimFrame ) {
		pAnimFrame->~CMotionFrame();
		pAnimFrame = (CMotionFrame*)pAnimFrame->Next;
	}
	m_motions.Release();
}

void CModel::RotateZero( D3DXMATRIX *mat ) {
	mat->_11 = 1.f;mat->_12 = 0.f;mat->_13 = 0.f;
	mat->_21 = 0.f;mat->_22 = 1.f;mat->_23 = 0.f;
	mat->_31 = 0.f;mat->_32 = 0.f;mat->_33 = 1.f;
}

//======================================================================
//
//		ボーンの読み込み
//
//	input
//		char *filename			: 読み込みファイル名（リソース名でも可
//		unsigned long FVF		: メッシュのFVF
//
//	output
//		エラー文字列へのポインタ。
//		正常終了の場合はNULL。
//
//======================================================================
HRESULT CModel::LoadBoneFromFile( char *FileName )
{
	HRESULT hr							= S_OK;
	
	//====================================================
	// ファイルをメモリに取り込む
	//====================================================
	char *pdat=NULL;
	int dwSize;
	D3DXMATRIX Matrix;
	unsigned long	cnt;
	HANDLE hFile = CreateFile(FileName,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_ARCHIVE,NULL);
	if( hFile!=INVALID_HANDLE_VALUE ){
		dwSize = GetFileSize(hFile,NULL);
	    pdat = new char[dwSize]();
	    ReadFile(hFile,pdat,dwSize,&cnt,NULL);
	    CloseHandle(hFile);
		hr = 0;
	} else {
		return -1;
	}
	//====================================================
	// ボーンの読み込み
	//====================================================
	int			type,pos=0,next;
	while( pos<dwSize ) {
		next = *((int*)(pdat+pos+4));next>>=3;next&=0x7ffff0;
		if( next<16 ) break; 
		if( next+pos>dwSize ) break;
		type = *((int*)(pdat+pos+4));type &=0x7f;
		switch( type ) {
			case 0x29 : // Bone
				m_nBone =(int)*(short*)(pdat+pos+0x12);
				for( int i=0 ; i<m_nBone ; i++ ) {
					m_Bones[i].LoadBone( pdat+pos+0x14+(i*30));
				}
				for( int i=0 ; i<m_nBone ; i++ ) {
					m_Bones[i].m_pParent = (CBone*)&m_Bones[m_Bones[i].m_mParent];
				}
				break;
		}
		pos+=next;
	}
	// 終了
	delete[] pdat;
	return hr;
}

//======================================================================
//
//		メッシュの読み込み
//
//	input
//		char *filename			: 読み込みファイル名（リソース名でも可
//		unsigned long FVF		: メッシュのFVF
//
//	output
//		エラー文字列へのポインタ。
//		正常終了の場合はNULL。
//
//======================================================================
HRESULT CModel::LoadMeshFromFile( char *FileName, unsigned long FVF,int PartsNo )
{
#pragma pack(push,2)
	typedef struct{
	  char		name[16];			//      名前
	  BYTE		Ver;				//0x00
	  BYTE		nazo;				//0x01
	  WORD		Type;				//0x02 &7f==0モデル 1=クロス
	  WORD		Flip;				//0x04 0==OFF  ON
	} DAT2AH ;
#pragma pack(pop)
	char	path[512];
	DAT2AH	*pHeader;
	HRESULT hr							= S_OK;

	//====================================================
	// ファイルをメモリに取り込む
	//====================================================
	char *pdat=NULL;
	int dwSize;
	unsigned long	cnt;
	strcpy(path, FileName);
	if (strlen(g_meshPath) > 0) {
		convert_path(path, g_meshPath);
	}
	HANDLE hFile = CreateFile(path,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_ARCHIVE,NULL);
	if( hFile!=INVALID_HANDLE_VALUE ){
		dwSize = GetFileSize(hFile,NULL);
	    pdat = new char[dwSize]();
	    ReadFile(hFile,pdat,dwSize,&cnt,NULL);
	    CloseHandle(hFile);
		hr = 0;
	} else {
		return -1;
	}

	//====================================================
	// メッシュの読み込み
	//====================================================
	int			type,pos=0,next;
	while( pos<dwSize ) {
		next = *((int*)(pdat+pos+4));next>>=3;next&=0x7ffff0;
		if( next<16 ) break; 
		if( next+pos>dwSize ) break;
		type = *((int*)(pdat+pos+4));type &=0x7f;
		switch( type ) {
		case 0x2a : //Mesh
			CMesh		*pMesh;
			pHeader = (DAT2AH*)(pdat+pos);
			pMesh = new CMesh;
//			pMesh->m_pParent = (CBone*)m_Bones;
			switch( pHeader->name[3] ) {
			case 'h':
				pMesh->SetPartsNo( 1 );break;
			case 'm':
				pMesh->SetPartsNo( 2 );break;
			case 'b':
				pMesh->SetPartsNo( 3 );break;
			case 'g':
				pMesh->SetPartsNo( 4 );break;
			case 'l':
				pMesh->SetPartsNo( 5 );break;
			case 'f':
				pMesh->SetPartsNo( 6 );break;
			default:
				pMesh->SetPartsNo( (short)PartsNo );
			}
			pMesh->LoadMesh( pdat+pos, this, FVF, 0 );
			if( pMesh->m_lpVB1==NULL || pMesh->m_lpVB2==NULL ) {
				SAFE_DELETE( pMesh );
			} else {
				m_Meshs.InsertEnd( pMesh ); m_nMesh++;
			}
			break;
		}
		pos+=next;
	}
	delete[] pdat;
//	return hr;
	pos = 0;
	hFile = CreateFile(FileName,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_ARCHIVE,NULL);
	if( hFile!=INVALID_HANDLE_VALUE ){
		dwSize = GetFileSize(hFile,NULL);
	    pdat = new char[dwSize]();
	    ReadFile(hFile,pdat,dwSize,&cnt,NULL);
	    CloseHandle(hFile);
		hr = 0;
	} else {
		return -1;
	}

	//====================================================
	// メッシュの読み込み
	//====================================================
	while( pos<dwSize ) {
		next = *((int*)(pdat+pos+4));next>>=3;next&=0x7ffff0;
		if( next<16 ) break; 
		if( next+pos>dwSize ) break;
		type = *((int*)(pdat+pos+4));type &=0x7f;
		switch( type ) {
		case 0x2a : //Mesh
			CMesh		*pMesh;
			pHeader = (DAT2AH*)(pdat+pos);
			if( pHeader->Flip ) {
				pMesh = new CMesh;
//				pMesh->m_pParent = (CBone*)m_Bones;
				switch( pHeader->name[3] ) {
				case 'h':
					pMesh->SetPartsNo( 1 );break;
				case 'm':
					pMesh->SetPartsNo( 2 );break;
				case 'b':
					pMesh->SetPartsNo( 3 );break;
				case 'g':
					pMesh->SetPartsNo( 4 );break;
				case 'l':
					pMesh->SetPartsNo( 5 );break;
				case 'f':
					pMesh->SetPartsNo( 6 );break;
				default:
					pMesh->SetPartsNo( (short)PartsNo );
				}
				pMesh->SetFlipFlag( true );
				pMesh->LoadMesh( pdat+pos, this, FVF, 1 );
				if( pMesh->m_lpVB1==NULL || pMesh->m_lpVB2==NULL ) {
					SAFE_DELETE( pMesh );
				} else {
					m_Meshs.InsertEnd( pMesh ); m_nMesh++;
				}
			}
			break;
		}
		pos+=next;
	} 
	// 終了
	delete[] pdat;
	return hr;
}
//======================================================================
//
//		メッシュインフォの読み込み
//
//	input
//		char *filename			: 読み込みファイル名（リソース名でも可
//		int  Offset				: 先頭からのオフセット
//
//	output
//		値。
//		インフォがない場合は０。
//
//======================================================================
int CModel::LoadInfoFromFile( char *FileName, int Offset )
{
	//====================================================
	// ファイルをメモリに取り込む
	//====================================================
	char *pdat=NULL;
	int dwSize;
	unsigned long	cnt;
	HANDLE hFile = CreateFile(FileName,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_ARCHIVE,NULL);
	if( hFile!=INVALID_HANDLE_VALUE ){
		dwSize = GetFileSize(hFile,NULL);
	    pdat = new char[dwSize]();
	    ReadFile(hFile,pdat,dwSize,&cnt,NULL);
	    CloseHandle(hFile);
	} else {
		return -1;
	}

	//====================================================
	// メッシュの読み込み
	//====================================================
	int			val,type,pos=0,next;
	while( pos<dwSize ) {
		next = *((int*)(pdat+pos+4));next>>=3;next&=0x7ffff0;
		if( next<16 ) break; 
		if( next+pos>dwSize ) break;
		type = *((int*)(pdat+pos+4));type &=0x7f;
		switch( type ) {
			case 0x45 : // Info
				val = (int)*(pdat+pos+Offset);
				return val;
		}
		pos+=next;
	}
	delete[] pdat;
	return 0;
}

//======================================================================
//
//		モーションの読み込み
//
//	input
//		char *filename			: 読み込みファイル名（リソース名でも可
//		unsigned long FVF		: メッシュのFVF
//
//	output
//		エラー文字列へのポインタ。
//		正常終了の場合はNULL。
//
//======================================================================
HRESULT CModel::LoadMotionFromFile( char *FileName )
{
	HRESULT hr							= S_OK;

	//====================================================
	// ファイルをメモリに取り込む
	//====================================================
	char *pdat=NULL;
	int dwSize;
	unsigned long	cnt;
	HANDLE hFile = CreateFile(FileName,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_ARCHIVE,NULL);
	if( hFile!=INVALID_HANDLE_VALUE ){
		dwSize = GetFileSize(hFile,NULL);
	    pdat = new char[dwSize]();
	    ReadFile(hFile,pdat,dwSize,&cnt,NULL);
	    CloseHandle(hFile);
		hr = 0;
	} else {
		return -1;
	}

	//====================================================
	// モーションの読み込み
	//====================================================
	int			type,pos=0,next;
	while( pos<dwSize ) {
		next = *((int*)(pdat+pos+4));next>>=3;next&=0x7ffff0;
		if( next<16 ) break; 
		if( next+pos>dwSize ) break;
		type = *((int*)(pdat+pos+4));type &=0x7f;
		switch( type ) {
			case 0x2b : // Motion
				{
					CMotionFrame *pMotionFrame;
					pMotionFrame = (CMotionFrame*)m_motions.Top();
					while( pMotionFrame ) {
						if( !strncmp(pdat+pos,pMotionFrame->GetMotionName(),4) ) {
							pMotionFrame->InitData();
							m_motions.Erase(pMotionFrame);
						}
						pMotionFrame = (CMotionFrame*)pMotionFrame->Next;
					}
					pMotionFrame = new CMotionFrame;
					pMotionFrame->LoadMotionSet( pdat+pos );
					m_motions.InsertEnd(pMotionFrame);
				}
				break;
		}
		pos+=next;
	}
	// 終了
	delete[] pdat;
	return hr;
}

//======================================================================
//
//		モーションの3文字検索・設定（一致するものすべて）
//
//	input
//		char *MotionName		: 検索モーション名
//
//	output
//		正常終了の場合はOK。
//
//======================================================================
bool CModel::SetMotion3FromList( int Type, char *MotionName )
{
	bool rc = false;
	CMotionFrame *pMotionFrame = (CMotionFrame*)m_motions.Top();
	while( pMotionFrame ) {
		if( !strncmp(MotionName,pMotionFrame->m_Name,3) ) {
			CMotionElement *pMotionElement = (CMotionElement*)pMotionFrame->m_MotionEList.Top();
			while( pMotionElement ) {
				if( Type == 0 ) 
					m_MotionArray[pMotionElement->GetBoneNo()].CopyMotionElement( pMotionElement );
				else
					m_MotionArray[pMotionElement->GetBoneNo()].AddMotionElement( pMotionElement );					
				pMotionElement = (CMotionElement*)pMotionElement->Next;
			}
			rc = true;
		}
		pMotionFrame = (CMotionFrame*)pMotionFrame->Next;
	}
	return rc;
}

//======================================================================
//
//		モーションの4文字検索・設定（4文字完全一致のみ）
//
//	input
//		char *MotionName		: 検索モーション名
//
//	output
//		正常終了の場合はOK。
//
//======================================================================
bool CModel::SetMotion4FromList( int Type, char *MotionName )
{
	CMotionFrame *pMotionFrame = (CMotionFrame*)m_motions.Top();
	while( pMotionFrame ) {
		if( !strncmp(MotionName,pMotionFrame->m_Name,4) ) {
			CMotionElement *pMotionElement = (CMotionElement*)pMotionFrame->m_MotionEList.Top();
			while( pMotionElement ) {
				if( Type == 0 ) 
					m_MotionArray[pMotionElement->GetBoneNo()].CopyMotionElement( pMotionElement );
				else
					m_MotionArray[pMotionElement->GetBoneNo()].AddMotionElement( pMotionElement );					
				pMotionElement = (CMotionElement*)pMotionElement->Next;
			}
			return true;
		}
		pMotionFrame = (CMotionFrame*)pMotionFrame->Next;
	}
	return false;
}

//======================================================================
//
//		モデルの行列初期化
//
//		全フレームの行列を初期化します。
//
//======================================================================
void CModel::InitTransform( void )
{
	D3DXMatrixIdentity( &m_mRootTransform );

	for( int i=0 ; i<128 ; i++ ) {
		D3DXMatrixIdentity( &m_Bones[i].m_mWorld );
	}
}
//======================================================================
//		モデルの行列更新
//		全フレームの行列を算出します。
//======================================================================
void CModel::InitialTransform(void)
{
	int i;

	for( i=0 ; i<m_nBone ; i++ ) {
		m_Bones[i].m_mWorld = m_Bones[i].m_mTransform;
	}
	for (i = 0; i<m_nBone; i++) {
		if( m_Bones[i].m_pParent ) 
			m_Bones[i].m_mWorld *= ((CBone*)m_Bones[i].m_pParent)->m_mWorld;
	}
	for( i=0 ; i<m_nBone ; i++ ) {
		//m_Bones[i].m_mWorld *= m_mRootTransform;
		D3DXMatrixInverse(&m_Bones[i].m_mInvTrans, 0, &m_Bones[i].m_mWorld);
	}
}


//======================================================================
//		モデルの行列更新
//		全フレームの行列を算出します。
//======================================================================
void CModel::DynamicTransform(void)
{
	int i;

	for (i = 0; i<m_nBone; i++) {
		m_Bones[i].m_mWorld = m_Bones[i].m_mTransform;
		m_Bones[i].m_mWorld *= *(m_MotionArray[i].GetMotionMatrix(m_Time, &m_Bones[i].m_mTransform));
	}
	for (i = 0; i<m_nBone; i++) {
		if (m_Bones[i].m_pParent)
			m_Bones[i].m_mWorld *= ((CBone*)m_Bones[i].m_pParent)->m_mWorld;
	}
	for (i = 0; i<m_nBone; i++) {
		m_Bones[i].m_mWorld *= m_mRootTransform;
	}
}


//======================================================================
//
//		フレームの最大時間を算出
//
//		全フレームの行列を算出します。
//
//======================================================================
float CModel::MaxMotionTime( void )
{
	float Time,MaxTime = 0.f;

	for( int i=0 ; i<m_nBone ; i++ ) {
		CMotionElement *pAnime = m_Bones[i].m_pMotion;
		if ( pAnime != NULL ) {
			Time = pAnime->GetFinalTime();
			MaxTime =MaxTime > Time ? MaxTime:Time;
		}
	}
	return MaxTime;
}

//======================================================================
//
//		レンダリング
//
//		モデルデータを親子関係を含めてレンダリングします。
//
//	output
//		レンダリングしたポリゴン数
//
//======================================================================
unsigned long CModel::Rendering( void )
{
	int				DispCheck;
	unsigned long	count = 0;

	GetDevice()->SetRenderState(D3DRS_LIGHTING, FALSE);   // 発電所を回す！
	//GetDevice()->SetRenderState(D3DRS_AMBIENT, 0xff030303);   // 世の中をちょっと白く照らす
	GetDevice()->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	GetDevice()->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	GetDevice()->SetRenderState(D3DRS_STENCILENABLE, FALSE);
	GetDevice()->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
	GetDevice()->SetRenderState(D3DRS_ALPHAREF, 0x01);
	GetDevice()->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);
	//GetDevice()->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);
	//GetDevice()->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	//GetDevice()->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	//GetDevice()->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE2X);
	//GetDevice()->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	//GetDevice()->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	//GetDevice()->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
	//GetDevice()->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE2X);
	//GetDevice()->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	//GetDevice()->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	//GetDevice()->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	//GetDevice()->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);

	if( g_mDispWire ) {
		GetDevice()->SetRenderState( D3DRS_FILLMODE,D3DFILL_WIREFRAME );
	} else {
		GetDevice()->SetRenderState( D3DRS_FILLMODE,D3DFILL_SOLID );
	}
	GetDevice()->SetTransform(D3DTS_VIEW, &g_mView);
	GetDevice()->SetTransform(D3DTS_PROJECTION, &g_mProjection);

	//---------------------------------------------------------
	// 頂点処理
	//---------------------------------------------------------
	IDirect3DVertexDeclaration9	*VertexFormat; 
	GetDevice()->SetSoftwareVertexProcessing(g_mIsUseSoftware);
	GetDevice()->CreateVertexDeclaration(VS0Formats, &VertexFormat);
	GetDevice()->SetVertexDeclaration(VertexFormat);
	int				indx1,indx2;
	D3DXVECTOR4		p1,n1;
	D3DXMATRIX		mat,mat1,mat2;
	CUSTOMVERTEX	*pV1, *pV2;
	CMesh *pMesh = (CMesh*)m_Meshs.Top();
	while ( pMesh != NULL )
	{
		DispCheck = pMesh->GetDispCheck();

		//---------------------------------------------------------
		// 頂点バッファ1をデバイスに設定
		//---------------------------------------------------------
		(pMesh->m_lpVB1)->Lock(0, pMesh->m_VBSize, (void**)&pV1, D3DLOCK_DISCARD);
		(pMesh->m_lpVB2)->Lock(0, pMesh->m_VBSize, (void**)&pV2, D3DLOCK_DISCARD);
		for (unsigned int i = 0; i<pMesh->m_NumVertices; i++, pV1++, pV2++) {
			indx1 = pV1->indx; if (indx1>pMesh->m_mBoneNum || indx1 <0) indx1 = 0;
			indx2 = pV2->indx; if (indx2>pMesh->m_mBoneNum || indx2 <0) indx2 = 0;
			mat1 = m_Bones[pMesh->m_pBoneTbl[indx1]].m_mInvTrans*m_Bones[pMesh->m_pBoneTbl[indx1]].m_mWorld*pV1->b1;
			mat2 = m_Bones[pMesh->m_pBoneTbl[indx2]].m_mInvTrans*m_Bones[pMesh->m_pBoneTbl[indx2]].m_mWorld*pV2->b1;
			mat = mat1 + mat2;
			p1.x = pV1->p.x; p1.y = pV1->p.y; p1.z = pV1->p.z; p1.w = 1.0;
			D3DXVec4Transform(&p1, &p1, &mat);
			n1.x = pV1->n.x; n1.y = pV1->n.y; n1.z = pV1->n.z; n1.w = 1.0;
			D3DXVec4Transform(&n1, &n1, &mat);
			D3DXVec4Normalize(&n1, &n1);
			pV2->p.x = p1.x; pV2->p.y = p1.y; pV2->p.z = p1.z;
			pV2->n.x = n1.x; pV2->n.y = n1.y; pV2->n.z = n1.z;
		}
		(pMesh->m_lpVB2)->Unlock();
		(pMesh->m_lpVB1)->Unlock();

		//---------------------------------------------------------
		// 頂点バッファ1をデバイスに設定
		//---------------------------------------------------------
		GetDevice()->SetStreamSource(0, pMesh->m_lpVB2, 0, D3DXGetFVFVertexSize(pMesh->m_FVF));
		//---------------------------------------------------------
		// インデックスバッファをデバイスに設定
		//---------------------------------------------------------
		GetDevice()->SetIndices(pMesh->m_lpIB);

		CStream		*pStream;
		//---------------------------------------------------------
		// サーフェイスごとにレンダリング
		//---------------------------------------------------------
		int stno = 0;
		pStream = (CStream*)pMesh->m_Streams.Top();
		while (pStream != NULL) {
			stno++;
			int	DispLevel = pStream->GetDispLevel();
			if (g_mPCFlag && DispLevel != 0 && DispLevel < DispCheck) {
				pStream = (CStream*)pStream->Next;
				continue;
			}

			//
			// c14 : マテリアル
			//
			CMaterial *pMaterial = (CMaterial*)pStream->m_pMaterial;

			// デバイスにテクスチャ設定
			if (pMaterial)
				GetDevice()->SetTexture(0, pMaterial->GetTexture());
			else
				GetDevice()->SetTexture(0, NULL);
			GetDevice()->DrawIndexedPrimitive(
				pStream->GetPrimitiveType(),
				0,
				0,
				pMesh->m_NumVertices,
				pStream->GetIndexStart(),
				pStream->GetFaceCount());
			pStream = (CStream*)pStream->Next;
		}
		count += pMesh->m_NumFaces;
		pMesh = (CMesh*)pMesh->Next;

	}
	//GetDevice()->SetRenderState( D3DRS_ALPHATESTENABLE,		FALSE );
	//GetDevice()->SetRenderState( D3DRS_CULLMODE,			D3DCULL_NONE );
	return count;
}

//======================================================================
//
//		レンダリング
//
//		ボーンデータをレンダリングします。
//======================================================================
void CModel::BoneRendering( void )
{
	D3DXVECTOR3		pos[3];
	D3DXMATRIX		mat;

//	// 変換行列
	GetDevice()->SetRenderState(D3DRS_LIGHTING, FALSE);   // 発電所を回す！
	GetDevice()->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	GetDevice()->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	GetDevice()->SetRenderState(D3DRS_STENCILENABLE, FALSE);
	GetDevice()->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
	GetDevice()->SetTransform(D3DTS_VIEW, &g_mView);
	GetDevice()->SetTransform(D3DTS_PROJECTION, &g_mProjection);
	GetDevice()->SetFVF(D3DFVF_XYZ);
	for (int i = 0; i < m_nBone; i++) {
		// child bone
		if (m_Bones[i].m_pParent == NULL) continue;
		mat = ((CBone*)m_Bones[i].m_pParent)->m_mWorld;
		pos[0].x = mat._41; pos[0].y = mat._42; pos[0].z = mat._43;
		mat = m_Bones[i].m_mWorld;
		pos[1].x = mat._41; pos[1].y = mat._42; pos[1].z = mat._43;
		GetDevice()->DrawPrimitiveUP(D3DPT_LINELIST, 2, pos, sizeof(D3DXVECTOR3));
	}
	return;
}



bool CModel::ConvertMesh(void) {

	CMesh *pMesh = (CMesh*)m_Meshs.Top();
	while (pMesh != NULL)
	{
		int				indx;
		D3DXVECTOR4		pos, p1, p2;
		D3DXVECTOR3		norm,n1, n2;
		D3DXMATRIX		mat;
		CUSTOMVERTEX	*pV1, *pV2;
		D3DXMatrixIdentity(&mat);
		(pMesh->m_lpVB1)->Lock(0, pMesh->m_VBSize, (void**)&pV1, D3DLOCK_DISCARD);
		(pMesh->m_lpVB2)->Lock(0, pMesh->m_VBSize, (void**)&pV2, D3DLOCK_DISCARD);
		for (unsigned int i = 0; i<pMesh->m_NumVertices; i++, pV1++, pV2++) {
			indx = pV1->indx; if (indx>pMesh->m_mBoneNum || indx <0) indx = 0;
			mat = m_Bones[pMesh->m_pBoneTbl[indx]].m_mWorld;
			p1.x = pV1->p.x; p1.y = pV1->p.y; p1.z = pV1->p.z; p1.w = pV1->b1;
			D3DXVec4Transform(&p1, &p1, &mat);
			n1.x = pV1->n.x; n1.y = pV1->n.y; n1.z = pV1->n.z;
			D3DXVec3TransformCoord(&n1, &n1, &mat);
			indx = pV2->indx; if (indx>pMesh->m_mBoneNum || indx <0) indx = 0;
			mat = m_Bones[pMesh->m_pBoneTbl[indx]].m_mWorld;
			p2.x = pV2->p.x; p2.y = pV2->p.y; p2.z = pV2->p.z; p2.w = pV2->b1;
			D3DXVec4Transform(&p2, &p2, &mat);
			n2.x = pV2->n.x; n2.y = pV2->n.y; n2.z = pV2->n.z;
			D3DXVec3TransformCoord(&n2, &n2, &mat);
			pos = (p1 + p2); norm = n1 + n2;
			D3DXVec3Normalize(&norm, &norm);
			pV1->p.x = pV2->p.x = pos.x;
			pV1->p.y = pV2->p.y = pos.y;
			pV1->p.z = pV2->p.z = pos.z;
			pV1->n.x = pV2->n.x = norm.x;
			pV1->n.y = pV2->n.y = norm.y;
			pV1->n.z = pV2->n.z = norm.z;
			pV2->u = pV1->u; pV2->v = pV1->v;
		}
		(pMesh->m_lpVB2)->Unlock();
		(pMesh->m_lpVB1)->Unlock();
		pMesh = (CMesh*)pMesh->Next;
	}
	return true;
}
//------------------------------------------------------------------------------------------//
//																							//
//																							//
//									CPC													//
//																							//
//																							//
//------------------------------------------------------------------------------------------//

//======================================================================
//
//		コンストラクタ
//
//======================================================================
CPC::CPC()
{
	m_mType				= 0;
	m_mRace				= 0;
	m_mFace				= 0;
	m_mHead				= 0;
	m_mBody				= 0;
	m_mHand				= 0;
	m_mLegs				= 0;
	m_mFoot				= 0;
	m_mInfoBody			= 0;
	m_mRightWeapon		= 0;
	m_mLeftWeapon		= 0;
	m_mRemoteWeapon		= 0;
	m_MotionLevel		= 0;
	m_MotionOffset		= 0;
	m_mInfoRWeapon		= 0;
	m_mInfoRWeapon2		= 0;
	m_mInfoLWeapon		= 0;
	m_mInfoLWeapon2		= 0;

}

//======================================================================
//
//		デストラクタ
//
//======================================================================
CPC::~CPC()
{
}
//======================================================================
//
//		ボーンのカウント(ファイルが有効化の判定）
//
//======================================================================
int CPC::CountBoneFromFile( char *FileName )
{
	//====================================================
	// ファイルをメモリに取り込む
	//====================================================
	char *pdat=NULL;
	int dwSize;
	unsigned long	cnt;
	HANDLE hFile = CreateFile(FileName,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_ARCHIVE,NULL);
	if( hFile!=INVALID_HANDLE_VALUE ){
		dwSize = GetFileSize(hFile,NULL);
	    pdat = new char[dwSize]();
	    ReadFile(hFile,pdat,dwSize,&cnt,NULL);
	    CloseHandle(hFile);
	} else {
		return 0;
	}
	//====================================================
	// ボーンのカウント
	//====================================================
	int		type,pos=0,next,count=0;
	while( pos<dwSize ) {
		next = *((int*)(pdat+pos+4));next>>=3;next&=0x7ffff0;
		if( next<16 ) break; 
		if( next+pos>dwSize ) break;
		type = *((int*)(pdat+pos+4));type &=0x7f;
		switch( type ) {
		case 0x29 :
			count++;
			break;
		}
		pos+=next;
	}
	delete[] pdat;
	return count;
}

//======================================================================
//
//		テクスチャのカウント(ファイルが有効化の判定）
//
//======================================================================
int CPC::CountTextureFromFile( char *FileName )
{
	//====================================================
	// ファイルをメモリに取り込む
	//====================================================
	char *pdat=NULL;
	int dwSize;
	unsigned long	cnt;
	HANDLE hFile = CreateFile(FileName,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_ARCHIVE,NULL);
	if( hFile!=INVALID_HANDLE_VALUE ){
		dwSize = GetFileSize(hFile,NULL);
	    pdat = new char[dwSize]();
	    ReadFile(hFile,pdat,dwSize,&cnt,NULL);
	    CloseHandle(hFile);
	} else {
		return 0;
	}
	//====================================================
	// テクスチャのカウント
	//====================================================
	int		type,pos=0,next,count=0;
	while( pos<dwSize ) {
		next = *((int*)(pdat+pos+4));next>>=3;next&=0x7ffff0;
		if( next<16 ) break; 
		if( next+pos>dwSize ) break;
		type = *((int*)(pdat+pos+4));type &=0x7f;
		switch( type ) {
		case 0x20 :
			count++;
			break;
		}
		pos+=next;
	}
	delete[] pdat;
	return count;
}

bool CPC::SetInventory( int Tag, int Data ) 
{
	switch( Tag ) {
		case 0x10:
			SetType( Data );		// タイプ
			break;
		case 0x20:
			SetRace( Data );		// ボーン
			break;
		case 0x30:
			SetFace( Data );		// 顔
			break;
		case 0x40:
			SetHead( Data );		// 頭
			break;
		case 0x50:
			SetBody( Data );		// 胴
			break;
		case 0x60:
			SetHand( Data );		// 手
			break;
		case 0x70:
			SetLegs( Data );		// 脚
			break;
		case 0x80:
			SetFoot( Data );		// 足
			break;
		case 0x90:
			SetRightWeapon( Data );		// 右手武器
			break;
		case 0xa0:
			SetLeftWeapon( Data );		// 左手武器
			break;
		case 0xb0:
			SetRemoteWeapon( Data );		// 遠隔武器
			break;
		default:
			return false;
	}
	return true;
}

void CPC::SetInventoryInit( void )
{
	SetType( 0 );
	SetRace( 0 );
	SetFace( 0 );
	SetHead( 0 );
	SetBody( 0 );
	SetHand( 0 );
	SetLegs( 0 );
	SetFoot( 0 );
	SetRightWeapon( 0 );
	SetLeftWeapon( 0 );
	SetRemoteWeapon( 0 );
}

//======================================================================
//
//		モデル属性読み込み
//
//======================================================================
bool CPC::LoadInventory( char *FileName )
{
	DWORD	ver,dmy,dwSize;
	char	*pFile;

	HANDLE hFile = CreateFile(FileName,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_ARCHIVE,NULL);
	if( hFile == INVALID_HANDLE_VALUE ) return false;
	dwSize = GetFileSize(hFile,NULL);
	if( (pFile = new char[dwSize]()) == NULL ) return false;
	ReadFile(hFile,pFile,dwSize,&dmy,NULL);
	CloseHandle(hFile);
	int *ptr = (int*)pFile;
	ver =*ptr;
	if( (ver&0xff000000) != 0xfe000000 ) return false;
	SetInventoryInit();
	int ItemNum = *(ptr+2); ptr += 3;
	for( int i=0 ; i<ItemNum ; i++ ) {
		int Tag			=	*ptr++;
		int Offset		=	*ptr++;
		int DataNum		=	*ptr++;
		int ItemData	=	*((int*)(pFile+Offset));
		if( (ver&0x00ffffff) == 0x002000 && Tag!=0x10 ) 
			ItemData = _GetFileNameFromIDsub(1,ItemData);
		SetInventory( Tag, ItemData );
	}
	SAFE_DELETES( pFile );
	return true;
}

typedef struct {
	int	Tag,Offset,DataNum;
} FILEITEM ;

typedef struct {
	int	FileID,FileSize,ItemNum;
	FILEITEM	FileItem[8];
	int	ItemData[8];
} FILEDATA ;


bool CPC::LoadDefaultMotion( void )
{
	char	FileName[128];

	GetFileNameFromDno( FileName, (DWORD)(0x10000+g_mBaseMotionFileNo[GetType()]) );
	LoadMotionFromFile( FileName );
	// Body 1
	if( GetInfoLWeapon()==1 ) 
		GetFileNameFromDno( FileName, (DWORD)(0x10000+g_mBaseMotionFileNo[GetType()]+2) );
	else 
		GetFileNameFromDno( FileName, (DWORD)(0x10000+g_mBaseMotionFileNo[GetType()]+1) );
	LoadMotionFromFile( FileName );
	// Legs 2
	if( GetInfoBody()==2 ) 
		GetFileNameFromDno( FileName, (DWORD)(0x10000+g_mBaseMotionFileNo[GetType()]+4) );
	else 
		GetFileNameFromDno( FileName, (DWORD)(0x10000+g_mBaseMotionFileNo[GetType()]+3) );
	LoadMotionFromFile( FileName );
	return true;
}

//======================================================================
//
//		モデル読み込み
//
//======================================================================
bool CPC::LoadPCParts()
{
	unsigned long FVF = (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1);
	HRESULT hr;
	char FileName[512];


	//==============================================================
	// データ初期化
	//==============================================================
	SetTime( 0.f );
	InitPartsData();
	InitMotionData();
	SetInfoLWeapon(-1);
	SetInfoLWeapon2(-1);
	SetInfoRWeapon(-1);
	SetInfoRWeapon2(-1);
	SetInfoBody(-1);
	if( (GetRace()&0xffff) == 0 ) return false;
	//==============================================================
	//  ファイルのロード
	//==============================================================
	// ボーンのロード
	GetFileNameFromDno(FileName,GetRace());
	if( CountBoneFromFile( FileName ) <=0 ) return false;
	hr = LoadBoneFromFile( FileName );


	// 顔　テクスチャ、メッシュのロード
	GetFileNameFromDno(FileName,GetFace());
	if( CountTextureFromFile( FileName ) > 0 ) {
		hr = LoadTextureFromFile( FileName );
		hr = LoadMeshFromFile( FileName, FVF, 1 );
	}
	// 防具　頭　テクスチャ、メッシュのロード
	GetFileNameFromDno(FileName,GetHead());
	if( CountTextureFromFile( FileName ) > 0 ) {
		hr = LoadTextureFromFile( FileName );
		hr = LoadMeshFromFile( FileName, FVF, 2 );
	}
	// 防具　胴　テクスチャ、メッシュのロード
	GetFileNameFromDno(FileName,GetBody());
	if( CountTextureFromFile( FileName ) > 0 ) {
		hr = LoadTextureFromFile( FileName );
		hr = LoadMeshFromFile( FileName, FVF, 3 );
		SetInfoBody( LoadInfoFromFile( FileName, 0x19) );
	}
	// 防具　手　テクスチャ、メッシュのロード
	GetFileNameFromDno(FileName,GetHand());
	if( CountTextureFromFile( FileName ) > 0 ) {
		hr = LoadTextureFromFile( FileName );
		hr = LoadMeshFromFile( FileName, FVF, 4 );
	}
	// 防具　脚　テクスチャ、メッシュのロード
	GetFileNameFromDno(FileName,GetLegs());
	if( CountTextureFromFile( FileName ) > 0 ) {
		hr = LoadTextureFromFile( FileName );
		hr = LoadMeshFromFile( FileName, FVF, 5 );
	}
	// 防具　足　テクスチャ、メッシュのロード
	GetFileNameFromDno(FileName,GetFoot());
	if( CountTextureFromFile( FileName ) > 0 ) {
		hr = LoadTextureFromFile( FileName );
		hr = LoadMeshFromFile( FileName, FVF, 6 );
	}
	//武器　右手　　テクスチャ、メッシュのロード
	GetFileNameFromDno(FileName,GetRightWeapon());
	if( CountTextureFromFile( FileName ) > 0 ) {
		hr = LoadTextureFromFile( FileName );
		hr = LoadMeshFromFile( FileName, FVF, 0 );
	}
	SetInfoRWeapon( LoadInfoFromFile( FileName , 0x13 ) );
	SetInfoRWeapon2( LoadInfoFromFile( FileName , 0x16 ) );
	//武器　左手　　テクスチャ、メッシュのロード
	GetFileNameFromDno(FileName,GetLeftWeapon());
	if( CountTextureFromFile( FileName ) > 0 ) {
		hr = LoadTextureFromFile( FileName );
		hr = LoadMeshFromFile( FileName, FVF, 0 );
	}
	SetInfoLWeapon( LoadInfoFromFile( FileName , 0x15 ) );
	SetInfoLWeapon2( LoadInfoFromFile( FileName , 0x13 ) );
	//遠隔　　　テクスチャ、メッシュのロード
	GetFileNameFromDno(FileName,GetRemoteWeapon());
	if( CountTextureFromFile( FileName ) > 0 ) {
		hr = LoadTextureFromFile( FileName );
		hr = LoadMeshFromFile( FileName, FVF, 0 );
	}
	CMesh *pMesh1;
	CMesh *pMesh2;
	pMesh1 = (CMesh*)m_Meshs.Top();
	while ( pMesh1 != NULL ){
		if( pMesh1->GetPartsNo() == 2 ) {
			pMesh2 = (CMesh*)m_Meshs.Top();
			while ( pMesh2 != NULL ) {
				if( pMesh2->GetPartsNo()==1 ) {
					if( pMesh2->GetDispCheck()<pMesh1->GetDispLevel()) 
						pMesh2->SetDispCheck( (short)pMesh1->GetDispLevel() );
				}
				pMesh2 = (CMesh*)pMesh2->Next;
			}
		}
		if( pMesh1->GetPartsNo() == 3 && pMesh1->GetDispLevel() == 0x12 ) {
			pMesh2 = (CMesh*)m_Meshs.Top();
			while ( pMesh2 != NULL ) {
				if( pMesh2->GetPartsNo()==4 )
					pMesh2->SetDispCheck( 6 );
				pMesh2 = (CMesh*)pMesh2->Next;
			}
		}
		if( pMesh1->GetPartsNo() == 5 && pMesh1->GetDispLevel() == 0x22 ) {
			pMesh2 = (CMesh*)m_Meshs.Top();
			while ( pMesh2 != NULL ) {
				if( pMesh2->GetPartsNo()==6 )
					pMesh2->SetDispCheck( 8 );
				pMesh2 = (CMesh*)pMesh2->Next;
			}
		}
		if( pMesh1->GetPartsNo() == 6 && pMesh1->GetDispLevel() == 0x32 ) {
			pMesh2 = (CMesh*)m_Meshs.Top();
			while ( pMesh2 != NULL ) {
				if( pMesh2->GetPartsNo()==5 && pMesh2->GetDispLevel() == 0x21 )
					pMesh2->SetDispCheck( 7 );
				pMesh2 = (CMesh*)pMesh2->Next;
			}
		}
		pMesh1 = (CMesh*)pMesh1->Next;
	}
	return true;
}

//======================================================================
//
//		モデル読み込み
//
//======================================================================
bool CPC::LoadPCMotion()
{
	//unsigned long FVF = (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1);
	//unsigned long FVF1 = (D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_DIFFUSE|D3DFVF_TEX1);
	char	FileName[128];
	BYTE	idx;
	InitMotionData();
	LoadDefaultMotion();
	switch (g_mPCMotion) {
		case 1:
			idx = GetInfoRWeapon() & 0x0f;
			if (idx == g_mkatanaINFO[GetType()]) {
				GetFileNameFromDno(FileName, (DWORD)(0x10000 +
					g_mMotionB2FileNo[GetType()] + 1));
				LoadMotionFromFile(FileName);
				if (GetInfoBody() == 2)
					GetFileNameFromDno(FileName, (DWORD)(0x10000 +
					g_mMotionB2FileNo[GetType()] + g_mmotionB2num[GetType()] * 2 + 1));
				else
					GetFileNameFromDno(FileName, (DWORD)(0x10000 +
					g_mMotionB2FileNo[GetType()] + g_mmotionB2num[GetType()] + 1));
				LoadMotionFromFile(FileName);
			}
			else if (idx == g_msinobiINFO[GetType()]) {
				GetFileNameFromDno(FileName, (DWORD)(0x10000 +
					g_mMotionB2FileNo[GetType()]));
				LoadMotionFromFile(FileName);
				if (GetInfoBody() == 2)
					GetFileNameFromDno(FileName, (DWORD)(0x10000 +
					g_mMotionB2FileNo[GetType()] + g_mmotionB2num[GetType()] * 2));
				else
					GetFileNameFromDno(FileName, (DWORD)(0x10000 +
					g_mMotionB2FileNo[GetType()] + g_mmotionB2num[GetType()]));
				LoadMotionFromFile(FileName);
			}
			else {
				if (idx == 0xa) {
					idx = g_mmotionBnum[GetType()] - 1;
				}
				else if (idx>g_msinobiINFO[GetType()]) {
					idx--;
					if (GetType() == 3) idx--;
				}
				GetFileNameFromDno(FileName, (DWORD)(0x10000 +
					g_mMotionBFileNo[GetType()] + idx));
				LoadMotionFromFile(FileName);
				if (GetInfoBody() == 2)
					GetFileNameFromDno(FileName, (DWORD)(0x10000 +
					g_mMotionBFileNo[GetType()] + g_mmotionBnum[GetType()] * 2 + idx));
				else
					GetFileNameFromDno(FileName, (DWORD)(0x10000 +
					g_mMotionBFileNo[GetType()] + g_mmotionBnum[GetType()] + idx));
				LoadMotionFromFile(FileName);
			}
			//			if( GetInfoLWeapon()==0 && (BYTE)GetInfoLWeapon2()<g_mmotionBLnum[GetType()] ) {
			if (GetInfoLWeapon() == 0 && GetInfoLWeapon2() >= 0 && GetInfoRWeapon() >= 0) {
				GetFileNameFromDno(FileName, (DWORD)(0x10000 +
					g_mMotionBLFileNo[GetType()] + g_mBLCnvTbl[GetType()][GetInfoLWeapon2()]));
				LoadMotionFromFile(FileName);
				GetFileNameFromDno(FileName, (DWORD)(0x10000 + g_mMotionBLFileNo[GetType()] +
					g_mmotionBLnum[GetType()] * 3 + g_mBLCnvTbl[GetType()][GetInfoRWeapon()]));
				LoadMotionFromFile(FileName);
				if (GetInfoBody() == 2) {
					GetFileNameFromDno(FileName, (DWORD)(0x10000 + g_mMotionBLFileNo[GetType()] +
						g_mmotionBLnum[GetType()] * 2 + g_mBLCnvTbl[GetType()][GetInfoLWeapon2()]));
					LoadMotionFromFile(FileName);
					GetFileNameFromDno(FileName, (DWORD)(0x10000 + g_mMotionBLFileNo[GetType()] +
						g_mmotionBLnum[GetType()] * 5 + g_mBLCnvTbl[GetType()][GetInfoLWeapon2()]));
					LoadMotionFromFile(FileName);
				}
				else {
					GetFileNameFromDno(FileName, (DWORD)(0x10000 + g_mMotionBLFileNo[GetType()] +
						g_mmotionBLnum[GetType()] * 1 + g_mBLCnvTbl[GetType()][GetInfoRWeapon()]));
					LoadMotionFromFile(FileName);
					GetFileNameFromDno(FileName, (DWORD)(0x10000 + g_mMotionBLFileNo[GetType()] +
						g_mmotionBLnum[GetType()] * 4 + g_mBLCnvTbl[GetType()][GetInfoRWeapon()]));
					LoadMotionFromFile(FileName);
				}
			}
			break;
		case 2:
			GetFileNameFromDno(FileName, (DWORD)(0x10000 + g_mMotionEFileNo[GetType()] ));
			LoadMotionFromFile(FileName);
			if (GetInfoBody() == 2)
				GetFileNameFromDno(FileName, (DWORD)(0x10000 +
				g_mMotionEFileNo[GetType()] + g_mmotionEnum * 2));
			else
				GetFileNameFromDno(FileName, (DWORD)(0x10000 +
				g_mMotionAFileNo[GetType()] + g_mmotionEnum ));
			LoadMotionFromFile(FileName);
			break;
		default:
			break;
	}
	SetMotion3FromList( 0, GetMotionName() );
	return true;
}

bool	CPC::Get8010Flag( void )
{
	int		Level;
	CMesh	*pMesh;

	pMesh = (CMesh*)m_Meshs.Top();
	while ( pMesh != NULL ){
		if( pMesh->GetPartsNo() == 3  ) {
			if( (Level = pMesh->GetDispLevel()) == 0x12 ) 
				return true;
		}
		pMesh = (CMesh*)pMesh->Next;
	}
	return false;
}

//------------------------------------------------------------------------------------------//
//																							//
//																							//
//									CNPC													//
//																							//
//																							//
//------------------------------------------------------------------------------------------//

//======================================================================
//
//		コンストラクタ
//
//======================================================================
CNPC::CNPC()
{
	m_mBody				= 0;
}

//======================================================================
//
//		デストラクタ
//
//======================================================================
CNPC::~CNPC()
{
}

//======================================================================
//
//		ボーンのカウント(ファイルが有効化の判定）
//
//======================================================================
int CNPC::CountBoneFromFile( char *FileName )
{
	//====================================================
	// ファイルをメモリに取り込む
	//====================================================
	char *pdat=NULL;
	int dwSize;
	unsigned long	cnt;
	HANDLE hFile = CreateFile(FileName,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_ARCHIVE,NULL);
	if( hFile!=INVALID_HANDLE_VALUE ){
		dwSize = GetFileSize(hFile,NULL);
	    pdat = new char[dwSize]();
	    ReadFile(hFile,pdat,dwSize,&cnt,NULL);
	    CloseHandle(hFile);
	} else {
		return 0;
	}
	//====================================================
	// ボーンのカウント
	//====================================================
	int		type,pos=0,next,count=0;
	while( pos<dwSize ) {
		next = *((int*)(pdat+pos+4));next>>=3;next&=0x7ffff0;
		if( next<16 ) break; 
		if( next+pos>dwSize ) break;
		type = *((int*)(pdat+pos+4));type &=0x7f;
		switch( type ) {
		case 0x29 :
			count++;
			break;
		}
		pos+=next;
	}
	delete[] pdat;
	return count;
}

//======================================================================
//
//		モーション読み込み
//
//======================================================================
bool CNPC::LoadNPCMotion()
{
	char FileName[512];
	InitMotionData();
	if( (GetBody()&0xffff) == 0 ) return false;
	GetFileNameFromDno(FileName,GetBody());
	LoadMotionFromFile( FileName);
	SetMotion3FromList( 0, GetMotionName() );
	return true;
}
//======================================================================
//
//		NPCデータ読み込み
//
//======================================================================
bool CNPC::LoadNPC()
{
	unsigned long FVF = (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1);
	unsigned long FVF1 = (D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_DIFFUSE|D3DFVF_TEX1);
	unsigned long FVF2 = (D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1);
	char FileName[512];

	//==============================================================
	// データ初期化
	//==============================================================
	SetTime( 0.f );
	InitPartsData();
	InitMotionData();
	if( (GetBody()&0xffff) == 0 ) return false;

	//==============================================================
	//  ファイルのロード
	//==============================================================
	GetFileNameFromDno(FileName,GetBody());
	if( CountBoneFromFile( FileName ) <= 0 ) return false;
	//==============================================================
	//  ファイルのロード
	//==============================================================
	HRESULT hr;
	// ボーンのロード
	hr = LoadBoneFromFile( FileName );
	if( hr ) return false;
	//　テクスチャ、メッシュのロード
	hr = LoadTextureFromFile( FileName );
	if( hr ) return false;
	hr = LoadMeshFromFile( FileName , FVF, 0 );
	if( hr ) return false;
	CMesh *pMesh1;
	CMesh *pMesh2;
	pMesh1 = (CMesh*)m_Meshs.Top();
	while ( pMesh1 != NULL ){
		if( pMesh1->GetPartsNo() == 2 ) {
			pMesh2 = (CMesh*)m_Meshs.Top();
			while ( pMesh2 != NULL ) {
				if( pMesh2->GetPartsNo()==1 ) {
					if( pMesh2->GetDispCheck()<pMesh1->GetDispLevel()) 
						pMesh2->SetDispCheck( (short)pMesh1->GetDispLevel() );
				}
				pMesh2 = (CMesh*)pMesh2->Next;
			}
		}
		if( pMesh1->GetPartsNo() == 3 && pMesh1->GetDispLevel() == 0x12 ) {
			pMesh2 = (CMesh*)m_Meshs.Top();
			while ( pMesh2 != NULL ) {
				if( pMesh2->GetPartsNo()==4 )
					pMesh2->SetDispCheck( 6 );
				pMesh2 = (CMesh*)pMesh2->Next;
			}
		}
		if( pMesh1->GetPartsNo() == 5 && pMesh1->GetDispLevel() == 0x22 ) {
			pMesh2 = (CMesh*)m_Meshs.Top();
			while ( pMesh2 != NULL ) {
//				if( pMesh2->GetPartsNo()==6 && pMesh2->GetDispLevel() == 0x31 )
				if( pMesh2->GetPartsNo()==6 )
					pMesh2->SetDispCheck( 8 );
				pMesh2 = (CMesh*)pMesh2->Next;
			}
		}
		if( pMesh1->GetPartsNo() == 6 && pMesh1->GetDispLevel() == 0x32 ) {
			pMesh2 = (CMesh*)m_Meshs.Top();
			while ( pMesh2 != NULL ) {
				if( pMesh2->GetPartsNo()==5 && pMesh2->GetDispLevel() == 0x21 )
					pMesh2->SetDispCheck( 7 );
				pMesh2 = (CMesh*)pMesh2->Next;
			}
		}
		pMesh1 = (CMesh*)pMesh1->Next;
	}
	LoadMotionFromFile( FileName);
	SetMotion3FromList( 0, GetMotionName() );

	return true;
}
