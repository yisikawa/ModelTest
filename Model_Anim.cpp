#include "Dx.h"
#include "Model.h"

// DEFINE
#define SAFE_RELEASE(p)		if ( (p) != NULL ) { (p)->Release(); (p) = NULL; }
#define SAFE_DELETES(p)		if ( (p) != NULL ) { delete [] (p); (p) = NULL; }
#define SAFE_DELETE(p)		if ( (p) != NULL ) { delete (p); (p) = NULL; }
#define PAI					(3.1415926535897932384626433832795f)

// Model_Common.cpp で定義されているユーティリティ関数の宣言
extern int Trim(char *s);

//------------------------------------------------------------------------------------------//																				//
//									CBone													//																		//
//------------------------------------------------------------------------------------------//

//======================================================================
//		コンストラクタ
//======================================================================
CBone::CBone()
{
	//m_mParent = -1;
	m_mTerm	  = false;
	m_pParent = NULL;
	m_pMotion = NULL;
	D3DXMatrixIdentity( &m_mTransform );
	D3DXMatrixIdentity( &m_mWorld );
	memset( m_Name, 0x00, sizeof(m_Name) );
}

//======================================================================
//		デストラクタ
//======================================================================
CBone::~CBone()
{
}

//======================================================================
//		コンストラクタ
//======================================================================
void CBone::InitData(void)
{
	m_pParent = NULL;
	m_pMotion = NULL;
	D3DXMatrixIdentity( &m_mTransform );
	D3DXMatrixIdentity( &m_mWorld );
	memset( m_Name, 0x00, sizeof(m_Name) );
}
//======================================================================
//
//		変換行列読み込み
//
//======================================================================
void CBone::LoadTransformMatrix( char *pFile )
{
	D3DXMATRIX m1,m2;

	vecT.x = *((float*)(pFile+16));
	vecT.y = *((float*)(pFile + 20));
	vecT.z = *((float*)(pFile + 24));
	memcpy(&vecQ, pFile, sizeof(D3DXQUATERNION));
	D3DXMatrixRotationQuaternion(&m1, &vecQ);
    D3DXMatrixTranslation(&m2,vecT.x,vecT.y,vecT.z);
    D3DXMatrixMultiply(&m1,&m1,&m2);
	m_mTransform = m1;
}

//======================================================================
//		名前設定
//======================================================================
void CBone::SetName( char *name )
{
	memset( m_Name, 0x00, sizeof(m_Name) );
	strcpy( m_Name, name );
}

//======================================================================
//		名前チェック
//======================================================================
bool CBone::CheckName( char *name )
{
	return (strcmp( m_Name, name ) == 0);
}

//======================================================================
//		アニメーション設定
//======================================================================
void CBone::SetMotion( CMotionElement *pMotion )
{
	m_pMotion = pMotion;
}

//======================================================================
//		名前取得
//======================================================================
char *CBone::GetName( void )
{
	return m_Name;
}

//======================================================================
//		行列取得
//======================================================================
D3DXMATRIX *CBone::GetMatrix( void )
{
	return &m_mWorld;
}

//======================================================================
//
//		フレーム読み込み
//
//	input
//		LPDIRECTXFILEDATA pDxFileData	: 
//		CBone *pParentBone			: 
//		CList *pBones					: 
//		CList *pMeshs					: 
//		unsigned long FVF				: 
//		IDirect3DDevice8 *pDev			: 
//
//	output
//		char *		: エラーメッセージへのポインタ（エラーがなければNULL
//
//======================================================================
char *CBone::LoadBone( char *pFile )
{
	// ボーン構造体
	typedef struct {
		unsigned char	parent_num,term;
		float			qx,qy,qz,qw;
		float			x,y,z;
	} BONE;
	// ローカル変数
	char ErrMsg[1024]						= "";
	char *err								= ErrMsg;
	char Name[256]							= "";

	//==============================================================
	// ペアレント
	//==============================================================
	m_mParent = (int)*(unsigned char*)(pFile+0);
	m_mTerm   = *(unsigned char*)(pFile+1)?true:false;

	//====================================================
	// フレーム名を取得
	//====================================================
	SetName( Name );

	//====================================================
	// 下の階層に行く
	//====================================================
	LoadTransformMatrix( pFile+2 );
	err = NULL;
	return err;
}

//------------------------------------------------------------------------------------------//																					//
//								CMotionFrame												//																				//
//------------------------------------------------------------------------------------------//
//======================================================================
//		コンストラクタ
//======================================================================
CMotionFrame::CMotionFrame()
{
	memset( m_Name, 0x00, sizeof(m_Name) );
}

//======================================================================
//		デストラクタ
//======================================================================
CMotionFrame::~CMotionFrame()
{
	CMotionElement *pMotionElement = (CMotionElement*)m_MotionEList.Top();
	while( pMotionElement ) {
		pMotionElement->~CMotionElement();
		pMotionElement = (CMotionElement*)pMotionElement->Next;
	}
	m_MotionEList.Release();

}

//======================================================================
//		初期化
//======================================================================
void CMotionFrame::InitData( void )
{
	memset( m_Name, 0x00, sizeof(m_Name) );
	CMotionElement *pMotionElement = (CMotionElement*)m_MotionEList.Top();
	while( pMotionElement ) {
		pMotionElement->~CMotionElement();
		pMotionElement = (CMotionElement*)pMotionElement->Next;
	}
	m_MotionEList.Release();
}

//======================================================================
//		アニメーション名称の設定
//======================================================================

int CMotionFrame::SetMotionName( char *MotionName )
{
	char tmp;
	strncpy( m_Name, MotionName, 4); m_Name[4] = '\0';
	if (isdigit(m_Name[0])) {
		if (isdigit(m_Name[1])) {
			tmp = m_Name[0];
			m_Name[0] = m_Name[2];
			m_Name[2] = tmp;
		}
		else {
			tmp = m_Name[0];
			m_Name[0] = m_Name[1];
			m_Name[1] = tmp;
		}
	}
	return 0;
}
//======================================================================
//		アニメーション名称の取得
//======================================================================

char *CMotionFrame::GetMotionName(void)
{
	return m_Name;
}

//======================================================================
//		アニメーションセット読みこみ
//======================================================================
void CMotionFrame::LoadMotionSet( char *pFile )
{
	//====================================================
	// フレーム名を取得
	//====================================================

	DAT2B *dat							= NULL;
	int   Element						= 0;
	int   Keys							= 0;
	float Times							= 0.f;
	float *fdat							= NULL;

	//====================================================
	// データ取得
	//====================================================
	SetMotionName( pFile );
	Element = (int)*((short*)(pFile+18));
	Keys    = (int)*((short*)(pFile+20));
    Times   = *((float*)(pFile+22));

	dat = (DAT2B *)(pFile+26);
	fdat = (float*)(pFile+26);
	//====================================================
	// 下の階層に行く
	//====================================================
	for( int i=0 ; i<Element ; i++ ) {
		CMotionElement *pMotionE = new CMotionElement;
		m_MotionEList.InsertEnd( pMotionE );
		pMotionE->SetBoneNo( dat[i].no );
		pMotionE->LoadMotion( i, Keys, Times, dat, fdat );
	}
}

//------------------------------------------------------------------------------------------//																				//
//								CMotionElement												//																				//
//------------------------------------------------------------------------------------------//

//======================================================================
//		コンストラクタ
//======================================================================
CMotionElement::CMotionElement()
{
	//m_BoneNo			= -1;
	m_RotationKeyNum	= 0;
	m_ScalingKeyNum		= 0;
	m_TranslateKeyNum	= 0;
	m_MatrixKeyNum		= 0;
	m_pRotationKeys		= NULL;
	m_pScalingKeys		= NULL;
	m_pTranslateKeys	= NULL;
	m_pMatrixKeys		= NULL;
}

//======================================================================
//		デストラクタ
//======================================================================
void CMotionElement::InitData( void )
{
	//m_BoneNo			= -1;
	m_RotationKeyNum	= 0;
	m_ScalingKeyNum		= 0;
	m_TranslateKeyNum	= 0;
	m_MatrixKeyNum		= 0;
	SAFE_DELETES( m_pRotationKeys );
	SAFE_DELETES( m_pScalingKeys );
	SAFE_DELETES( m_pTranslateKeys );
	SAFE_DELETES( m_pMatrixKeys );
}

//======================================================================
//		クラス初期化
//======================================================================
CMotionElement::~CMotionElement()
{
	SAFE_DELETES( m_pRotationKeys );
	SAFE_DELETES( m_pScalingKeys );
	SAFE_DELETES( m_pTranslateKeys );
	SAFE_DELETES( m_pMatrixKeys );
}
//======================================================================
//		ボーン番号のセット
//======================================================================

int CMotionElement::SetBoneNo(int BoneNo )
{
	m_BoneNo = BoneNo;
	return 0;
}

//======================================================================
//		ボーン番号の取得
//======================================================================

int CMotionElement::GetBoneNo(void)
{
	return m_BoneNo;
}
//======================================================================
//		アニメーション読み込み
//======================================================================
void CMotionElement::LoadMotion(int No, int Keys,
					float Times,DAT2B *bDat,float *fDat)	
{
	//-----------------------------------------------------
	// 回転
	//-----------------------------------------------------
	m_RotationKeyNum = Keys;
	m_pRotationKeys = new KEYROTATION [ Keys ]();
	for ( int i = 0; i < Keys ; i++ )
	{
		m_pRotationKeys[i].Time		= unsigned long(1.f/Times*100*i);
		m_pRotationKeys[i].Quat.x	= (bDat[No].idx_qtx&0x7fffffff)?fDat[bDat[No].idx_qtx+i]:bDat[No].qtx;
		m_pRotationKeys[i].Quat.y	= (bDat[No].idx_qty&0x7fffffff)?fDat[bDat[No].idx_qty+i]:bDat[No].qty;
		m_pRotationKeys[i].Quat.z	= (bDat[No].idx_qtz&0x7fffffff)?fDat[bDat[No].idx_qtz+i]:bDat[No].qtz;
		m_pRotationKeys[i].Quat.w	= (bDat[No].idx_qtw&0x7fffffff)?fDat[bDat[No].idx_qtw+i]:bDat[No].qtw;
	}
	//-----------------------------------------------------
	// スケール
	//-----------------------------------------------------
	m_ScalingKeyNum = Keys;
	m_pScalingKeys = new KEYSCALING [ Keys ]();
	for ( int i = 0; i < Keys ; i++ )
	{
		m_pScalingKeys[i].Time		= unsigned long(1.f/Times*100*i);
		m_pScalingKeys[i].Scale.x	= (bDat[No].idx_sx&0x7fffffff)?fDat[bDat[No].idx_sx+i]:bDat[No].sx;
		m_pScalingKeys[i].Scale.y	= (bDat[No].idx_sy&0x7fffffff)?fDat[bDat[No].idx_sy+i]:bDat[No].sy;
		m_pScalingKeys[i].Scale.z	= (bDat[No].idx_sz&0x7fffffff)?fDat[bDat[No].idx_sz+i]:bDat[No].sz;
	}
	//-----------------------------------------------------
	// 移動
	//-----------------------------------------------------
	m_TranslateKeyNum = Keys;
	m_pTranslateKeys = new KEYTRANSLATION [ Keys ]();
	for ( int i = 0; i < Keys ; i++ )
	{
		m_pTranslateKeys[i].Time	= unsigned long(1.f/Times*100*i);
		m_pTranslateKeys[i].Pos.x	= (bDat[No].idx_tx&0x7fffffff)?fDat[bDat[No].idx_tx+i]:bDat[No].tx;
		m_pTranslateKeys[i].Pos.y	= (bDat[No].idx_ty&0x7fffffff)?fDat[bDat[No].idx_ty+i]:bDat[No].ty;
		m_pTranslateKeys[i].Pos.z	= (bDat[No].idx_tz&0x7fffffff)?fDat[bDat[No].idx_tz+i]:bDat[No].tz;
	}
}

//======================================================================
//		アニメーション時間取得
//======================================================================
float CMotionElement::GetFinalTime( void )	
{
	float Time = 0.f;

	if( m_pRotationKeys ) Time = (float)m_pRotationKeys[m_RotationKeyNum-1].Time;
	return Time;
}

//======================================================================
//		アニメーション追加
//======================================================================
void CMotionElement::AddMotion(int No, int Keys,
					float Times,DAT2B *bDat,float *fDat)	
{
	unsigned int	i,j;
	unsigned long	RotationKeyNum;
	unsigned long	ScalingKeyNum;
	unsigned long	TranslateKeyNum;
	unsigned long	base;
	KEYROTATION*	pRotationKeys;
	KEYSCALING*		pScalingKeys;
	KEYTRANSLATION*	pTranslateKeys;

	//-----------------------------------------------------
	// 回転
	//-----------------------------------------------------
	RotationKeyNum	= m_RotationKeyNum;
	pRotationKeys	= new KEYROTATION [RotationKeyNum]();
	for( i=0 ; i<RotationKeyNum ; i++ ) {
		pRotationKeys[i] = m_pRotationKeys[i];
	}
	SAFE_DELETES( m_pRotationKeys );
	m_RotationKeyNum = RotationKeyNum + Keys;
	m_pRotationKeys = new KEYROTATION [ m_RotationKeyNum ]();
	for( i=0 ; i<RotationKeyNum ; i++ ) {
		m_pRotationKeys[i] = pRotationKeys[i];
	}
	SAFE_DELETES( pRotationKeys );

	base = m_pRotationKeys[RotationKeyNum-1].Time;
	for ( i = RotationKeyNum,j = 0 ; i < m_RotationKeyNum ; i++,j++ )
	{
		m_pRotationKeys[i].Time		= unsigned long(1.f/Times*100*j) + base;
		m_pRotationKeys[i].Quat.x	= (bDat[No].idx_qtx&0x7fffffff)?fDat[bDat[No].idx_qtx+j]:bDat[No].qtx;
		m_pRotationKeys[i].Quat.y	= (bDat[No].idx_qty&0x7fffffff)?fDat[bDat[No].idx_qty+j]:bDat[No].qty;
		m_pRotationKeys[i].Quat.z	= (bDat[No].idx_qtz&0x7fffffff)?fDat[bDat[No].idx_qtz+j]:bDat[No].qtz;
		m_pRotationKeys[i].Quat.w	= (bDat[No].idx_qtw&0x7fffffff)?fDat[bDat[No].idx_qtw+j]:bDat[No].qtw;
	}
	//-----------------------------------------------------
	// スケール
	//-----------------------------------------------------
	ScalingKeyNum	= m_ScalingKeyNum;
	pScalingKeys	= new KEYSCALING [ScalingKeyNum]();
	for( i=0 ; i<ScalingKeyNum ; i++ ) {
		pScalingKeys[i] = m_pScalingKeys[i];
	}
	SAFE_DELETES( m_pScalingKeys );
	m_ScalingKeyNum = ScalingKeyNum + Keys;
	m_pScalingKeys = new KEYSCALING [ m_ScalingKeyNum ]();
	for( i=0 ; i<ScalingKeyNum ; i++ ) {
		m_pScalingKeys[i] = pScalingKeys[i];
	}
	SAFE_DELETES( pScalingKeys );
	base = m_pScalingKeys[ScalingKeyNum-1].Time;
	for ( i = ScalingKeyNum,j = 0 ; i < m_ScalingKeyNum ; i++,j++ )
	{
		m_pScalingKeys[i].Time		= unsigned long(1.f/Times*100*j) + base;
		m_pScalingKeys[i].Scale.x	= (bDat[No].idx_sx&0x7fffffff)?fDat[bDat[No].idx_sx+j]:bDat[No].sx;
		m_pScalingKeys[i].Scale.y	= (bDat[No].idx_sy&0x7fffffff)?fDat[bDat[No].idx_sy+j]:bDat[No].sy;
		m_pScalingKeys[i].Scale.z	= (bDat[No].idx_sz&0x7fffffff)?fDat[bDat[No].idx_sz+j]:bDat[No].sz;
	}
	//-----------------------------------------------------
	// 移動
	//-----------------------------------------------------
	TranslateKeyNum	= m_TranslateKeyNum;
	pTranslateKeys	= new KEYTRANSLATION [TranslateKeyNum]();
	for( i=0 ; i<TranslateKeyNum ; i++ ) {
		pTranslateKeys[i] = m_pTranslateKeys[i];
	}
	SAFE_DELETES( m_pTranslateKeys );
	m_TranslateKeyNum = TranslateKeyNum + Keys;
	m_pTranslateKeys = new KEYTRANSLATION [ m_TranslateKeyNum ]();
	for( i=0 ; i<TranslateKeyNum ; i++ ) {
		m_pTranslateKeys[i] = pTranslateKeys[i];
	}
	SAFE_DELETES( pTranslateKeys );
	base = m_pTranslateKeys[TranslateKeyNum-1].Time;
	for ( i = TranslateKeyNum,j = 0 ; i < m_TranslateKeyNum ; i++,j++ )
	{
		m_pTranslateKeys[i].Time	= unsigned long(1.f/Times*100*j) + base;
		m_pTranslateKeys[i].Pos.x	= (bDat[No].idx_tx&0x7fffffff)?fDat[bDat[No].idx_tx+j]:bDat[No].tx;
		m_pTranslateKeys[i].Pos.y	= (bDat[No].idx_ty&0x7fffffff)?fDat[bDat[No].idx_ty+j]:bDat[No].ty;
		m_pTranslateKeys[i].Pos.z	= (bDat[No].idx_tz&0x7fffffff)?fDat[bDat[No].idx_tz+j]:bDat[No].tz;
	}
}

//======================================================================
//		コピー　MotionElement
//======================================================================
bool CMotionElement::CopyMotionElement( CMotionElement *pAnimElement )
{
	unsigned int i;

	if( m_pRotationKeys ) SAFE_DELETES( m_pRotationKeys );
	if( m_pScalingKeys ) SAFE_DELETES( m_pScalingKeys );
	if( m_pTranslateKeys ) SAFE_DELETES( m_pTranslateKeys );

	//-----------------------------------------------------
	// 回転
	//-----------------------------------------------------
	m_RotationKeyNum = pAnimElement->m_RotationKeyNum;
	m_pRotationKeys = new KEYROTATION [ m_RotationKeyNum ]();
	for ( i = 0; i < m_RotationKeyNum ; i++ ) {
		m_pRotationKeys[i]		= pAnimElement->m_pRotationKeys[i];
	}
	//-----------------------------------------------------
	// スケール
	//-----------------------------------------------------
	m_ScalingKeyNum = pAnimElement->m_ScalingKeyNum;
	m_pScalingKeys = new KEYSCALING [ m_ScalingKeyNum ]();
	for ( i = 0; i < m_ScalingKeyNum ; i++ ) {
		m_pScalingKeys[i]		= pAnimElement->m_pScalingKeys[i];
	}
	//-----------------------------------------------------
	// 移動
	//-----------------------------------------------------
	m_TranslateKeyNum = pAnimElement->m_TranslateKeyNum;
	m_pTranslateKeys = new KEYTRANSLATION [ m_TranslateKeyNum ]();
	for ( i = 0; i < m_TranslateKeyNum ; i++ ) {
		m_pTranslateKeys[i]	= pAnimElement->m_pTranslateKeys[i];
	}
	return true;
}

//======================================================================
//		追加MotionElement
//======================================================================
bool CMotionElement::AddMotionElement( CMotionElement *pAnimElement )	
{
	unsigned int	i,j;
	unsigned long	RotationKeyNum;
	unsigned long	ScalingKeyNum;
	unsigned long	TranslateKeyNum;
	unsigned long	base;
	KEYROTATION*	pRotationKeys;
	KEYSCALING*		pScalingKeys;
	KEYTRANSLATION*	pTranslateKeys;

	//-----------------------------------------------------
	// 回転
	//-----------------------------------------------------
	RotationKeyNum	= m_RotationKeyNum;
	pRotationKeys	= new KEYROTATION [RotationKeyNum]();
	for( i=0 ; i<RotationKeyNum ; i++ ) {
		pRotationKeys[i] = m_pRotationKeys[i];
	}
	SAFE_DELETES( m_pRotationKeys );
	m_RotationKeyNum = RotationKeyNum + pAnimElement->m_RotationKeyNum;
	m_pRotationKeys = new KEYROTATION [ m_RotationKeyNum ]();
	for( i=0 ; i<RotationKeyNum ; i++ ) {
		m_pRotationKeys[i] = pRotationKeys[i];
	}
	SAFE_DELETES( pRotationKeys );

	base = m_pRotationKeys[RotationKeyNum-1].Time;
	for ( i = RotationKeyNum,j = 0 ; i < m_RotationKeyNum ; i++,j++ ) {
		m_pRotationKeys[i].Time		= pAnimElement->m_pRotationKeys[j].Time + base;
		m_pRotationKeys[i].Quat		= pAnimElement->m_pRotationKeys[j].Quat;
	}
	//-----------------------------------------------------
	// スケール
	//-----------------------------------------------------
	ScalingKeyNum	= m_ScalingKeyNum;
	pScalingKeys	= new KEYSCALING [ScalingKeyNum]();
	for( i=0 ; i<ScalingKeyNum ; i++ ) {
		pScalingKeys[i] = m_pScalingKeys[i];
	}
	SAFE_DELETES( m_pScalingKeys );
	m_ScalingKeyNum = ScalingKeyNum + pAnimElement->m_ScalingKeyNum;
	m_pScalingKeys = new KEYSCALING [ m_ScalingKeyNum ]();
	for( i=0 ; i<ScalingKeyNum ; i++ ) {
		m_pScalingKeys[i] = pScalingKeys[i];
	}
	SAFE_DELETES( pScalingKeys );
	base = m_pScalingKeys[ScalingKeyNum-1].Time;
	for ( i = ScalingKeyNum,j = 0 ; i < m_ScalingKeyNum ; i++,j++ ) {
		m_pScalingKeys[i].Time		= pAnimElement->m_pScalingKeys[j].Time + base;
		m_pScalingKeys[i].Scale		= pAnimElement->m_pScalingKeys[j].Scale;
	}
	//-----------------------------------------------------
	// 移動
	//-----------------------------------------------------
	TranslateKeyNum	= m_TranslateKeyNum;
	pTranslateKeys	= new KEYTRANSLATION [TranslateKeyNum]();
	for( i=0 ; i<TranslateKeyNum ; i++ ) {
		pTranslateKeys[i] = m_pTranslateKeys[i];
	}
	SAFE_DELETES( m_pTranslateKeys );
	m_TranslateKeyNum = TranslateKeyNum + pAnimElement->m_TranslateKeyNum;
	m_pTranslateKeys = new KEYTRANSLATION [ m_TranslateKeyNum ]();
	for( i=0 ; i<TranslateKeyNum ; i++ ) {
		m_pTranslateKeys[i] = pTranslateKeys[i];
	}
	SAFE_DELETES( pTranslateKeys );
	base = m_pTranslateKeys[TranslateKeyNum-1].Time;
	for ( i = TranslateKeyNum,j = 0 ; i < m_TranslateKeyNum ; i++,j++ ) {
		m_pTranslateKeys[i].Time	= pAnimElement->m_pTranslateKeys[j].Time + base;
		m_pTranslateKeys[i].Pos		= pAnimElement->m_pTranslateKeys[j].Pos;
	}
	return true;
}

//======================================================================
//		拡大縮小キー取得
//======================================================================
D3DXMATRIX *CMotionElement::GetMotionScaling( float time )
{
	static D3DXMATRIX Matrix;

	unsigned long KeyNum = m_ScalingKeyNum;
	KEYSCALING *pKeys = m_pScalingKeys;

	float fTime = (float)fmod( time, (float)pKeys[KeyNum-1].Time );

	// 現在の時間がどのキー近辺にあるか？
	unsigned long i = 0;
	for ( i = 0; i < KeyNum; i++ ) if ( fTime <= (float)pKeys[i].Time ) break;

	// 前後のキーを設定
	unsigned long Key0 = (i == 0) ? (0) : (i - 1);
	unsigned long Key1 = (i == KeyNum) ? (0) : (i);

	// 前のフレーム時間と後ろのフレーム時間
	float fTime1 = (float)pKeys[Key0].Time;
	float fTime2 = (float)pKeys[Key1].Time;

	// 線形補完
	float LerpValue = 0;
	if ( fTime1 != fTime2 ) LerpValue = (fTime - fTime1)  / (fTime2 - fTime1);

	D3DXVECTOR3 vScale;
	D3DXVec3Lerp( &vScale, &pKeys[Key0].Scale, &pKeys[Key1].Scale, LerpValue );
	D3DXMatrixScaling( &Matrix, vScale.x, vScale.y, vScale.z );

	return &Matrix;
}

//======================================================================
//		移動キー取得
//======================================================================
D3DXMATRIX *CMotionElement::GetMotionTranslate( float time )
{
	static D3DXMATRIX Matrix;

	unsigned long KeyNum = m_TranslateKeyNum;
	KEYTRANSLATION *pKeys = m_pTranslateKeys;

	float fTime = (float)fmod( time, (float)pKeys[KeyNum-1].Time );

	// 現在の時間がどのキー近辺にあるか？
	unsigned long i = 0;
	for ( i = 0; i < KeyNum; i++ ) if ( fTime <= (float)pKeys[i].Time ) break;

	// 前後のキーを設定
	unsigned long Key0 = (i == 0) ? (0) : (i - 1);
	unsigned long Key1 = (i == KeyNum) ? (0) : (i);

	// 前のフレーム時間と後ろのフレーム時間
	float fTime1 = (float)pKeys[Key0].Time;
	float fTime2 = (float)pKeys[Key1].Time;

	// 線形補完
	float LerpValue = 0;
	if ( fTime1 != fTime2 ) LerpValue = (fTime - fTime1)  / (fTime2 - fTime1);

	D3DXVECTOR3 vPos;
	D3DXVec3Lerp( &vPos, &pKeys[Key0].Pos, &pKeys[Key1].Pos, LerpValue );
	D3DXMatrixTranslation( &Matrix, vPos.x, vPos.y, vPos.z );

	return &Matrix;
}

//======================================================================
//		回転キー取得
//======================================================================
D3DXMATRIX *CMotionElement::GetMotionRotation( float time )
{
	static D3DXMATRIX Matrix;

	unsigned long KeyNum = m_RotationKeyNum;
	KEYROTATION *pKeys = m_pRotationKeys;

	float fTime = (float)fmod( time, (float)pKeys[KeyNum-1].Time );

	// 現在の時間がどのキー近辺にあるか？
	unsigned long i = 0;
	for ( i = 0; i < KeyNum; i++ ) if ( fTime <= (float)pKeys[i].Time ) break;

	// 前後のキーを設定
	unsigned long Key0 = (i == 0) ? (0) : (i - 1);
	unsigned long Key1 = (i == KeyNum) ? (0) : (i);

	// 前のフレーム時間と後ろのフレーム時間
	float fTime1 = (float)pKeys[Key0].Time;
	float fTime2 = (float)pKeys[Key1].Time;

	// 線形補完
	float LerpValue = 0;
	if ( fTime1 != fTime2 ) LerpValue = (fTime - fTime1)  / (fTime2 - fTime1);

	// クオータニオン間で補完
	D3DXQUATERNION quat, q0, q1;

	q0.x = pKeys[Key0].Quat.x;
	q0.y = pKeys[Key0].Quat.y;
	q0.z = pKeys[Key0].Quat.z;
	q0.w = pKeys[Key0].Quat.w;

	q1.x = pKeys[Key1].Quat.x;
	q1.y = pKeys[Key1].Quat.y;
	q1.z = pKeys[Key1].Quat.z;
	q1.w = pKeys[Key1].Quat.w;

	D3DXQuaternionSlerp( &quat, &q0, &q1, LerpValue );
	D3DXMatrixRotationQuaternion( &Matrix, &quat );

	return &Matrix;
}
//= == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == =
//		Animation Matrix 生成
//======================================================================
D3DXMATRIX *CMotionElement::GetAnimationMatrix(int keyNo,D3DXMATRIX *iMatrix)
{
	static D3DXMATRIX oMatrix;
	D3DXVECTOR3 vCT(0.f, 0.f, 0.f);
	D3DXMatrixIdentity(&oMatrix);

	if ((m_RotationKeyNum + m_TranslateKeyNum + m_ScalingKeyNum) <= 0) return  &oMatrix;

	KEYROTATION		*pKeys1 = m_pRotationKeys;
	KEYTRANSLATION	*pKeys2 = m_pTranslateKeys;
	KEYSCALING		*pKeys3 = m_pScalingKeys;

	D3DXVec3TransformCoord(&vCT, &vCT, iMatrix);
//	D3DXMatrixTransformation(&oMatrix, NULL, NULL, NULL, NULL, &pKeys1[keyNo].Quat, NULL);
	D3DXMatrixTransformation(&oMatrix, &vCT, &pKeys1[keyNo].Quat, &pKeys3[keyNo].Scale, &vCT, &pKeys1[keyNo].Quat, &pKeys2[keyNo].Pos);

	return &oMatrix;
}

//======================================================================
//		Matrix 生成
//======================================================================
D3DXMATRIX *CMotionElement::GetMotionMatrix(float time, D3DXMATRIX *iMatrix )
{
	static D3DXMATRIX oMatrix;
	D3DXVECTOR3 vCT(0.f, 0.f, 0.f), vPos, vScale;

	D3DXMatrixIdentity(&oMatrix);
	if ((m_RotationKeyNum + m_TranslateKeyNum + m_ScalingKeyNum) <= 0) return  &oMatrix;

	unsigned long KeyNum = m_RotationKeyNum;
	KEYROTATION *pKeys1 = m_pRotationKeys;

	KEYTRANSLATION *pKeys2 = m_pTranslateKeys;

	KEYSCALING *pKeys3 = m_pScalingKeys;

	float fTime = (float)fmod(time, (float)pKeys1[KeyNum - 1].Time);

	// 現在の時間がどのキー近辺にあるか？
	unsigned long i = 0;
	for (i = 0; i < KeyNum; i++) if (fTime <= (float)pKeys1[i].Time) break;

	// 前後のキーを設定
	unsigned long Key0 = (i == 0) ? (0) : (i - 1);
	unsigned long Key1 = (i == KeyNum) ? (0) : (i);

	// 前のフレーム時間と後ろのフレーム時間
	float fTime1 = (float)pKeys1[Key0].Time;
	float fTime2 = (float)pKeys1[Key1].Time;

	// 線形補完
	float LerpValue = 0;
	if (fTime1 != fTime2) LerpValue = (fTime - fTime1) / (fTime2 - fTime1);

	// クオータニオン間で補完
	D3DXQUATERNION quat, q0, q1;

	q0.x = pKeys1[Key0].Quat.x;
	q0.y = pKeys1[Key0].Quat.y;
	q0.z = pKeys1[Key0].Quat.z;
	q0.w = pKeys1[Key0].Quat.w;

	q1.x = pKeys1[Key1].Quat.x;
	q1.y = pKeys1[Key1].Quat.y;
	q1.z = pKeys1[Key1].Quat.z;
	q1.w = pKeys1[Key1].Quat.w;

	D3DXQuaternionSlerp(&quat, &q0, &q1, LerpValue);
	D3DXVec3Lerp(&vPos, &pKeys2[Key0].Pos, &pKeys2[Key1].Pos, LerpValue);
	D3DXVec3Lerp(&vScale, &pKeys3[Key0].Scale, &pKeys3[Key1].Scale, LerpValue);
	D3DXVec3TransformCoord(&vCT, &vCT, iMatrix);
	D3DXMatrixTransformation(&oMatrix, &vCT, &quat, &vScale, &vCT, &quat, &vPos);

	return &oMatrix;
}
//======================================================================
//		Matrix 生成2
//======================================================================
D3DXMATRIX *CMotionElement::GetMotionMatrix2(float time)
{
	static D3DXMATRIX oMatrix;
	D3DXVECTOR3 vCT(0.f, 0.f, 0.f), vPos, vScale;

	D3DXMatrixIdentity(&oMatrix);
	if ((m_RotationKeyNum + m_TranslateKeyNum + m_ScalingKeyNum) <= 0) return  &oMatrix;

	unsigned long KeyNum = m_RotationKeyNum;
	KEYROTATION *pKeys1 = m_pRotationKeys;

	KEYTRANSLATION *pKeys2 = m_pTranslateKeys;

	KEYSCALING *pKeys3 = m_pScalingKeys;

	float fTime = (float)fmod(time, (float)pKeys1[KeyNum - 1].Time);

	// 現在の時間がどのキー近辺にあるか？
	unsigned long i = 0;
	for (i = 0; i < KeyNum; i++) if (fTime <= (float)pKeys1[i].Time) break;

	// 前後のキーを設定
	unsigned long Key0 = (i == 0) ? (0) : (i - 1);
	unsigned long Key1 = (i == KeyNum) ? (0) : (i);

	// 前のフレーム時間と後ろのフレーム時間
	float fTime1 = (float)pKeys1[Key0].Time;
	float fTime2 = (float)pKeys1[Key1].Time;

	// 線形補完
	float LerpValue = 0;
	if (fTime1 != fTime2) LerpValue = (fTime - fTime1) / (fTime2 - fTime1);

	// クオータニオン間で補完
	D3DXQUATERNION quat, q0, q1;

	q0.x = pKeys1[Key0].Quat.x;
	q0.y = pKeys1[Key0].Quat.y;
	q0.z = pKeys1[Key0].Quat.z;
	q0.w = pKeys1[Key0].Quat.w;

	q1.x = pKeys1[Key1].Quat.x;
	q1.y = pKeys1[Key1].Quat.y;
	q1.z = pKeys1[Key1].Quat.z;
	q1.w = pKeys1[Key1].Quat.w;

	D3DXQuaternionSlerp(&quat, &q0, &q1, LerpValue);
	D3DXVec3Lerp(&vPos, &pKeys2[Key0].Pos, &pKeys2[Key1].Pos, LerpValue);
	D3DXVec3Lerp(&vScale, &pKeys3[Key0].Scale, &pKeys3[Key1].Scale, LerpValue);
	D3DXMatrixTransformation(&oMatrix, &vCT, &quat, &vScale, &vCT, &quat, &vPos);

	return &oMatrix;
}
