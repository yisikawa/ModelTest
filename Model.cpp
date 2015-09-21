// INCLUDE
#define _CRT_SECURE_NO_WARNINGS
#include <vector>
#include <sstream>
#include <iostream>
#include <string>
#include <stdio.h>
#include <d3d9.h>
#include <d3dx9.h>
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

static const D3DXMATRIX matrixMirrorX(-1.0f,0,0,0,  0, 1.0f,0,0,  0,0, 1.0f,0,  0,0,0,1.0f);
static const D3DXMATRIX matrixMirrorY( 1.0f,0,0,0,  0,-1.0f,0,0,  0,0, 1.0f,0,  0,0,0,1.0f);
static const D3DXMATRIX matrixMirrorZ( 1.0f,0,0,0,  0, 1.0f,0,0,  0,0,-1.0f,0,  0,0,0,1.0f);
// グローバル
extern	CPC			*pPC;
extern	CNPC		*pNPC;
extern	bool		g_mPCFlag;
		int			g_mPCMotion = 0; // 0:default 1:attack 2:emotion 3:WS
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

// 二刀流コンバートテーブル
int g_mBLCnvTbl[9][10] = {
	0,1,2,2,0,0,0,3,0,0,		// HM
	0,1,2,3,0,0,3,0,0,0,		// HF
	0,0,1,0,0,0,0,0,0,3,		// EM
	0,0,0,0,0,1,0,0,0,0,		// EF
	0,0,2,1,2,0,0,0,0,0,		// TM
	0,0,2,1,2,0,0,0,0,0,		// TF
	0,1,1,2,0,0,0,0,0,0,		// M
	0,0,1,0,2,0,0,0,0,0,		// G
	0,0,0,0,0,0,0,0,0,0
};

//ヒュム♂、ヒュム♀、エル♂、エル♀、タル♂、タル♀、ミスラ、ガルカ
//各種族の基本モーションのファイルナンバー
int g_mBaseMotionFileNo[] = {27*0x80+82, 32*0x80+58, 37*0x80+31, 42*0x80+4, 46*0x80+93, 46*0x80+93, 51*0x80+89, 56*0x80+59};
//各種族の戦闘モーションの最初のファイルナンバー
int g_mMotionBFileNo[] = {32*0x80+13, 36*0x80+117, 41*0x80+84, 46*0x80+57, 51*0x80+19, 51*0x80+19, 56*0x80+14, 60*0x80+112};
//各種族の戦闘モーションのファイル数
int g_mmotionBnum[] = {9, 8, 10, 6, 6, 6, 9, 8};
//各種族の片手刀の武器INFO
int g_msinobiINFO[] = {7, 6 ,3, 5, 3, 3, 3, 4};
//各種族の両手刀の武器INFO
int g_mkatanaINFO[] = {-1, -1, -1, 7, -1, -1, -1, 8};
//各種族の片手刀の戦闘モーションのファイルナンバー　エル・ガルはこの次に両手刀モーションがある
int g_mMotionB2FileNo[] = {98*0x80+55, 98*0x80+86, 98*0x80+117, 99*0x80+20, 99*0x80+55, 99*0x80+55, 99*0x80+86, 99*0x80+117};
//各種族の片手刀INFO以降の武器でファイル番号を繰り下げる数
int g_mmotionB2num[] = {1, 1, 1, 2, 1, 1, 1, 2};
//各種族のエモーションの最初のファイルナンバー
int g_mMotionEFileNo[] = {32*0x80+40, 37*0x80+13, 41*0x80+114, 46*0x80+75, 51*0x80+37, 51*0x80+71, 56*0x80+41, 61*0x80+8};
//各種族のエモーションのファイルの数
int g_mmotionEnum = 6;
//各種族のチョコボ・釣りモーションの最初のファイルナンバー
int g_mMotionAFileNo[] = {90*0x80+14, 90*0x80+20, 90*0x80+26, 90*0x80+32, 90*0x80+38, 90*0x80+38, 90*0x80+44, 90*0x80+50};
//各種族のチョコボ・釣りモーションのファイルの数
int g_mmotionAnum = 2;
//各種族の二刀流左手武器攻撃モーションの最初のファイルナンバー
int g_mMotionBLFileNo[] = {108*0x80+100, 110*0x80+19, 111*0x80+66, 112*0x80+113, 114*0x80+20, 114*0x80+20, 115*0x80+61, 116*0x80+108};
//各種族の二刀流左手武器攻撃モーションのファイル数
int g_mmotionBLnum[] = {4, 4, 4, 2, 3, 3, 4, 3};
//各種族のウェポンバッシュ？モーションの最初のファイルナンバー
int g_mMotionBAFileNo[] = {95*0x80+5, 95*0x80+14, 95*0x80+23, 95*0x80+32, 95*0x80+41, 95*0x80+41, 95*0x80+50, 95*0x80+59};
//各種族のウェポンバッシュ？モーションのファイルの数
int g_mmotionBAnum = 3;
//各種族のつるはし・鎌・sitエモモーションの最初のファイルナンバー
int g_mMotionTuruFileNo[] = {123*0x80+13, 123*0x80+22, 123*0x80+31, 123*0x80+40, 123*0x80+49, 123*0x80+58, 123*0x80+67, 123*0x80+76};
//各種族のつるはし・鎌・sitエモモーションのファイル数
int g_mmotionTurunum = 3;


int Trim(char *s) {
	int i;
	int count = 0;

	/* 空ポインタか? */
	if (s == NULL) { /* yes */
		return -1;
	}

	/* 文字列長を取得する */
	i = strlen(s);

	/* 末尾から順に空白でない位置を探す */
	while (--i >= 0 && s[i] == ' ') count++;

	/* 終端ナル文字を付加する */
	s[i + 1] = '\0';

	/* 先頭から順に空白でない位置を探す */
	i = 0;
	while (s[i] != '\0' && s[i] == ' ') i++;
	strcpy(s, &s[i]);

	return i + count;
}

char * // 文字列へのポインタ
strrstr
(
const char *string, // 検索対象文字列
const char *pattern // 検索する文字列
)
{
	// 文字列終端に達するまで検索を繰り返す。
	const char *last = NULL;
	{for (const char *p = string; NULL != (p = strstr(p, pattern)); ++p)
	{
		last = p;
		if ('\0' == *p)
			return (char *)last;
	}}
	return (char *)last;
}//strrstr

//======================================================================
// 頂点フォーマット
//======================================================================

D3DVERTEXELEMENT9 VS0Formats[] =
{
	// First stream is first mesh
	{ 0, 0, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
	{ 0, 16, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDINDICES, 0 },
	{ 0, 20, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0 },
	{ 0, 32, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
	D3DDECL_END()
};

//------------------------------------------------------------------------------------------//
//									CListBase												//
//------------------------------------------------------------------------------------------//
//======================================================================
//		コンストラクタ
//======================================================================
CListBase::CListBase()
{
	ReferenceCount = 1;
	Prev = Next = NULL;
	pParentList = NULL;
}

//======================================================================
//		デストラクタ
//======================================================================
CListBase::~CListBase()
{
	if ( pParentList != NULL )
	{
		pParentList->Erase( this );
	}
}

//======================================================================
//		開放
//======================================================================
long CListBase::Release( void )
{
	long ref = ReferenceCount - 1;

	// 参照がなくなったら破棄
	if ( --ReferenceCount == 0 ) delete this;

	return ref;
}

//======================================================================
//		参照カウンタインクリメント
//======================================================================
void CListBase::AddRef( void )
{
	ReferenceCount++;
}

//------------------------------------------------------------------------------------------//																						//
//									CList													//																						//
//------------------------------------------------------------------------------------------//
//======================================================================
//		コンストラクタ
//======================================================================
CList::CList()
{
	Init();
}
//======================================================================
//		デストラクタ
//======================================================================
CList::~CList()
{
	Release();
}

//======================================================================
//		初期化
//======================================================================
void CList::Init( void )
{
	ListTop = NULL;
	ListEnd = NULL;
	Count = 0;
}

//======================================================================
//		先頭取得
//======================================================================
LPCListBase CList::Top( void )
{
	return ListTop;
}

//======================================================================
//		終端取得
//======================================================================
LPCListBase CList::End( void )
{
	return ListEnd;
}

//======================================================================
//		リスト解体
//======================================================================
void CList::Release( void )
{
	LPCListBase p = ListTop;
	while ( p != NULL )
	{
		// p の次を事前に取得（p が Release() 後解体されてる可能性高い）
		LPCListBase pp = p->Next;
		// 解体
		p->Release();
		// 次
		p = pp;
	}
	Init();
}

//======================================================================
//		リストの先頭に挿入
//======================================================================
void CList::InsertTop( LPCListBase p )
{
	//------------------------------------------------
	// 他のリストに登録されてるときはそちらから切断
	//------------------------------------------------
	if ( p->pParentList != NULL ) p->pParentList->Erase( p );
	p->pParentList = this;

	//------------------------------------------------
	// 接続
	//------------------------------------------------
	p->Prev = NULL;
	p->Next = ListTop;
	ListTop = p;
	if ( p->Next != NULL ) p->Next->Prev = p;
	if ( ListEnd == NULL ) ListEnd = p;
	Count++;
}

//======================================================================
//		リストの終端に挿入
//======================================================================
void CList::InsertEnd( LPCListBase p )
{
	//------------------------------------------------
	// 他のリストに登録されてるときはそちらから切断
	//------------------------------------------------
	if ( p->pParentList != NULL ) p->pParentList->Erase( p );
	p->pParentList = this;

	//------------------------------------------------
	// 接続
	//------------------------------------------------
	CListBase *pEnd = ListEnd;

	p->Prev = pEnd;
	p->Next = NULL;
	ListEnd = p;

	if ( pEnd == NULL )
	{
		ListTop = p;
	}
	else
	{
		pEnd->Next = p;
	}
	Count++;
}

//======================================================================
//		ターゲットの前にに挿入
//======================================================================
void CList::InsertPrev( LPCListBase pTarget, LPCListBase pIt )
{
	if( pIt == NULL ) return;
	if( pTarget==NULL ) {
		InsertTop( pIt );return;
	}
	//------------------------------------------------
	// 他のリストに登録されてるときはそちらから切断
	//------------------------------------------------
	if ( pIt->pParentList != NULL ) pIt->pParentList->Erase( pIt );
	pIt->pParentList = this;

	//------------------------------------------------
	// 接続
	//------------------------------------------------

	pIt->Prev = pTarget->Prev;
	pIt->Next = pTarget;
	pTarget->Prev = pIt;
	if( ListTop == pTarget ) {
		ListTop = pIt;
		pIt->Prev = NULL;
	}
	Count++;
}
//======================================================================
//		ターゲットの次に挿入
//======================================================================
void CList::InsertNext( LPCListBase pTarget,LPCListBase pIt )
{
	if(  pIt== NULL ) 
		return;
	if( pTarget==NULL ) {
		InsertTop( pIt );return;
	}
	//------------------------------------------------
	// 他のリストに登録されてるときはそちらから切断
	//------------------------------------------------
	if ( pIt->pParentList != NULL ) pIt->pParentList->Erase( pIt );
	pIt->pParentList = this;

	//------------------------------------------------
	// 接続
	//------------------------------------------------

	pIt->Prev = pTarget;
	pIt->Next = pTarget->Next;
	pTarget->Next = pIt;
	if( ListEnd == pTarget ) 
		ListEnd = pIt;
	Count++;
}
//======================================================================
//		リストから削除
//======================================================================
void CList::Erase( LPCListBase p )
{
	if ( p->pParentList != NULL ) p->pParentList = NULL;

	BYTE flag = 0x00;
	if ( p->Prev == NULL ) flag |= 0x01;		// 前に何もないとき
	if ( p->Next == NULL ) flag |= 0x02;		// 後に何もないとき

	//============================================
	//	該当するデータの削除
	//============================================
	switch ( flag )
	{
	///////////////////////////////////// 前後に何かあるとき
	case 0x00:
		p->Prev->Next = p->Next;
		p->Next->Prev = p->Prev;
		break;
	///////////////////////////////////// 前に何もないとき
	case 0x01:
		ListTop = p->Next;
		ListTop->Prev = NULL;
		break;
	///////////////////////////////////// 後に何もないとき
	case 0x02:
		ListEnd = ListEnd->Prev;
		p->Prev->Next = NULL;
		break;
	///////////////////////////////////// 前後に何もないとき
	case 0x03:
		ListTop = NULL;
		ListEnd = NULL;
		break;
	}
	Count--;
}

//======================================================================
//		特定のデータ取り出し
//======================================================================
LPCListBase CList::Data( long no )
{
	LPCListBase p = ListTop;
	while ( (p != NULL) && no-- )
	{
		p = p->Next;
	}
	return p;
}

//======================================================================
//		サイズ取得
//======================================================================
long CList::Size( void )
{
	return Count;
}

//------------------------------------------------------------------------------------------//																					//
//									CStream													//																				//
//------------------------------------------------------------------------------------------//


//======================================================================
//		コンストラクタ
//======================================================================
CStream::CStream()
{
	m_MinIdx		= 65535;
	m_MaxIdx		= -65535;
	m_pMaterial		= NULL;
	m_PrimitiveType	= D3DPT_TRIANGLELIST;
	m_IndexStart	= 0;
	m_FaceCount		= 0;
	m_DispLevel		= 0;
	m_texNo			= 0;
}

//======================================================================
//		デストラクタ
//======================================================================
CStream::~CStream()
{
}

//======================================================================
//		インデックスデータ設定
//======================================================================
void CStream::SetData( D3DPRIMITIVETYPE PrimitiveType, unsigned long index_start, unsigned long face_count )
{
	m_PrimitiveType	= PrimitiveType;
	m_IndexStart	= index_start;
	m_FaceCount		= face_count;
}

//======================================================================
//		プリミティブタイプ取得
//======================================================================
D3DPRIMITIVETYPE CStream::GetPrimitiveType( void )
{
	return m_PrimitiveType;
}

//======================================================================
//		インデックス開始位置取得
//======================================================================
unsigned long CStream::GetIndexStart( void )
{
	return m_IndexStart;
}

//======================================================================
//		インデックス使用数取得
//======================================================================
unsigned long CStream::GetFaceCount( void )
{
	return m_FaceCount;
}

//------------------------------------------------------------------------------------------//																					//
//									CMaterial												//																					//
//------------------------------------------------------------------------------------------//


//======================================================================
//
//		コンストラクタ
//
//======================================================================
CMaterial::CMaterial()
{
	m_pTexture		= NULL;
	m_IndexStart	= 0;
	m_FaceCount		= 0;
	memset( &m_Material, 0x00, sizeof(m_Material) );
	m_Material.Diffuse.a = 1.0f;
	m_Material.Diffuse.r = 1.0f;
	m_Material.Diffuse.g = 1.0f;
	m_Material.Diffuse.b = 1.0f;
	m_Material.Ambient.a = 1.0f;
	m_Material.Ambient.r = 1.0f;
	m_Material.Ambient.g = 1.0f;
	m_Material.Ambient.b = 1.0f;
}

//======================================================================
//		デストラクタ
//======================================================================
CMaterial::~CMaterial()
{
	SAFE_RELEASE( m_pTexture );
}

//======================================================================
//		インデックスデータ設定
//======================================================================
void CMaterial::SetData( unsigned long index_start, unsigned long face_count )
{
	m_IndexStart	= index_start;
	m_FaceCount		= face_count;
}

//======================================================================
//		マテリアルデータ設定
//======================================================================
void CMaterial::SetMaterial( D3DMATERIAL9 *pMat )
{
	m_Material = *pMat;
}

//======================================================================
//		テクスチャ設定
//======================================================================
void CMaterial::SetTexture( IDirect3DTexture9 *pTex )
{
	SAFE_RELEASE( m_pTexture );
	m_pTexture = pTex;
	if ( m_pTexture != NULL ) m_pTexture->AddRef();
}

//======================================================================
//		インデックス開始位置取得
//======================================================================
unsigned long CMaterial::GetIndexStart( void )
{
	return m_IndexStart;
}

//======================================================================
//		インデックス使用数取得
//======================================================================
unsigned long CMaterial::GetFaceCount( void )
{
	return m_FaceCount;
}

//======================================================================
//		マテリアルデータ取得
//======================================================================
D3DMATERIAL9 *CMaterial::GetMaterial( void )
{
	return &m_Material;
}

//======================================================================
//		テクスチャ取得
//======================================================================
IDirect3DTexture9 *CMaterial::GetTexture( void )
{
	return m_pTexture;
}

//------------------------------------------------------------------------------------------//																				//
//									CMesh													//																				//
//------------------------------------------------------------------------------------------//
//======================================================================
//		コンストラクタ
//======================================================================
CMesh::CMesh()
{
//	m_pParent				= NULL;
	m_pBoneTransforms		= NULL;
	m_pBoneTbl				= NULL;
	m_pBoneTblNum			= NULL;
	m_mBoneNum				= 0;
	m_PartsNo				= 0;
	m_DispLevel				= 0;
	m_FlipFlag				= false;
	m_lpIB					= NULL;
	m_lpVB1					= NULL;
	m_lpVB2					= NULL;
	m_NumIndex = m_NumVertices = m_NumFaces = m_VBSize = m_IBSize = m_FVF = 0;
	m_Stnum					= 0;
	m_Streams.Init();
	m_mBoneNum;
}

//======================================================================
//		デストラクタ
//======================================================================
CMesh::~CMesh()
{
	SAFE_RELEASE( m_lpIB );
	SAFE_RELEASE( m_lpVB1 );
	SAFE_RELEASE( m_lpVB2 );
	SAFE_DELETES( m_pBoneTbl );
	SAFE_DELETES( m_pBoneTblNum );
	m_Streams.Release();

}

void CMesh::Conv1VertexC1(CUSTOMVERTEX *pV,int No1,int No2, WORD Type,
				  WORD *pBone, CLOTHVERTEX1 *pVertexC,float u,float v,int Flip)
{
	int			bidx,MatrixFlag;
	D3DXVECTOR3	p1;
	D3DXMATRIX	mm;

	bidx = pBone[No1*2]&0x7f;
	MatrixFlag = 0;
	if( Flip ) {
		bidx = pBone[No1*2];
		MatrixFlag = (bidx>>14)&0x3;
		bidx >>= 7; bidx &= 0x7f;
	}
	if( Flip ) {
		p1.x = pVertexC[No2].x; p1.y = pVertexC[No2].y; p1.z = pVertexC[No2].z;
		switch( MatrixFlag ) {
			case 1:
				mm = matrixMirrorX;
				break;
			case 2:
				mm = matrixMirrorY;
				break;
			case 3:
				mm = matrixMirrorZ;
				break;
		}
		D3DXVec3TransformCoord(&p1,&p1,&mm);
		pV->p.x = p1.x; pV->p.y = p1.y; pV->p.z = p1.z;
	} else {
		pV->p.x = pVertexC[No2].x; pV->p.y = pVertexC[No2].y; pV->p.z = pVertexC[No2].z;
	}
	pV->b1 = 1.0;
	if( Type&0x80 ) {
		pV->indx	= bidx;
	} else {
		for( int i=0 ; i<m_mBoneNum ; i++ ) {
			if( m_pBoneTbl[i] == bidx ) {
				pV->indx = i;
				break;
			}
		}
	}
	pV->u	= u; pV->v = v;	
}

void CMesh::Conv1VertexC2(CUSTOMVERTEX *pV,int No1, int No2, WORD Type,
				  WORD *pBone, CLOTHVERTEX2 *pVertexC2,float u,float v,int Flip)
{
	int			bidx,MatrixFlag;
	D3DXVECTOR3	p1;
	D3DXMATRIX	mm;

	bidx = pBone[No1*2]&0x7f;
	MatrixFlag = 0;
	if( Flip ) {
		bidx = pBone[No1*2];
		MatrixFlag = (bidx>>14)&0x3;
		bidx >>= 7; bidx &= 0x7f;
	}
	if( Flip ) {
		p1.x = pVertexC2[No2].x1; p1.y = pVertexC2[No2].y1; p1.z = pVertexC2[No2].z1;
		switch( MatrixFlag ) {
			case 1:
				mm = matrixMirrorX;
				break;
			case 2:
				mm = matrixMirrorY;
				break;
			case 3:
				mm = matrixMirrorZ;
				break;
		}
		D3DXVec3TransformCoord(&p1,&p1,&mm);
		pV->p.x = p1.x; pV->p.y = p1.y; pV->p.z = p1.z;
	} else {
		pV->p.x = pVertexC2[No2].x1; pV->p.y = pVertexC2[No2].y1; pV->p.z = pVertexC2[No2].z1;
	}
	pV->b1		= pVertexC2[No2].w1;
	if( Type&0x80 ) {
		pV->indx	= bidx;
	} else {
		for( int i=0 ; i<m_mBoneNum ; i++ ) {
			if( m_pBoneTbl[i] == bidx ) {
				pV->indx = i;
				break;
			}
		}
	}
	pV->u	= u; pV->v = v;		
}

void CMesh::Conv1Vertex1(CUSTOMVERTEX *pV,int No1,int No2, WORD Type,
				  WORD *pBone, MODELVERTEX1 *pVertex, float u, float v, int Flip)
{
	int			bidx,MatrixFlag;
	D3DXVECTOR3	p1,h1;
	D3DXMATRIX	mm;

	bidx = pBone[No1*2]&0x7f;
	MatrixFlag = 0;
	if( Flip ) {
		bidx = pBone[No1*2];
		MatrixFlag = (bidx>>14)&0x3;
		bidx >>= 7; bidx &= 0x7f;				
	}
	if( Flip ) {
		p1.x = pVertex[No2].x; p1.y = pVertex[No2].y; p1.z = pVertex[No2].z;
		h1.x = pVertex[No2].hx; h1.y = pVertex[No2].hy; h1.z = pVertex[No2].hz;
		switch( MatrixFlag ) {
			case 1:
				mm = matrixMirrorX;
				break;
			case 2:
				mm = matrixMirrorY;
				break;
			case 3:
				mm = matrixMirrorZ;
				break;
		}
		D3DXVec3TransformCoord(&p1,&p1,&mm);
		D3DXVec3TransformCoord(&h1,&h1,&mm);
		pV->p.x = p1.x; pV->p.y = p1.y; pV->p.z = p1.z;
		pV->n.x = h1.x; pV->n.y = h1.y; pV->n.z = h1.z;
	} else {
		pV->p.x = pVertex[No2].x; pV->p.y = pVertex[No2].y; pV->p.z = pVertex[No2].z;
		pV->n.x = pVertex[No2].hx; pV->n.y = pVertex[No2].hy; pV->n.z = pVertex[No2].hz;
	}			
	pV->b1		= 1.0;
	if( Type&0x80 ) {
		pV->indx	= bidx;
	} else {
		for( int i=0 ; i<m_mBoneNum ; i++ ) {
			if( m_pBoneTbl[i] == bidx ) {
				pV->indx = i;
				break;
			}
		}
	}
	pV->u	= u; pV->v = v;
}

void CMesh::Conv1Vertex2(CUSTOMVERTEX *pV,int No1,int No2, WORD Type,
				  WORD *pBone, MODELVERTEX2 *pVertex2, float u, float v, int Flip)
{
	int			bidx,MatrixFlag;
	D3DXVECTOR3	p1,h1;
	D3DXMATRIX	mm;

	bidx = pBone[No1*2]&0x7f;
	MatrixFlag = 0;
	if( Flip ) {
		bidx = pBone[No1*2];
		MatrixFlag = (bidx>>14)&0x3;
		bidx >>= 7; bidx &= 0x7f;
	}
	if( Flip ) {
		p1.x = pVertex2[No2].x1; p1.y = pVertex2[No2].y1; p1.z = pVertex2[No2].z1;
		h1.x = pVertex2[No2].hx1; h1.y = pVertex2[No2].hy1; h1.z = pVertex2[No2].hz1;
		switch( MatrixFlag ) {
			case 1:
				mm = matrixMirrorX;
				break;
			case 2:
				mm = matrixMirrorY;
				break;
			case 3:
				mm = matrixMirrorZ;
				break;
		}
		D3DXVec3TransformCoord(&p1,&p1,&mm);
		D3DXVec3TransformCoord(&h1,&h1,&mm);
		pV->p.x = p1.x; pV->p.y = p1.y; pV->p.z = p1.z;
		pV->n.x = h1.x; pV->n.y = h1.y; pV->n.z = h1.z;
	} else {
		pV->p.x = pVertex2[No2].x1; pV->p.y = pVertex2[No2].y1; pV->p.z = pVertex2[No2].z1;
		pV->n.x = pVertex2[No2].hx1; pV->n.y = pVertex2[No2].hy1; pV->n.z = pVertex2[No2].hz1;
	}
	pV->b1		= pVertex2[No2].w1;
	if( Type&0x80 ) {
		pV->indx	= bidx;
	} else {
		for( int i=0 ; i<m_mBoneNum ; i++ ) {
			if( m_pBoneTbl[i] == bidx ) {
				pV->indx = i;
				break;
			}
		}
	}
	pV->u	= u; pV->v = v;
}

void CMesh::Conv2VertexC1(CUSTOMVERTEX *pV,int No1,int No2, WORD Type,
				  WORD *pBone, CLOTHVERTEX1 *pVertexC,float u,float v,int Flip)
{
	pV->p.x = 0.0; pV->p.y = 0.0; pV->p.z = 0.0;
	pV->b1 = 0.0;
	pV->indx	= 0;
	pV->u	= 0; pV->v = 0;
}

void CMesh::Conv2VertexC2(CUSTOMVERTEX *pV,int No1, int No2, WORD Type,
				  WORD *pBone, CLOTHVERTEX2 *pVertexC2,float u,float v,int Flip)
{
	int			bidx,MatrixFlag;
	D3DXVECTOR3	p1;
	D3DXMATRIX	mm;

	bidx = pBone[No1*2+1]&0x7f;
	MatrixFlag = 0;
	if( Flip ) {
		bidx = pBone[No1*2+1];
		MatrixFlag = (bidx>>14)&0x3;
		bidx >>= 7; bidx &= 0x7f;
	}
	if( Flip ) {
		p1.x = pVertexC2[No2].x2; p1.y = pVertexC2[No2].y2; p1.z = pVertexC2[No2].z2;
		switch( MatrixFlag ) {
			case 1:
				mm = matrixMirrorX;
				break;
			case 2:
				mm = matrixMirrorY;
				break;
			case 3:
				mm = matrixMirrorZ;
				break;
		}
		D3DXVec3TransformCoord(&p1,&p1,&mm);
		pV->p.x = p1.x; pV->p.y = p1.y; pV->p.z = p1.z;
	} else {
		pV->p.x = pVertexC2[No2].x2; pV->p.y = pVertexC2[No2].y2; pV->p.z = pVertexC2[No2].z2;
	}
	pV->b1		= pVertexC2[No2].w2;
	if( Type&0x80 ) {
		pV->indx	= bidx;
	} else {
		for( int i=0 ; i<m_mBoneNum ; i++ ) {
			if( m_pBoneTbl[i] == bidx ) {
				pV->indx = i;
				break;
			}
		}
	}
	pV->u	= u; pV->v = v;
}			

void CMesh::Conv2Vertex1(CUSTOMVERTEX *pV,int No1,int No2, WORD Type,
				  WORD *pBone, MODELVERTEX1 *pVertex, float u, float v, int Flip)
{
	pV->p.x = 0.0; pV->p.y = 0.0; pV->p.z = 0.0;
	pV->b1		= 0.0;
	pV->indx	= 0;
	pV->n.x = 0.0; pV->n.y = 0.0; pV->n.z = 0.0;
	pV->u	= 0.; pV->v = 0.;
}

void CMesh::Conv2Vertex2(CUSTOMVERTEX *pV,int No1,int No2, WORD Type,
				  WORD *pBone, MODELVERTEX2 *pVertex2, float u, float v, int Flip)
{
	int			bidx,MatrixFlag;
	D3DXVECTOR3	p1,h1;
	D3DXMATRIX	mm;
	bidx = pBone[No1*2+1]&0x7f;
	MatrixFlag = 0;
	if( Flip ) {
		bidx = pBone[No1*2+1];
		MatrixFlag = (bidx>>14)&0x3;
		bidx >>= 7; bidx &= 0x7f;
	}
	if( Flip ) {
		p1.x = pVertex2[No2].x2; p1.y = pVertex2[No2].y2; p1.z = pVertex2[No2].z2;
		h1.x = pVertex2[No2].hx2; h1.y = pVertex2[No2].hy2; h1.z = pVertex2[No2].hz2;
		switch( MatrixFlag ) {
			case 1:
				mm = matrixMirrorX;
				break;
			case 2:
				mm = matrixMirrorY;
				break;
			case 3:
				mm = matrixMirrorZ;
				break;
		}
		D3DXVec3TransformCoord(&p1,&p1,&mm);
		D3DXVec3TransformCoord(&h1,&h1,&mm);
		pV->p.x = p1.x; pV->p.y = p1.y; pV->p.z = p1.z;
		pV->n.x = h1.x; pV->n.y = h1.y; pV->n.z = h1.z;
	} else {
		pV->p.x = pVertex2[No2].x2; pV->p.y = pVertex2[No2].y2; pV->p.z = pVertex2[No2].z2;
		pV->n.x = pVertex2[No2].hx2; pV->n.y = pVertex2[No2].hy2; pV->n.z = pVertex2[No2].hz2;
	}
	pV->b1		= pVertex2[No2].w2;
	if( Type&0x80 ) {
		pV->indx	= bidx;
	} else {
		for( int i=0 ; i<m_mBoneNum ; i++ ) {
			if( m_pBoneTbl[i] == bidx ) {
				pV->indx = i;
				break;
			}
		}
	}
	pV->u	= u; pV->v = v;
}

//=======================================================
//  ボーンテーブルの作成
//========================================================
void CMesh::SetMeshBone( int Num1, int Num2, WORD Type, WORD BoneTblNum, WORD *pBone, WORD *pBoneTbl, int Flip )
{
	int		bidx,ii,jj;

	m_pBoneTbl = new WORD[BoneTblNum];
	m_pBoneTblNum = new DWORD[BoneTblNum];
	memset(m_pBoneTbl,0,BoneTblNum*sizeof(WORD));
	memset(m_pBoneTblNum,0,BoneTblNum*sizeof(DWORD));
	m_mBoneNum =0;
	if( Type&0x80 ) {
		m_mBoneNum = BoneTblNum;
		memcpy(m_pBoneTbl,pBoneTbl,BoneTblNum*sizeof(WORD));
	} else {
		for( ii=0 ; ii<Num1 ; ii++ ) {
			bidx = pBone[ii*2]&0x7f;
			if( Flip ) {
				bidx = pBone[ii*2];
				bidx >>= 7; bidx &= 0x7f;
			}
			for( jj=0 ; jj<m_mBoneNum ; jj++ ) {
				if( bidx == m_pBoneTbl[jj] ) {
					m_pBoneTblNum[jj]++;
					break;
				}
			}
			if( jj>=m_mBoneNum ) {
				m_pBoneTbl[m_mBoneNum] = (WORD)bidx;
				m_pBoneTblNum[m_mBoneNum]++;
				m_mBoneNum++;
			}
		}
		for( ii=0 ; ii<Num2 ; ii++ ) {
			bidx = pBone[(ii+Num1)*2]&0x7f;
			if( Flip ) {
				bidx = pBone[(ii+Num1)*2];
				bidx >>= 7; bidx &= 0x7f;
			}
			for( jj=0 ; jj<m_mBoneNum ; jj++ ) {
				if( bidx == m_pBoneTbl[jj] ) {
					m_pBoneTblNum[jj]++;
					break;
				}
			}
			if( jj>=m_mBoneNum ) {
				m_pBoneTbl[m_mBoneNum] = (WORD)bidx;
				m_pBoneTblNum[m_mBoneNum]++;
				m_mBoneNum++;
			}
			bidx = pBone[(ii+Num1)*2+1]&0x7f;
			if( Flip ) {
				bidx = pBone[(ii+Num1)*2+1];
				bidx >>= 7; bidx &= 0x7f;
			}
			for( jj=0 ; jj<m_mBoneNum ; jj++ ) {
				if( bidx == m_pBoneTbl[jj] ) {
					m_pBoneTblNum[jj]++;
					break;
				}
			}
			if( jj>=m_mBoneNum ) {
				m_pBoneTbl[m_mBoneNum] = (WORD)bidx;
				m_pBoneTblNum[m_mBoneNum]++;
				m_mBoneNum++;
			}
		}
	}
}

//======================================================================
//		メッシュ読み込み
//======================================================================

HRESULT CMesh::LoadMesh( char *pFile, CModel *pModel, unsigned long FVF,int Flip  )
{
	HRESULT hr						= D3D_OK; 

	//==============================================================
	// メッシュの生成
	//==============================================================
	int				Renumber,*pUVidx;
	float			*pUTemp,*pVTemp;
	char			*pPoly;
	short			Num1,Num2;
	WORD			*pBone,*pBoneTbl;
	int				addpos,NumIndex,NumFaces,NumVertices,FaceNum,FaceType;
	MODELVERTEX1	*pVertex;
	MODELVERTEX2	*pVertex2;
	CLOTHVERTEX1	*pVertexC;
	CLOTHVERTEX2	*pVertexC2;
	TEXLIST			*pTexList;
	TEXLIST2		*pTexList2;
	D3DXMATRIX		mm;
	D3DXVECTOR3		p1,h1;

	DAT2AHeader *pcp=(DAT2AHeader *)pFile; 
	Num1		=	*((short*)(pFile+16+pcp->OffsetWeight*2));
	Num2		=	*((short*)(pFile+16+pcp->OffsetWeight*2+2));
	pBone		=	(WORD*)(pFile+16+pcp->OffsetBone*2);
	pBoneTbl	=   (WORD*)(pFile+16+pcp->OffsetBoneTbl*2);
	pVertex		=	(MODELVERTEX1*)(pFile+16+pcp->OffsetVertex*2);
	pVertex2	=	(MODELVERTEX2*)(((char*)pVertex)+ Num1*0x18    );
	pVertexC	=	(CLOTHVERTEX1*)(pFile+16+pcp->OffsetVertex*2);
	pVertexC2	=	(CLOTHVERTEX2*)(((char*)pVertex)+ Num1*0xC    );
	pPoly		=	(char*)(pFile+16+ pcp->OffsetPoly*2);
	NumVertices	=	Num1 + Num2;
	NumFaces	=	0;
	NumIndex	=	0;
	addpos		=	0;
	pUVidx		= new int[NumVertices*10];
	pUTemp		= new float[NumVertices*10];
	pVTemp		= new float[NumVertices*10];
	for( int i=0 ; i<NumVertices ; i++ ) pUVidx[i] = -1;
	Renumber = NumVertices;
#pragma pack(push,2)
	typedef struct {
		short	tag;
		int		i1;
		float	f1,f2;
		short	s1,s2;
		float	f3;
		int		i2,i3;
		short	s3;
		float	f4;
		short	s4;
		float	f5;
		int		i4;
	} H8010 ;
#pragma pack(pop)

	H8010 *pH8010;
	SetDispLevel( (pcp->Type>>8) );
    while(addpos<pcp->PolyNum*2 ){
		FaceType	= (int)*(WORD*)(pPoly+addpos  );
		FaceNum		= (int)*(WORD*)(pPoly+addpos+2);
        if( FaceType==0xffff ) break;
        if( 0x8010 == (FaceType&0x80F0) ){
          addpos+=0x2e; continue;
		} else if( 0x8000 == (FaceType&0x80F0) ){
          addpos+=0x12; continue; 
		} else if( 0x5453 == FaceType ){
			pTexList = (TEXLIST *)(pPoly+addpos+4);
			NumIndex +=3;
			if( pUVidx[pTexList[0].i1] == -1 ) {
				pUVidx[pTexList[0].i1] = pTexList[0].i1;
				pUTemp[pTexList[0].i1] = pTexList[0].u1;
				pVTemp[pTexList[0].i1] = pTexList[0].v1;
			} else if( pUTemp[pTexList[0].i1] != pTexList[0].u1 ||
					   pVTemp[pTexList[0].i1] != pTexList[0].v1 ) {
				pUVidx[Renumber]=pTexList[0].i1;
				pTexList[0].i1 = (short)Renumber;
				pUTemp[Renumber] = pTexList[0].u1;
				pVTemp[Renumber] = pTexList[0].v1;
				Renumber++;
			}
			if( pUVidx[pTexList[0].i2] == -1 ) {
				pUVidx[pTexList[0].i2] = pTexList[0].i2;
				pUTemp[pTexList[0].i2] = pTexList[0].u2;
				pVTemp[pTexList[0].i2] = pTexList[0].v2;
			} else if( pUTemp[pTexList[0].i2] != pTexList[0].u2 ||
					   pVTemp[pTexList[0].i2] != pTexList[0].v2 ) {
				pUVidx[Renumber]=pTexList[0].i2;
				pTexList[0].i2 = (short)Renumber;
				pUTemp[Renumber] = pTexList[0].u2;
				pVTemp[Renumber] = pTexList[0].v2;
				Renumber++;
			}
			if( pUVidx[pTexList[0].i3] == -1 ) {
				pUVidx[pTexList[0].i3] = pTexList[0].i3;
				pUTemp[pTexList[0].i3] = pTexList[0].u3;
				pVTemp[pTexList[0].i3] = pTexList[0].v3;
			} else if( pUTemp[pTexList[0].i3] != pTexList[0].u3 ||
					   pVTemp[pTexList[0].i3] != pTexList[0].v3 ) {
				pUVidx[Renumber]=pTexList[0].i3;
				pTexList[0].i3 = (short)Renumber;
				pUTemp[Renumber] = pTexList[0].u3;
				pVTemp[Renumber] = pTexList[0].v3;
				Renumber++;
			}
			pTexList2 = (TEXLIST2 *)(pPoly+addpos+4+0x1e);
			for( int i=0 ; i<FaceNum-1 ; i++ ) {
				NumIndex++;
				if( pUVidx[pTexList2[i].i] == -1 ) {
					pUVidx[pTexList2[i].i] = pTexList2[i].i;
					pUTemp[pTexList2[i].i] = pTexList2[i].u;
					pVTemp[pTexList2[i].i] = pTexList2[i].v;
				} else if( pUTemp[pTexList2[i].i] != pTexList2[i].u ||
						   pVTemp[pTexList2[i].i] != pTexList2[i].v ) {
					pUVidx[Renumber]=pTexList2[i].i;
					pTexList2[i].i = (short)Renumber;
					pUTemp[Renumber] = pTexList2[i].u;
					pVTemp[Renumber] = pTexList2[i].v;
					Renumber++;
				}
			}
			addpos+=FaceNum*10  + 0x18; NumFaces += FaceNum; continue; 
		} else if( 0x4353 == FaceType ){
			addpos+=FaceNum*20  + 0x0C; NumFaces += FaceNum; continue; 
		} else if( 0x0043 == FaceType ){
			addpos+=FaceNum*10  + 0x4;  NumFaces += FaceNum; continue; 
		} else if( 0x0054 == FaceType ){
			pTexList = (TEXLIST *)(pPoly+addpos+4);
			for( int i=0 ; i<FaceNum ; i++ ) {
				NumIndex += 3;
				if( pUVidx[pTexList[i].i1] == -1 ) {
					pUVidx[pTexList[i].i1] = pTexList[i].i1;
					pUTemp[pTexList[i].i1] = pTexList[i].u1;
					pVTemp[pTexList[i].i1] = pTexList[i].v1;
				} else if( pUTemp[pTexList[i].i1] != pTexList[i].u1 ||
						   pVTemp[pTexList[i].i1] != pTexList[i].v1 ) {
					pUVidx[Renumber]=pTexList[i].i1;
					pTexList[i].i1 = (short)Renumber;
					pUTemp[Renumber] = pTexList[i].u1;
					pVTemp[Renumber] = pTexList[i].v1;
					Renumber++;
				}
				if( pUVidx[pTexList[i].i2] == -1 ) {
					pUVidx[pTexList[i].i2] = pTexList[i].i2;
					pUTemp[pTexList[i].i2] = pTexList[i].u2;
					pVTemp[pTexList[i].i2] = pTexList[i].v2;
				} else if( pUTemp[pTexList[i].i2] != pTexList[i].u2 ||
						   pVTemp[pTexList[i].i2] != pTexList[i].v2 ) {
					pUVidx[Renumber]=pTexList[i].i2;
					pTexList[i].i2 = (short)Renumber;
					pUTemp[Renumber] = pTexList[i].u2;
					pVTemp[Renumber] = pTexList[i].v2;
					Renumber++;
				}
				if( pUVidx[pTexList[i].i3] == -1 ) {
					pUVidx[pTexList[i].i3] = pTexList[i].i3;
					pUTemp[pTexList[i].i3] = pTexList[i].u3;
					pVTemp[pTexList[i].i3] = pTexList[i].v3;
				} else if( pUTemp[pTexList[i].i3] != pTexList[i].u3 ||
						   pVTemp[pTexList[i].i3] != pTexList[i].v3 ) {
					pUVidx[Renumber]=pTexList[i].i3;
					pTexList[i].i3 = (short)Renumber;
					pUTemp[Renumber] = pTexList[i].u3;
					pVTemp[Renumber] = pTexList[i].v3;
					Renumber++;
				}
			} 
			addpos+=FaceNum*30  + 0x4;  NumFaces += FaceNum;
			continue; 
		}
        break;
	}
	NumVertices = Renumber;
	m_NumVertices = NumVertices;
	m_NumFaces = NumFaces;
	m_NumIndex = NumIndex;
	m_FVF = FVF_BLENDVERTEX;
	m_VBSize = D3DXGetFVFVertexSize(FVF_BLENDVERTEX)*NumVertices;
	m_IBSize = NumIndex*sizeof(WORD);
	hr = CreateIB( &m_lpIB, NumIndex*sizeof(WORD), 0 );
	if FAILED( hr ) return hr;
	hr = CreateVB( &m_lpVB1, NumVertices*D3DXGetFVFVertexSize(FVF_BLENDVERTEX), 0, FVF_BLENDVERTEX );
	if FAILED( hr ) return hr;
	hr = CreateVB( &m_lpVB2, NumVertices*D3DXGetFVFVertexSize(FVF_BLENDVERTEX), 0, FVF_BLENDVERTEX );
	if FAILED( hr ) return hr;
	//=======================================================
	//  ボーンテーブルの作成
	//========================================================

	SetMeshBone( Num1, Num2, pcp->Type, pcp->BoneTblNum, pBone, pBoneTbl, Flip );

	//==============================================================
	// Mesh Data inport
	//  Vertex Inport
	CUSTOMVERTEX*	pV;
	if( FAILED( m_lpVB1->Lock( 0,					// バッファの最初からデータを格納する。
						m_VBSize,					// ロードするデータのサイズ。
						(void**)&pV,				// 返されるインデックス データ。
						D3DLOCK_DISCARD ) ) )       // デフォルト フラグをロックに送る。
		return E_FAIL;
	if( pcp->Type&0x7f ) {	// クロス
		for( int i=0 ; i<NumVertices ; i++,pV++ ) {
			if( pUVidx[i] <Num1 ) {
				Conv1VertexC1(pV, pUVidx[i], pUVidx[i], pcp->Type, pBone, pVertexC, pUTemp[i], pVTemp[i], Flip);
			} else {
				Conv1VertexC2(pV, pUVidx[i], pUVidx[i]-Num1, pcp->Type, pBone, pVertexC2, pUTemp[i], pVTemp[i], Flip);
			}
		}
	} else {
		for( int i=0 ; i<NumVertices ; i++,pV++ ) {
			if( pUVidx[i] < Num1 ) {
				Conv1Vertex1( pV, pUVidx[i], pUVidx[i], pcp->Type, pBone, pVertex, pUTemp[i], pVTemp[i], Flip);
			} else {
				Conv1Vertex2( pV, pUVidx[i], pUVidx[i]-Num1, pcp->Type, pBone, pVertex2, pUTemp[i], pVTemp[i], Flip);
			}
		}
	}
	if( FAILED( hr = m_lpVB1->Unlock() ) ) {
		return hr;
	}
	if( FAILED( m_lpVB2->Lock( 0,					// バッファの最初からデータを格納する。
						m_VBSize,					// ロードするデータのサイズ。
						(void**)&pV,				// 返されるインデックス データ。
						D3DLOCK_DISCARD ) ) )       // デフォルト フラグをロックに送る。
		return E_FAIL;
	if( pcp->Type&0x7f ) {	// クロス
		for( int i=0 ; i<NumVertices ; i++,pV++ ) {
			if( pUVidx[i] <Num1 ) {
				Conv2VertexC1(pV, pUVidx[i], pUVidx[i], pcp->Type, pBone, pVertexC, pUTemp[i], pVTemp[i], Flip);
			} else {
				Conv2VertexC2(pV, pUVidx[i], pUVidx[i]-Num1, pcp->Type, pBone, pVertexC2, pUTemp[i], pVTemp[i], Flip);
			}
		}
	} else {
		for( int i=0 ; i<NumVertices ; i++,pV++ ) {
			if( pUVidx[i] <Num1 ) {
				Conv2Vertex1( pV, pUVidx[i], pUVidx[i], pcp->Type, pBone, pVertex, pUTemp[i], pVTemp[i], Flip);
			} else {
				Conv2Vertex2( pV, pUVidx[i], pUVidx[i]-Num1, pcp->Type, pBone, pVertex2, pUTemp[i], pVTemp[i], Flip);
			}
		}
	}
	if( FAILED( hr = m_lpVB2->Unlock() ) ) {
		return hr;
	}
	SAFE_DELETES( pUVidx );
	SAFE_DELETES( pUTemp );
	SAFE_DELETES( pVTemp );

	// Face Inport
	char	  TexName[18];
	CMaterial *pMaterial=NULL;
    WORD*	pIndex;
	int		count,texNo=0;
	count = addpos	=	0;
	pH8010			=	NULL;

 	if( FAILED( m_lpIB->Lock( 0,                 // バッファの最初からデータを格納する。
						m_IBSize, // ロードするデータのサイズ。
						(void**)&pIndex, // 返されるインデックス データ。
						D3DLOCK_DISCARD ) ) )            // デフォルト フラグをロックに送る。
		return E_FAIL;
   while(addpos<pcp->PolyNum*2 ){
		FaceType	= (int)*(WORD*)(pPoly+addpos  );
		FaceNum		= (int)*(WORD*)(pPoly+addpos+2);
        if( FaceType==0xffff ) break;
        if( 0x8010 == (FaceType&0x80F0) ){
		  pH8010 = (H8010*)(pPoly+addpos);
          addpos+=0x2e; continue;
		} else if( 0x8000 == (FaceType&0x80F0) ){
			//===========================================
			// テクスチャ名称の設定
			strncpy(TexName,pPoly+addpos+2,16);
			texNo=0;
			pMaterial = (CMaterial*)pModel->m_Materials.Top();
			while ( pMaterial ) {
				if( !memcmp(TexName,pMaterial->m_Name,16) ){
					break;
				}
				texNo++;
				pMaterial = (CMaterial*)pMaterial->Next;
			}
			addpos+=0x12; continue; 
		} else if( 0x5453 == FaceType ) {
			pTexList = (TEXLIST *)(pPoly+addpos+4);
			CStream		*pStream;
			pStream = new CStream;
			m_Streams.InsertEnd( pStream );	m_Stnum++;
			pStream->SetData(D3DPT_TRIANGLESTRIP,count,FaceNum);
			pStream->m_texNo = texNo;
			pStream->m_pMaterial = pMaterial;
			if( pH8010 ) {
				pStream->SetDispLevel( (pH8010->s1>>8) );
			}
			count += 2 + FaceNum;
			*pIndex++ = pTexList[0].i1; 
			pStream->m_MinIdx = min( pStream->m_MinIdx,pTexList[0].i1);
			pStream->m_MaxIdx = max( pStream->m_MaxIdx,pTexList[0].i1);
			*pIndex++ = pTexList[0].i2;
			pStream->m_MinIdx = min( pStream->m_MinIdx,pTexList[0].i2);
			pStream->m_MaxIdx = max( pStream->m_MaxIdx,pTexList[0].i2);
			*pIndex++ = pTexList[0].i3;
			pStream->m_MinIdx = min( pStream->m_MinIdx,pTexList[0].i3);
			pStream->m_MaxIdx = max( pStream->m_MaxIdx,pTexList[0].i3);
			pTexList2 = (TEXLIST2 *)(pPoly+addpos+4+0x1e);
			for( int i=0 ; i<FaceNum-1 ; i++ ) {
				*pIndex++	=	pTexList2[i].i;
				pStream->m_MinIdx = min( pStream->m_MinIdx,pTexList2[i].i);
				pStream->m_MaxIdx = max( pStream->m_MaxIdx,pTexList2[i].i);
			}
			addpos+=FaceNum*10  + 0x18; NumFaces += FaceNum; continue; 
		} else if( 0x4353 == FaceType ){
          addpos+=FaceNum*20  + 0x0C; NumFaces += FaceNum; continue; 
		} else if( 0x0043 == FaceType ){
       addpos+=FaceNum*10  + 0x4;  NumFaces += FaceNum; continue; 
		} else if( 0x0054 == FaceType ){
			pTexList = (TEXLIST *)(pPoly+addpos+4);
			CStream		*pStream;
			pStream = new CStream;
			m_Streams.InsertEnd( pStream );	m_Stnum++;
			pStream->SetData(D3DPT_TRIANGLELIST,count,FaceNum);
			pStream->m_texNo = texNo;
			pStream->m_pMaterial = pMaterial;
			if( pH8010 ) {
				pStream->SetDispLevel( (pH8010->s1>>8) );
			}
			count += FaceNum * 3;
			for( int i=0 ; i<FaceNum ; i++ ) {
				*pIndex++	=	pTexList[i].i1;
				pStream->m_MinIdx = min( pStream->m_MinIdx,pTexList[i].i1);
				pStream->m_MaxIdx = max( pStream->m_MaxIdx,pTexList[i].i1);
				*pIndex++	=	pTexList[i].i2;
				pStream->m_MinIdx = min( pStream->m_MinIdx,pTexList[i].i2);
				pStream->m_MaxIdx = max( pStream->m_MaxIdx,pTexList[i].i2);
				*pIndex++	=	pTexList[i].i3;
				pStream->m_MinIdx = min( pStream->m_MinIdx,pTexList[i].i3);
				pStream->m_MaxIdx = max( pStream->m_MaxIdx,pTexList[i].i3);
			} 
			addpos+=FaceNum*30  + 0x4;  NumFaces += FaceNum; continue; 
		}
        break;
	} 
 	if( FAILED( hr = m_lpIB->Unlock() ) ) {
		return hr;
	}
	return hr;
}

// count number of Texture
int  CMesh::cntNumMaterial(void){
	int numTex = 0;

	CStream *pStream = (CStream*)m_Streams.Top();
	while (pStream != NULL) {
		if (pStream->m_pMaterial != NULL){
			numTex++;
		}
		pStream = (CStream*)pStream->Next;
	}
	return numTex;
}
// count number of face
int	 CMesh::cntNumFace(void){
	int dispCheck = GetDispCheck();
	int fCnt = 0, numFace = 0;

	CStream *pStream = (CStream*)m_Streams.Top();
	while (pStream != NULL) {
		fCnt += pStream->GetFaceCount();
		int	dispLevel = pStream->GetDispLevel();
		if (g_mPCFlag==false || dispLevel == 0 || dispLevel >= dispCheck) {
			numFace += pStream->GetFaceCount();
		}
		pStream = (CStream*)pStream->Next;
	}
	if (m_NumFaces != fCnt) {
		m_NumFaces = fCnt;
	}
	return numFace;
}
// print out Vertex data
bool CMesh::prtVertex(FILE *fd){
	bool notfirst = false;
	CUSTOMVERTEX	*pV;

	m_lpVB1->Lock(0, m_VBSize, (void**)&pV, D3DLOCK_DISCARD);

	//頂点数出力
	fprintf(fd, " %d;\n", m_NumVertices);
	// 頂点座標出力
	notfirst = false;
	for (unsigned int i = 0; i<m_NumVertices; i++, pV++) {
		if (notfirst)
			fprintf(fd, ",\n");
		else
			notfirst = true;
		if (fabs(pV->p.x)>10000.0 ||
			fabs(pV->p.y) > 10000.0 ||
			fabs(pV->p.z) > 10000.0)  {
			pV->p.x = 0.; pV->p.y = 0.; pV->p.z = 0.;
			pV->n.x = 0.; pV->n.y = 0.; pV->n.z = 0.;
			pV->u = 0.; pV->v = 0.;
			pV->b1 = 0.; pV->indx = 0;
		}
		fprintf(fd, "        %6.6f;%6.6f;%6.6f;", pV->p.x, pV->p.y, pV->p.z);
	}
	fprintf(fd, ";\n");
	m_lpVB1->Unlock();
	return true;
}
// print out Face data
bool CMesh::prtFace(FILE *fd) {
	int dispCheck = GetDispCheck();
	bool notfirst = false;
	CUSTOMVERTEX	*pV;
	WORD			*pI, *pIndex;
	int				i1, i2, i3, t1, t2, t3;	
	
	// 面数出力
	fprintf(fd, " %d;\n", m_NumFaces);
	// 面データ出力
	notfirst = false;
	m_lpIB->Lock(0, m_IBSize, (void**)&pIndex, D3DLOCK_DISCARD);
	m_lpVB1->Lock(0, m_VBSize, (void**)&pV, D3DLOCK_DISCARD);
	CStream *pStream = (CStream*)m_Streams.Top();
	while (pStream != NULL) {
		int	dispLevel = pStream->GetDispLevel();
		if (g_mPCFlag && dispLevel != 0 && dispLevel < dispCheck) {
			pStream = (CStream*)pStream->Next;
			continue;
		}
		pI = pIndex + pStream->GetIndexStart();
		if (pStream->m_PrimitiveType == D3DPT_TRIANGLESTRIP) {
			i1 = *pI++; i2 = *pI++;
			for (unsigned int i = 0; i < pStream->GetFaceCount(); i++) {
				i3 = *pI++;
				if (i % 2) {
					if (m_FlipFlag) {
						t1 = i3; t2 = i2; t3 = i1;
					}
					else {
						t1 = i1; t2 = i2; t3 = i3;
					}
				}
				else {
					if (m_FlipFlag) {
						t1 = i1; t2 = i2; t3 = i3;
					}
					else {
						t1 = i3; t2 = i2; t3 = i1;
					}
				}
				if (notfirst)
					fprintf(fd, ",\n");
				else
					notfirst = true;
				fprintf(fd, "  3;%d,%d,%d;", t1, t2, t3);
				i1 = i2; i2 = i3;
			}
		}
		else if (pStream->m_PrimitiveType == D3DPT_TRIANGLELIST) {
			for (unsigned int i = 0; i < pStream->GetFaceCount(); i++) {
				i1 = *pI++; i2 = *pI++; i3 = *pI++;
				if (m_FlipFlag) {
					t1 = i1; t2 = i2; t3 = i3;
				}
				else {
					t1 = i3; t2 = i2; t3 = i1;
				}
				if (notfirst)
					fprintf(fd, ",\n");
				else
					notfirst = true;
				fprintf(fd, "  3;%d,%d,%d;", t1, t2, t3);
			}
		}
		pStream = (CStream*)pStream->Next;
	}
	fprintf(fd, "  ;\n");
	m_lpVB1->Unlock();
	m_lpIB->Unlock();
	return true;
}

// print out Normal data
bool CMesh::prtNormal(FILE *fd){
	bool notfirst = false;
	CUSTOMVERTEX	*pV;

	m_lpVB1->Lock(0, m_VBSize, (void**)&pV, D3DLOCK_DISCARD);

	//頂点数出力
	fprintf(fd, " %d;\n", m_NumVertices);
	// 頂点座標出力
	notfirst = false;
	for (unsigned int i = 0; i<m_NumVertices; i++, pV++) {
		if (notfirst)
			fprintf(fd, ",\n");
		else
			notfirst = true;
		fprintf(fd, "        %6.6f;%6.6f;%6.6f;", pV->n.x, pV->n.y, pV->n.z);
	}
	fprintf(fd, ";\n");
	m_lpVB1->Unlock();
	return true;
}

// print out texture coord 
bool CMesh::prtTexCoord(FILE *fd){
	bool notfirst = false;
	CUSTOMVERTEX	*pV;

	fprintf(fd, "MeshTextureCoords {\n");
	// テクスチャ座標数出力
	fprintf(fd, " %d;\n", m_NumVertices);
	// テクスチャ座標データ出力
	notfirst = false;
	m_lpVB1->Lock(0, m_VBSize, (void**)&pV, D3DLOCK_DISCARD);
	for (unsigned int i = 0; i<m_NumVertices; i++, pV++) {
		if (notfirst)
			fprintf(fd, ",\n");
		else
			notfirst = true;
		if (i == 15) {
			int hh = 0;
		}
		if (pV->u < 0.0) pV->u = 0.0;
		if (pV->v < 0.0) pV->v = 0.0;
		if (pV->u > 1.0) pV->u = 1.0;
		if (pV->v > 1.0) pV->v = 1.0;
		fprintf(fd, "        %4.6f;%4.6f;", pV->u, pV->v);
	}
	fprintf(fd, ";\n");
	fprintf(fd, "      }\n");
	m_lpVB1->Unlock();
	return true;
}

// printout Material list
bool CMesh::prtMaterialList(FILE *fd) {
//	int numTex = cntNumMaterial();
	int noTex = 0, fCnt = 0;

	// マテリアル数出力
//	fprintf(fd, " %d;\n", numTex);
	fprintf(fd, " 1;\n");
	// 面数出力
	fprintf(fd, "  %d;\n", m_NumFaces);
	// マテリアル番号出力
	bool notfirst = false;

	WORD			*pI, *pIndex;
	int				i1, i2, i3, t1, t2, t3;
	CUSTOMVERTEX	*pV;
	int dispCheck = GetDispCheck();

	m_lpIB->Lock(0, m_IBSize, (void**)&pIndex, D3DLOCK_DISCARD);
	m_lpVB1->Lock(0, m_VBSize, (void**)&pV, D3DLOCK_DISCARD);
	CStream *pStream = (CStream*)m_Streams.Top();
	while (pStream != NULL) {
		int	dispLevel = pStream->GetDispLevel();
		if ( g_mPCFlag && dispLevel != 0 && dispLevel < dispCheck) {
			pStream = (CStream*)pStream->Next;
			continue;
		}
		pI = pIndex + pStream->GetIndexStart();
		if (pStream->m_PrimitiveType == D3DPT_TRIANGLESTRIP) {
			i1 = *pI++; i2 = *pI++;
			for (unsigned int i = 0; i < pStream->GetFaceCount(); i++) {
				i3 = *pI++;
				if (i % 2) {
					if (m_FlipFlag) {
						t1 = i3; t2 = i2; t3 = i1;
					}
					else {
						t1 = i1; t2 = i2; t3 = i3;
					}
				}
				else {
					if (m_FlipFlag) {
						t1 = i1; t2 = i2; t3 = i3;
					}
					else {
						t1 = i3; t2 = i2; t3 = i1;
					}
				}
				if (notfirst)
					fprintf(fd, ",\n");
				else
					notfirst = true;
				fprintf(fd, "  0");
				//fprintf(fd, "  %d", pStream->m_texNo);
				i1 = i2; i2 = i3;
			}
		}
		else if (pStream->m_PrimitiveType == D3DPT_TRIANGLELIST) {
			for (unsigned int i = 0; i < pStream->GetFaceCount(); i++) {
				i1 = *pI++; i2 = *pI++; i3 = *pI++;
				if (m_FlipFlag) {
					t1 = i1; t2 = i2; t3 = i3;
				}
				else {
					t1 = i3; t2 = i2; t3 = i1;
				}
				if (notfirst)
					fprintf(fd, ",\n");
				else
					notfirst = true;
				fprintf(fd, "  0");
//				fprintf(fd, "  %d", pStream->m_texNo);
			}
		}
		fCnt += pStream->GetFaceCount();
		pStream = (CStream*)pStream->Next;
	}
	fprintf(fd, ";\n");
	m_lpVB1->Unlock();
	m_lpIB->Unlock();
	return true;
}
// print out material 
bool CMesh::prtMaterial(char *FPath, char *FName, FILE *fd) {
	CStream *pStream = (CStream*)m_Streams.Top();
	if (pStream->m_pMaterial == NULL) return false;
	char texName[256]; strcpy(texName, (pStream->m_pMaterial)->m_Name); Trim(texName);
	fprintf(fd, " Material {\n");
	fprintf(fd, " 1.000;1.000;1.000;1.000;;\n");
	fprintf(fd, " 0.000;\n");
	fprintf(fd, " 1.000;1.000;1.000;;\n");
	fprintf(fd, " 0.000;0.000;0.000;;\n");
	fprintf(fd, "  TextureFilename {\n");
	fprintf(fd, "  \"%s.bmp\";\n", texName);
	fprintf(fd, "  }\n");
	fprintf(fd, " }\n");
	return true;
}


int CMesh::cntBone2Ver(int boneNo) {
	int	 boneCnt = 0;

	CUSTOMVERTEX	*pV1, *pV2;
	m_lpVB1->Lock(0, m_VBSize, (void**)&pV1, D3DLOCK_DISCARD);
	m_lpVB2->Lock(0, m_VBSize, (void**)&pV2, D3DLOCK_DISCARD);
	for (unsigned int i = 0; i<m_NumVertices; i++, pV1++, pV2++) {
		if (pV1->b1>0.f && m_pBoneTbl[pV1->indx] == boneNo) {
			boneCnt++;
		}
		else if (pV2->b1>0.f && m_pBoneTbl[pV2->indx] == boneNo) {
			boneCnt++;
		}
	}
	m_lpVB1->Unlock();
	m_lpVB2->Unlock();
	return boneCnt;
}

bool CMesh::prtBone2VerNo(FILE *fd, int boneNo) {
	bool notfirst = false;
	CUSTOMVERTEX	*pV1, *pV2;

	m_lpVB1->Lock(0, m_VBSize, (void**)&pV1, D3DLOCK_DISCARD);
	m_lpVB2->Lock(0, m_VBSize, (void**)&pV2, D3DLOCK_DISCARD);
	for (unsigned int i = 0; i<m_NumVertices; i++, pV1++, pV2++) {
		if (pV1->b1>0.f && m_pBoneTbl[pV1->indx] == boneNo) {
			if (notfirst)
				fprintf(fd, ",\n");
			else
				notfirst = true;
			fprintf(fd, "  %5d", i);
		}
		else if (pV2->b1>0.f && m_pBoneTbl[pV2->indx] == boneNo) {
			if (notfirst)
				fprintf(fd, ",\n");
			else
				notfirst = true;
			fprintf(fd, "  %5d", i);
		}
	}
	fprintf(fd, ";\n");
	m_lpVB1->Unlock();
	m_lpVB2->Unlock();
	return true;
}

bool CMesh::prtBone2VerWeight(FILE *fd, int boneNo) {
	bool notfirst = false;
	CUSTOMVERTEX	*pV1, *pV2;

	m_lpVB1->Lock(0, m_VBSize, (void**)&pV1, D3DLOCK_DISCARD);
	m_lpVB2->Lock(0, m_VBSize, (void**)&pV2, D3DLOCK_DISCARD);
	for (unsigned int i = 0; i<m_NumVertices; i++, pV1++, pV2++) {
		if (pV1->b1>0.f && m_pBoneTbl[pV1->indx] == boneNo) {
			if (notfirst)
				fprintf(fd, ",\n");
			else
				notfirst = true;
			fprintf(fd, "  %1.6f", pV1->b1);
		}
		else if (pV2->b1>0.f && m_pBoneTbl[pV2->indx] == boneNo) {
			if (notfirst)
				fprintf(fd, ",\n");
			else
				notfirst = true;
			fprintf(fd, "  %1.6f", pV2->b1);
		}
	}
	fprintf(fd, ";\n");
	m_lpVB1->Unlock();
	m_lpVB2->Unlock();
	return true;
}


//------------------------------------------------------------------------------------------//																				//
//									CBone													//																		//
//------------------------------------------------------------------------------------------//

//======================================================================
//		コンストラクタ
//======================================================================
CBone::CBone()
{
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
	strncpy(m_Name,MotionName,4);m_Name[4]='\0';
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
	m_pRotationKeys = new KEYROTATION [ Keys ];
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
	m_pScalingKeys = new KEYSCALING [ Keys ];
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
	m_pTranslateKeys = new KEYTRANSLATION [ Keys ];
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
	pRotationKeys	= new KEYROTATION [RotationKeyNum];
	for( i=0 ; i<RotationKeyNum ; i++ ) {
		pRotationKeys[i] = m_pRotationKeys[i];
	}
	SAFE_DELETES( m_pRotationKeys );
	m_RotationKeyNum = RotationKeyNum + Keys;
	m_pRotationKeys = new KEYROTATION [ m_RotationKeyNum ];
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
	pScalingKeys	= new KEYSCALING [ScalingKeyNum];
	for( i=0 ; i<ScalingKeyNum ; i++ ) {
		pScalingKeys[i] = m_pScalingKeys[i];
	}
	SAFE_DELETES( m_pScalingKeys );
	m_ScalingKeyNum = ScalingKeyNum + Keys;
	m_pScalingKeys = new KEYSCALING [ m_ScalingKeyNum ];
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
	pTranslateKeys	= new KEYTRANSLATION [TranslateKeyNum];
	for( i=0 ; i<TranslateKeyNum ; i++ ) {
		pTranslateKeys[i] = m_pTranslateKeys[i];
	}
	SAFE_DELETES( m_pTranslateKeys );
	m_TranslateKeyNum = TranslateKeyNum + Keys;
	m_pTranslateKeys = new KEYTRANSLATION [ m_TranslateKeyNum ];
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
	m_pRotationKeys = new KEYROTATION [ m_RotationKeyNum ];
	for ( i = 0; i < m_RotationKeyNum ; i++ ) {
		m_pRotationKeys[i]		= pAnimElement->m_pRotationKeys[i];
	}
	//-----------------------------------------------------
	// スケール
	//-----------------------------------------------------
	m_ScalingKeyNum = pAnimElement->m_ScalingKeyNum;
	m_pScalingKeys = new KEYSCALING [ m_ScalingKeyNum ];
	for ( i = 0; i < m_ScalingKeyNum ; i++ ) {
		m_pScalingKeys[i]		= pAnimElement->m_pScalingKeys[i];
	}
	//-----------------------------------------------------
	// 移動
	//-----------------------------------------------------
	m_TranslateKeyNum = pAnimElement->m_TranslateKeyNum;
	m_pTranslateKeys = new KEYTRANSLATION [ m_TranslateKeyNum ];
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
	pRotationKeys	= new KEYROTATION [RotationKeyNum];
	for( i=0 ; i<RotationKeyNum ; i++ ) {
		pRotationKeys[i] = m_pRotationKeys[i];
	}
	SAFE_DELETES( m_pRotationKeys );
	m_RotationKeyNum = RotationKeyNum + pAnimElement->m_RotationKeyNum;
	m_pRotationKeys = new KEYROTATION [ m_RotationKeyNum ];
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
	pScalingKeys	= new KEYSCALING [ScalingKeyNum];
	for( i=0 ; i<ScalingKeyNum ; i++ ) {
		pScalingKeys[i] = m_pScalingKeys[i];
	}
	SAFE_DELETES( m_pScalingKeys );
	m_ScalingKeyNum = ScalingKeyNum + pAnimElement->m_ScalingKeyNum;
	m_pScalingKeys = new KEYSCALING [ m_ScalingKeyNum ];
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
	pTranslateKeys	= new KEYTRANSLATION [TranslateKeyNum];
	for( i=0 ; i<TranslateKeyNum ; i++ ) {
		pTranslateKeys[i] = m_pTranslateKeys[i];
	}
	SAFE_DELETES( m_pTranslateKeys );
	m_TranslateKeyNum = TranslateKeyNum + pAnimElement->m_TranslateKeyNum;
	m_pTranslateKeys = new KEYTRANSLATION [ m_TranslateKeyNum ];
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
//------------------------------------------------------------------------------------------//																						//
//									CData													//																				//
//------------------------------------------------------------------------------------------//
//======================================================================
//		コンストラクタ
//======================================================================
CData::CData()
{
	m_Time					= 0;
	m_VertexFormat			= NULL;
	m_VertexSize			= 0;
	m_hVertexShader			= NULL;
	m_Materials.Init();

	D3DXMatrixIdentity( &m_mRootTransform );
}

//======================================================================
//		デストラクタ
//======================================================================
CData::~CData()
{
	m_Materials.Release();
}

//======================================================================
//
//		テクスチャの読み込み
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
HRESULT CData::LoadTextureFromFile( char *FileName  )
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
	    pdat = new char[dwSize];
	    ReadFile(hFile,pdat,dwSize,&cnt,NULL);
	    CloseHandle(hFile);
		hr = 0;
	} else {
		return -1;
	}

	//====================================================
	// テクスチャの読み込み
	//====================================================
	int			type,pos=0,next;
	while( pos<dwSize ) {
		next = *((int*)(pdat+pos+4));next>>=3;next&=0x7ffff0;
		if( next<16 ) break; 
		if( next+pos>dwSize ) break;
		type = *((int*)(pdat+pos+4));type &=0x7f;
		switch( type ) {
		case 0x20 :
			CMaterial *pMaterial;
			pMaterial = new CMaterial;
			m_Materials.InsertEnd( pMaterial );
			// テクスチャ
			char	TexName[18];
			strncpy(TexName,pdat+pos+16+1,16);TexName[16]='\0';
			pMaterial->SetName(TexName);
			LPDIRECT3DTEXTURE9 pTex;
			UINT xx,yy;
			xx = *(UINT*)(pdat+pos+33+4);
			yy = *(UINT*)(pdat+pos+33+8);
			D3DLOCKED_RECT rc;
			if( *(DWORD*)(pdat+pos+33+0x28) == 'DXT3' ){
				hr = GetDevice()->CreateTexture(xx,yy,0,0 ,D3DFMT_DXT3,D3DPOOL_MANAGED,&pTex,NULL);
				if( hr!=D3D_OK ) break;
				hr = pTex->LockRect(0,&rc,NULL,0);
				if(hr==D3D_OK){
				  CopyMemory(rc.pBits,pdat+pos+33+0x28+12,(xx/4) * (yy/4) * 16 );
				  pTex->UnlockRect(0);
				}
			} else if( *(DWORD*)(pdat+pos+33+0x28) == 'DXT1' ){
				hr = GetDevice()->CreateTexture(xx,yy,0,0,D3DFMT_DXT1,D3DPOOL_MANAGED,&pTex,NULL);
				if( hr!=D3D_OK ) break;
				hr = pTex->LockRect(0,&rc,NULL,0);
				if(hr==D3D_OK){
				  CopyMemory(rc.pBits,pdat+pos+33+0x28+12,(xx/4) * (yy/4) * 8  );
				  pTex->UnlockRect(0);
				}
			} else {
				hr = GetDevice()->CreateTexture(xx,yy,0,0,D3DFMT_A8R8G8B8,D3DPOOL_MANAGED,&pTex,NULL);
				if( hr!=D3D_OK ) return NULL;
				hr = pTex->LockRect(0,&rc,NULL,0);
				if(hr==D3D_OK){
					for( DWORD jy=0; jy<yy; jy++ ){
						for( DWORD jx=0; jx<xx; jx++ ){
							DWORD *pp  = (DWORD *)rc.pBits;
							BYTE  *idx = (BYTE  *)(pdat+pos+33+0x28+0x400);
							DWORD *pal = (DWORD *)(pdat+pos+33+0x28);
							pp[(yy-jy-1)*xx+jx] = pal[idx[jy*xx+jx]];
						}
					}
				}
				pTex->UnlockRect(0);
			}
			pMaterial->SetTexture( pTex );
			SAFE_RELEASE( pTex );
			break;
		}
		pos+=next;
	}
	// 終了
	delete pdat;
	return hr;
}


//======================================================================
//
//		モデルの行列初期化
//
//		行列を初期化します。
//
//======================================================================
void CData::InitTransform( void )
{
	D3DXMatrixIdentity( &m_mRootTransform );
}
 

//======================================================================
//
//		行列取得
//
//		モデルの行列に取得します。
//
//	input
//		D3DXMATRIX &mat		: 取得した行列を格納する行列
//
//======================================================================
void CData::GetMatrix( D3DXMATRIX &mat )
{
	mat = m_mRootTransform;
}

//======================================================================
//
//		行列設定
//
//		モデルの行列に任意の行列に設定。
//
//	input
//		D3DXMATRIX &mat		: 設定する行列
//
//======================================================================
void CData::SetMatrix( D3DXMATRIX &mat )
{
	m_mRootTransform = mat;
}

//======================================================================
//
//		行列乗算
//
//		モデルの行列に任意の行列を乗算します。
//
//	input
//		D3DXMATRIX &mat		: 乗算する行列
//
//======================================================================
void CData::MulMatrix( D3DXMATRIX &mat )
{
	m_mRootTransform *= mat;
}

//======================================================================
//
//		移動
//
//		モデルを移動させます。
//
//	input
//		float px		: Ｘ軸方向への移動量
//		float py		: Ｙ軸方向への移動量
//		float pz		: Ｚ軸方向への移動量
//
//======================================================================
void CData::Translation( float px, float py, float pz )
{
	D3DXMATRIX mat;
	D3DXMatrixTranslation( &mat, px, py, pz );
	MulMatrix( mat );
}


//======================================================================
//
//		拡大縮小
//
//		拡大縮小させます。
//		拡大縮小はモデルの法線にも影響が出ますので、
//		其の場合はレンダリング時に法線の正規化が必要です。
//
//	input
//		float sx		: Ｘ方向の拡大率
//		float sy		: Ｙ方向の拡大率
//		float sz		: Ｚ方向の拡大率
//
//======================================================================
void CData::Scaling( float sx, float sy, float sz )
{
	D3DXVECTOR3 pos;
	D3DXMATRIX	m1,m2,m3;

	GetWorldPosition( pos );
	D3DXMatrixTranslation( &m1, -pos.x, -pos.y, -pos.z );
	D3DXMatrixScaling( &m2, sx, sy, sz );
	D3DXMatrixTranslation( &m3, pos.x, pos.y, pos.z );
	m1 = m1*m2*m3;
	MulMatrix( m1 );
}


//======================================================================
//
//		Ｘ軸回転
//
//		Ｘ軸で回転させます。
//
//	input
//		float rot			: 回転角度（ラジアンで指定
//
//======================================================================
void CData::RotationX( float rot )
{
	D3DXMATRIX mat;
	D3DXMatrixRotationX( &mat, rot );
	MulMatrix( mat );
}

//======================================================================
//
//		Ｙ軸回転
//
//		Ｙ軸で回転させます。
//
//	input
//		float rot			: 回転角度（ラジアンで指定
//
//======================================================================
void CData::RotationY( float rot )
{
	D3DXMATRIX mat;
	D3DXMatrixRotationY( &mat, rot );
	MulMatrix( mat );
}

//======================================================================
//
//		Ｚ軸回転
//
//		Ｚ軸で回転させます。
//
//	input
//		float rot			: 回転角度（ラジアンで指定
//
//======================================================================
void CData::RotationZ( float rot )
{
	D3DXMATRIX mat;
	D3DXMatrixRotationZ( &mat, rot );
	MulMatrix( mat );
}

//======================================================================
//
//		指定位置を中心に回転
//
//		指定位置でY軸で回転させます。
//
//	input
//		float pos			: 回転中心位置
//		float rot			: 回転角度（ラジアンで指定
//
//======================================================================
void CData::RotationCenter( D3DXVECTOR3 pos, float rot )
{
	D3DXMATRIX mat;
	D3DXMatrixTranslation( &mat, -pos.x, -pos.y, -pos.z );
	MulMatrix( mat );
	D3DXMatrixRotationY( &mat, rot );
	MulMatrix( mat );
	D3DXMatrixTranslation( &mat, pos.x, pos.y, pos.z );
	MulMatrix( mat );
}
//======================================================================
//
//		その位置を中心に回転
//
//		その位置でY軸で回転させます。
//
//	input
//		float rot			: 回転角度（ラジアンで指定
//
//======================================================================
void CData::RotationDir( float rot )
{
	D3DXMATRIX mat;
	D3DXVECTOR3 pos;

	GetMatrix( mat );
	pos.x = mat._41;
	pos.y = mat._42;
	pos.z = mat._43;
	D3DXMatrixTranslation( &mat, -pos.x, -pos.y, -pos.z );
	MulMatrix( mat );
	D3DXMatrixRotationY( &mat, rot );
	MulMatrix( mat );
	D3DXMatrixTranslation( &mat, pos.x, pos.y, pos.z );
	MulMatrix( mat );
}

//======================================================================
//
//		Ｘ軸ミラー
//
//		Ｘ軸でミラーさせます。
//
//	input
//		none
//
//======================================================================
void CData::MirrorX( void )
{
	D3DXMATRIX mat=matrixMirrorX;
	MulMatrix( mat );
}

//======================================================================
//
//		Ｙ軸ミラー
//
//		Ｙ軸でミラーさせます。
//
//	input
//		none
//======================================================================
void CData::MirrorY( void )
{
	D3DXMATRIX mat=matrixMirrorY;
	//D3DXMATRIX mat;
	//D3DXMatrixRotationZ(&mat, PAI);
	SetMatrix(mat);
	//MulMatrix(mat);
}

//======================================================================
//
//		Ｚ軸ミラー
//
//		Ｚ軸でミラーさせます。
//
//	input
//		none
//
//======================================================================
void CData::MirrorZ( void )
{
	D3DXMATRIX mat=matrixMirrorZ;
	MulMatrix( mat );
}
//======================================================================
//
//		注視
//
//		現在のワールド空間位置から at 方向を注視します。
//		このとき上方向は up ベクトルで指定します。
//
//	input
//		D3DXVECTOR3 &at		: 注視点
//		D3DXVECTOR3 &up		: 上方向（省略時 { 0, 1, 0 }
//
//	output
//		true 成功 / false 失敗（近すぎる
//
//======================================================================
bool CData::LookAt( D3DXVECTOR3 &at, D3DXVECTOR3 &up )
{
	float length;
	D3DXVECTOR3 from, right;

	// モデルの位置を取得
	GetWorldPosition( from );

	// 視点のＺベクトルを取得
	D3DXVECTOR3 eye = at - from;
	length = D3DXVec3Length( &eye );
	if ( length < 1e-6f ) return false;
	// 正規化
	eye *= 1.0f/length;

	up -= -D3DXVec3Dot( &up, &eye ) * eye;
	length = D3DXVec3Length( &up );

	// 正しいアップベクトルの検索
	if ( length < 1e-6f )
	{
		up = D3DXVECTOR3(0,1,0) - eye.y*eye;
		length = D3DXVec3Length( &up );

		if ( length < 1e-6f )
		{
			up = D3DXVECTOR3(0,0,1) - eye.z*eye;
			length = D3DXVec3Length( &up );

			if ( length < 1e-6f) return false;
		}
	}

	// Ｙ及びＸベクトルを取得
	up *= 1.0f/length;
	D3DXVec3Cross( &right, &up, &eye );

	// オブジェクトを注視
	D3DXMATRIX mat;
	GetMatrix( mat );
	mat._11 = right.x;	mat._12 = right.y;	mat._13 = right.z;	mat._14 = 0;
	mat._21 = up.x;		mat._22 = up.y;		mat._23 = up.z;		mat._24 = 0;
	mat._31 = eye.x;	mat._32 = eye.y;	mat._33 = eye.z;	mat._34 = 0;
	mat._41 = from.x;	mat._42 = from.y;	mat._43 = from.z;	mat._43 = 1;
	SetMatrix( mat );

	return true;
}

//======================================================================
//
//		ワールド空間での位置
//
//		ワールド空間での位置を取得します。
//
//	input
//		D3DXVECTOR3 &pos		: 位置を格納するベクトル
//
//======================================================================
void CData::GetWorldPosition( D3DXVECTOR3 &pos )
{
	D3DXMATRIX mat;
	GetMatrix( mat );
	pos.x = mat._41;
	pos.y = mat._42;
	pos.z = mat._43;
}

//======================================================================
//
//		X軸を取り出します。
//
//	input
//		D3DXVECTOR3 &pos		: X軸を格納するベクトル
//
//======================================================================
void CData::GetXAxis( D3DXVECTOR3 &pos )
{
	D3DXMATRIX mat;
	GetMatrix( mat );
	pos.x = mat._11;
	pos.y = mat._12;
	pos.z = mat._13;
}

//======================================================================
//
//		Y軸を取り出します。
//
//	input
//		D3DXVECTOR3 &pos		: Y軸を格納するベクトル
//
//======================================================================
void CData::GetYAxis( D3DXVECTOR3 &pos )
{
	D3DXMATRIX mat;
	GetMatrix( mat );
	pos.x = mat._21;
	pos.y = mat._22;
	pos.z = mat._23;
}

//======================================================================
//
//		Z軸を取り出します。
//
//	input
//		D3DXVECTOR3 &pos		: Z軸を格納するベクトル
//
//======================================================================
void CData::GetZAxis( D3DXVECTOR3 &pos )
{
	D3DXMATRIX mat;
	GetMatrix( mat );
	pos.x = mat._31;
	pos.y = mat._32;
	pos.z = mat._33;
}

//======================================================================
//
//		スクリーンでの位置
//
//		カメラの前方にある場合はX,Y座標は0～1が格納される。
//		カメラの後方にある場合の値は不定。
//
//	input
//		D3DXVECTOR4 &pos		: 位置を格納するベクトル
//
//	output
//		true カメラの前 / false カメラの後ろ
//
//======================================================================
bool CData::GetScreenPosition( D3DXVECTOR4 &pos )
{
	// オブジェクト×ビュー×投影のマトリクスを取得
	D3DXMATRIX mat;
	GetMatrix( mat );
	D3DXMATRIX mTransform = mat * g_mView * g_mProjection;

	// スクリーン座標の算出
	float x = mTransform._41;
	float y = mTransform._42;
	float z = mTransform._43;
	float w = mTransform._44;
	float sz = z / w;

	// オブジェクトがカメラの前方にある場合
	if ( (sz >= 0.0f) && (sz < w) )
	{
		pos.x = (1.0f+(x/w)) * 0.5f;
		pos.y = (1.0f-(y/w)) * 0.5f;
		pos.z = sz;
		pos.w = w;

		return true;
	}

	return false;
}

 
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
	    pdat = new char[dwSize];
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
	delete pdat;
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

	DAT2AH	*pHeader;
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
	    pdat = new char[dwSize];
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
	delete pdat;
//	return hr;
	pos = 0;
	hFile = CreateFile(FileName,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_ARCHIVE,NULL);
	if( hFile!=INVALID_HANDLE_VALUE ){
		dwSize = GetFileSize(hFile,NULL);
	    pdat = new char[dwSize];
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
	delete pdat;
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
	    pdat = new char[dwSize];
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
	delete pdat;
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
	    pdat = new char[dwSize];
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
	delete pdat;
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

//======================================================================
//		Xファイルセーブ
//		データをMQOフォーマットで出力します
//======================================================================
bool CModel::saveX(char *FPath, char *FName)
{
	FILE *fd;
	char *ptr, strmsg[256], path[256];
	D3DXMATRIX lmatrix,rootMatrix;

	D3DXMATRIX mat;
	D3DXMatrixRotationY(&mat, (float)(PAI/2.));
	D3DXMatrixRotationZ(&rootMatrix, (float)PAI);
	rootMatrix *= mat;
	D3DXMatrixIdentity(&lmatrix);

	if ((ptr = strstr(FPath, ".x"))) *ptr = '\0';
	if ((ptr = strstr(FName, ".x"))) *ptr = '\0';
	sprintf(path, "%s.x", FPath);
	HANDLE hFile = CreateFile(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_ARCHIVE, NULL);
	if (hFile != INVALID_HANDLE_VALUE) {
		CloseHandle(hFile);
		wsprintf(strmsg, "ファイル %s　を上書きしますか？", path);
		if (MessageBox(NULL, strmsg, "セーブファイル上書き", MB_YESNO | MB_ICONQUESTION) == IDNO) return false;
	}
	if ((fd = fopen(path, "w")) == NULL) return false;
	fprintf(fd, "xof 0303txt 0032\n\n");
	fprintf(fd, "AnimTickPerSecond {\n\t\t3000;\n\t}\n");
	fprintf(fd, " Frame Scene_Root {\n");
	outputMatrix(fd, &rootMatrix);
	fprintf(fd, " Frame body {\n");
	outputMatrix(fd, &lmatrix); 
#if 0
	outputMultiMeshX(FPath, FName, fd); // スキンマルチメッシュ　部品交換用
#else
	fprintf(fd, " Mesh {\n"); // スキンワンメッシュ用
	outputMeshX(FPath, FName, fd);// スキンワンメッシュ用 
	outputSkinX(fd);// スキンワンメッシュ用
	fprintf(fd, "}\n");// スキンワンメッシュ用
#endif
	fprintf(fd, "}\n");
	outputFrameX(fd);
	fprintf(fd, "}\n");
	std::vector<std::string> strlist;
	strlist.clear();
	char nameback[8]; strcpy(nameback, m_MotionName);
	if (g_mPCFlag) {
		CMotionFrame *pMotionFrame = (CMotionFrame*)pPC->m_motions.Top();
		int cnt = 0;
		while (pMotionFrame) {
			char mName[6]; strncpy(mName, (char*)pMotionFrame->m_Name, 3); mName[3] = '\0';
			int k = 0;
			for (; k < (int)strlist.size(); k++) {
				if (strlist[k] == mName) break;
			}
			if (k>=(int)strlist.size()) {
				strlist.push_back(mName);
				pPC->SetMotionName(mName);
				pPC->LoadPCMotion();
				outputAnimationSet(fd, mName);
			}
			pMotionFrame = (CMotionFrame*)pMotionFrame->Next;
		}
		pPC->SetMotionName(nameback);
		pPC->LoadPCMotion();
	}
	else {
		CMotionFrame *pMotionFrame = (CMotionFrame*)pNPC->m_motions.Top();
		int cnt = 0;
		while (pMotionFrame) {
			char mName[6]; strncpy(mName, (char*)pMotionFrame->m_Name, 3); mName[3] = '\0';
			int k = 0;
			for (; k < (int)strlist.size(); k++) {
				if (strlist[k] == mName) break;
			}
			if (k>=(int)strlist.size()) {
				strlist.push_back(mName);
				pNPC->SetMotionName(mName);
				pNPC->LoadNPCMotion();
				outputAnimationSet(fd, mName);
			}
			pMotionFrame = (CMotionFrame*)pMotionFrame->Next;
		}
		pNPC->SetMotionName(nameback);
		pNPC->LoadNPCMotion();
	}
	fclose(fd);
	strlist.clear(); 
	return true;
}

bool CModel::outputConvMatrix(FILE *fd, D3DXMATRIX *iMatrix) {
	if (fd == NULL || iMatrix == NULL) return false;
	D3DXVECTOR3	vec,tvec;

	vec.x = iMatrix->_41; vec.y = iMatrix->_42; vec.z = iMatrix->_43;
	fprintf(fd, "    Translate %6.6f,%6.6f,%6.6f\n", vec.x, vec.y, vec.z);
	tvec.x = iMatrix->_11; tvec.y = iMatrix->_12; vec.z = iMatrix->_13;
	vec.x = D3DXVec3Length(&tvec);
	tvec.x = iMatrix->_21; tvec.y = iMatrix->_22; vec.z = iMatrix->_23;
	vec.y = D3DXVec3Length(&tvec);
	tvec.x = iMatrix->_31; tvec.y = iMatrix->_32; vec.z = iMatrix->_33;
	vec.z = D3DXVec3Length(&tvec);
	fprintf(fd, "    Scale %6.6f,%6.6f,%6.6f\n", vec.x, vec.y, vec.z);
	fprintf(fd, " FrameTransformMatrix {\n");
	outputMatrixSub(fd, iMatrix);
	fprintf(fd, " }\n");
	return true;
}

bool CModel::outputMatrix(FILE *fd, D3DXMATRIX *iMatrix) {

	if (fd == NULL || iMatrix == NULL) return false;
	fprintf(fd, " FrameTransformMatrix {\n");
	outputMatrixSub(fd, iMatrix);
	fprintf(fd, " }\n");
	return true;
}

bool CModel::outputMatrixSub(FILE *fd, D3DXMATRIX *iMatrix) {

	if (fd == NULL || iMatrix == NULL) return false;
	fprintf(fd, "  %6.6f,%6.6f,%6.6f,%6.6f,%6.6f,%6.6f,%6.6f,%6.6f,%6.6f,%6.6f,%6.6f,%6.6f,%6.6f,%6.6f,%6.6f,%6.6f;;\n"
		, iMatrix->_11, iMatrix->_12, iMatrix->_13, iMatrix->_14, iMatrix->_21, iMatrix->_22, iMatrix->_23, iMatrix->_24,
		iMatrix->_31, iMatrix->_32, iMatrix->_33, iMatrix->_34, iMatrix->_41, iMatrix->_42, iMatrix->_43, iMatrix->_44);
	return true;
}


bool CModel::outputMeshX(char *FPath, char *FName, FILE *fd) {
	char *ptr, fpath[256], texpath[256];

	if (fd == NULL) return false;
	strcpy(fpath, FPath);
	if ((ptr = strrstr(fpath, FName))) *ptr = '\0';
	CMaterial *pMaterial = (CMaterial*)m_Materials.Top();
	while (pMaterial != NULL)
	{
		char texName[256]; strcpy(texName, pMaterial->m_Name); Trim(texName);
		sprintf(texpath, "%s%s.bmp", fpath, texName);
		D3DXSaveTextureToFile(texpath, D3DXIFF_BMP, pMaterial->m_pTexture, NULL);
		pMaterial = (CMaterial*)pMaterial->Next;

	}
	outputVertex(fd);
	outputFace(fd);
	//法線出力
	fprintf(fd, "   MeshNormals {\n");
	outputNormal(fd);
	outputNormalFace(fd);
	fprintf(fd, "}\n");
	outputTexCoord(fd);
	outputVerDup(fd);
	outputMaterialList(FPath, FName, fd);
	return true;
}


bool CModel::outputMultiMeshX(char *FPath, char *FName, FILE *fd) {
	int partsNo = 1;
	char *ptr,fpath[256],texpath[256];
	D3DXMATRIX lmatrix;
	D3DXMatrixIdentity(&lmatrix);

	strcpy(fpath, FPath);
	if ( (ptr = strrstr(fpath, FName)) ) *ptr = '\0';
	CMaterial *pMaterial = (CMaterial*)m_Materials.Top();
	while (pMaterial != NULL)
	{
		char texName[256]; strcpy(texName, pMaterial->m_Name); Trim(texName);
		sprintf(texpath, "%s%s.bmp", fpath, texName);
		D3DXSaveTextureToFile(texpath, D3DXIFF_BMP, pMaterial->m_pTexture, NULL);
		pMaterial = (CMaterial*)pMaterial->Next;

	}
	CMesh *pMesh = (CMesh*)m_Meshs.Top();
	while (pMesh != NULL) {
		// 面数カウント
		int dispCheck = pMesh->GetDispCheck();
		int numFace = pMesh->cntNumFace();
		// フレーム出力
		fprintf(fd, " Frame Parts%03d {\n", partsNo);
		outputMatrix(fd, &lmatrix);
		// メッシュ名出力
		fprintf(fd, " Mesh Parts%03d {\n", partsNo);
		//fprintf(fd, " Mesh %s {\n", pMesh->m_Name);
		pMesh->prtVertex(fd);
		pMesh->prtFace(fd);
		//法線出力
		fprintf(fd, "MeshNormals {\n");
		// 法線頂点出力
		pMesh->prtNormal(fd);
		// 法線面数出力
		pMesh->prtFace(fd);
		fprintf(fd, "     }\n");
		// テクスチャ座標出力
		pMesh->prtTexCoord(fd);

		//　マテリアル出力
		fprintf(fd, " MeshMaterialList {\n");
		pMesh->prtMaterialList(fd);
		pMesh->prtMaterial(FPath, FName, fd);
		fprintf(fd, "}\n");
		fprintf(fd, "   XSkinMeshHeader {\n");
		fprintf(fd, "         2;\n");
		fprintf(fd, "         2;\n");
		int cntSkinWeights = 0;
		for (int i = 0; i < m_nBone; i++) {
			int	 boneCnt = pMesh->cntBone2Ver(i);
			if (boneCnt > 0) cntSkinWeights++;
		}
		fprintf(fd, "         %d;\n", cntSkinWeights);
		fprintf(fd, "  }\n");
		D3DXMATRIX iMatrix;
		D3DXMatrixIdentity(&iMatrix);
		for (int i = 0; i < m_nBone; i++) {
			int	 boneCnt = pMesh->cntBone2Ver(i);
			if (boneCnt <= 0) continue;
			fprintf(fd, " SkinWeights {\n");
			fprintf(fd, " \"Bone%03d\";\n", i);
			fprintf(fd, "   %d;\n", boneCnt);
			pMesh->prtBone2VerNo(fd, i);
			pMesh->prtBone2VerWeight(fd, i);
			iMatrix = m_Bones[i].m_mInvTrans;
			outputMatrixSub(fd, &iMatrix);
			fprintf(fd, " }\n");
		}
		fprintf(fd, "}\n"); // メッシュ終了
		fprintf(fd, "}\n"); // フレーム終了
		partsNo++;
		pMesh = (CMesh*)pMesh->Next;
	}
	return true;
}

bool CModel::outputSkinX(FILE *fd) {
	if (fd == NULL) return false;
	fprintf(fd, "   XSkinMeshHeader {\n");
	fprintf(fd, "         2;\n");
	fprintf(fd, "         2;\n");
	int cntSkinWeights=0;
	for (int i = 0; i < m_nBone; i++) {
		int	 boneCnt = countBone2Ver(i);
		if (boneCnt > 0) cntSkinWeights++;
	}
	fprintf(fd, "         %d;\n", cntSkinWeights);
	fprintf(fd, "  }\n");
	D3DXMATRIX iMatrix;
	D3DXMatrixIdentity(&iMatrix);
	for (int i = 0; i < m_nBone; i++) {
		int	 boneCnt = countBone2Ver(i);
		if (boneCnt <= 0) continue;
		fprintf(fd, " SkinWeights {\n");
		fprintf(fd, " \"Bone%03d\";\n", i);
		fprintf(fd, "   %d;\n", boneCnt);
		outputSkinWeights(fd, i);
		iMatrix = m_Bones[i].m_mInvTrans;
		outputMatrixSub(fd, &iMatrix);
		fprintf(fd, " }\n");
	}
	return true;
}

bool CModel::outputSkinWeights(FILE *fd, int boneNo) {
	outputBone2VerNo(fd, boneNo);
	outputBone2VerWeight(fd, boneNo);
	return true;
}


bool CModel::outputFrameX(FILE *fd) {
	D3DXMATRIX iMatrix;
	int cnt = 0;
	if (fd == NULL) return false;
	fprintf(fd, "  Frame Bone000 {\n");
	iMatrix = m_Bones[0].m_mTransform;
	//iMatrix = m_mRootTransform;
	outputMatrix(fd, &iMatrix);
	cnt = outputFrameXsub(fd, 0);
	fprintf(fd, " }\n");
	return true;
}

int CModel::outputFrameXsub(FILE *fd, int boneNo) {
	int noChild = 0;
	static int callCnt = 0;
	D3DXMATRIX iMatrix;
	D3DXMatrixIdentity(&iMatrix);
	for (int i = boneNo + 1; i < m_nBone; i++) {
		if (m_Bones[i].m_pParent != NULL &&
			m_Bones[i].m_mParent == boneNo) {
			//int	 boneCnt = countBone2Ver(i);
			//if (boneCnt > 0) {
				fprintf(fd, " Frame Bone%03d {\n", i);
				iMatrix = m_Bones[i].m_mTransform;
				outputMatrix(fd, &iMatrix);
				outputFrameXsub(fd, i);
				fprintf(fd, " }\n");
			//}
			//else {
			//	outputFrameXsub(fd, i);
			//}
			callCnt++;
		}
		else {
			noChild++;
		}
	}
	return callCnt;
}

bool CModel::outputAnimationSet(FILE *fd,char *nameMotion) {
	if (fd == NULL) return false;
	fprintf(fd, "  AnimationSet %s {\n", nameMotion);
	for (int i = 0; i < m_nBone; i++) {
		outputAnimationX(fd, i);
	}
	fprintf(fd, "}\n");
	return true;
}

bool CModel::outputAnimationX(FILE *fd, int boneNo) {

	if (fd == NULL) return false;
	D3DXMATRIX iMatrix;
	int tboneNo = boneNo;
	int keyNum = m_MotionArray[tboneNo].m_RotationKeyNum;
	int tkeyNum = keyNum;
	if (keyNum <= 0) {
		tboneNo = 0;
		tkeyNum = m_MotionArray[tboneNo].m_RotationKeyNum;
	}
	fprintf(fd, " Animation {\n");
	fprintf(fd, "   { Bone%03d }\n", boneNo);
	fprintf(fd, " AnimationKey {\n");
	fprintf(fd, " 4;\n");
	fprintf(fd, " %d;\n", tkeyNum);
	bool notfirst = false;
	for (int i = 0; i < tkeyNum; i++) {
		float fTime = (float)m_MotionArray[tboneNo].m_pRotationKeys[i].Time;
		if (keyNum > 0) {
			iMatrix = m_Bones[tboneNo].m_mTransform;
			iMatrix *= *(m_MotionArray[tboneNo].GetAnimationMatrix(i, &m_Bones[tboneNo].m_mTransform));
		}
		else {
			iMatrix = m_Bones[boneNo].m_mTransform;
		}
		if (notfirst)
			fprintf(fd, ",\n");
		else
			notfirst = true;
		fprintf(fd, " %d;16; %6.6f,%6.6f,%6.6f,%6.6f,%6.6f,%6.6f,%6.6f,%6.6f,%6.6f,%6.6f,%6.6f,%6.6f,%6.6f,%6.6f,%6.6f,%6.6f;;",
			(int)(fTime*0.01), iMatrix._11, iMatrix._12, iMatrix._13, iMatrix._14, iMatrix._21, iMatrix._22, iMatrix._23, iMatrix._24,
			iMatrix._31, iMatrix._32, iMatrix._33, iMatrix._34, iMatrix._41, iMatrix._42, iMatrix._43, iMatrix._44);
	}
	fprintf(fd, ";\n");
	fprintf(fd, "}\n");
	fprintf(fd, "}\n");
	return true;
}

int CModel::totalVertex(void) {
	int numVer = 0, numFace = 0;

	// 頂点数、面数カウント
	CMesh *pMesh = (CMesh*)m_Meshs.Top();
	while (pMesh != NULL)
	{
		numVer += pMesh->m_NumVertices;
		pMesh = (CMesh*)pMesh->Next;
	}
	return numVer;
}

int CModel::totalFace(void) {
	int numVer = 0, numFace = 0;

	// 頂点数、面数カウント
	CMesh *pMesh = (CMesh*)m_Meshs.Top();
	while (pMesh != NULL)
	{
		int dispCheck = pMesh->GetDispCheck();
		int fCnt = 0;
		CStream *pStream = (CStream*)pMesh->m_Streams.Top();
		while (pStream != NULL) {
			fCnt += pStream->GetFaceCount();
			int	dispLevel = pStream->GetDispLevel();
			if ( g_mPCFlag==false || dispLevel == 0 || dispLevel >= dispCheck) {
				numFace += pStream->GetFaceCount();
			}
			pStream = (CStream*)pStream->Next;
		}
		if (pMesh->m_NumFaces != fCnt) {
			pMesh->m_NumFaces = fCnt;
		}
		pMesh = (CMesh*)pMesh->Next;
	}
	return numFace;
}

bool CModel::outputVertex(FILE *fd) {
	int numVer = totalVertex();
	int numFace = totalFace();
	//頂点数出力
	fprintf(fd, " %d;\n", numVer);
	// 頂点座標出力
	bool notfirst = false;
	CMesh *pMesh = (CMesh*)m_Meshs.Top();
	while (pMesh != NULL) {
		CUSTOMVERTEX	*pV1;
		(pMesh->m_lpVB1)->Lock(0, pMesh->m_VBSize, (void**)&pV1, D3DLOCK_DISCARD);
		for (unsigned int i = 0; i<pMesh->m_NumVertices; i++, pV1++) {
			if (notfirst)
				fprintf(fd, ",\n");
			else
				notfirst = true;
			if (fabs(pV1->p.x)>10000.0 ||
				fabs(pV1->p.y) > 10000.0 ||
				fabs(pV1->p.z) > 10000.0)  {
				pV1->p.x = 0.; pV1->p.y = 0.; pV1->p.z = 0.;
				pV1->n.x = 0.; pV1->n.y = 0.; pV1->n.z = 0.;
				pV1->u = 0.; pV1->v = 0.;
				pV1->b1 = 0.; pV1->indx = 0;
			}
			fprintf(fd, "        %6.6f;%6.6f;%6.6f;", pV1->p.x, pV1->p.y, pV1->p.z);
		}
		//if (pMesh->Next!=NULL) fprintf(fd, ",\n");
		(pMesh->m_lpVB1)->Unlock();
		pMesh = (CMesh*)pMesh->Next;
	}
	fprintf(fd, ";\n");
	return true;
}


bool CModel::outputFace(FILE *fd) {
	int numVer = totalVertex();
	int numFace = totalFace();
	// 面数出力
	fprintf(fd, " %d;\n", numFace);
	// パッチ出力
	int vCnt = 0, fCnt = 0;
	bool notfirst = false;
	CMesh *pMesh = (CMesh*)m_Meshs.Top();
	while (pMesh != NULL) {
		WORD			*pI, *pIndex;
		int				i1, i2, i3, t1, t2, t3;
		CUSTOMVERTEX	*pV;
		int dispCheck = pMesh->GetDispCheck();
		(pMesh->m_lpIB)->Lock(0, pMesh->m_IBSize, (void**)&pIndex, D3DLOCK_DISCARD);
		(pMesh->m_lpVB1)->Lock(0, pMesh->m_VBSize, (void**)&pV, D3DLOCK_DISCARD);
		CStream *pStream = (CStream*)pMesh->m_Streams.Top();
		while (pStream != NULL) {
			int	dispLevel = pStream->GetDispLevel();
			if (g_mPCFlag && dispLevel != 0 && dispLevel < dispCheck) {
				pStream = (CStream*)pStream->Next;
				continue;
			}
			pI = pIndex + pStream->GetIndexStart();
			if (pStream->m_PrimitiveType == D3DPT_TRIANGLESTRIP) {
				i1 = *pI++; i2 = *pI++;
				for (unsigned int i = 0; i < pStream->GetFaceCount(); i++) {
					i3 = *pI++;
					if (i % 2) {
						if (pMesh->m_FlipFlag) {
							t1 = i3; t2 = i2; t3 = i1;
						}
						else {
							t1 = i1; t2 = i2; t3 = i3;
						}
					}
					else {
						if (pMesh->m_FlipFlag) {
							t1 = i1; t2 = i2; t3 = i3;
						}
						else {
							t1 = i3; t2 = i2; t3 = i1;
						}
					}
					if (notfirst)
						fprintf(fd, ",\n");
					else
						notfirst = true;
					fprintf(fd, "  3;%d,%d,%d;", t1 + vCnt, t2 + vCnt, t3 + vCnt);
					i1 = i2; i2 = i3;
				}
			}
			else if (pStream->m_PrimitiveType == D3DPT_TRIANGLELIST) {
				for (unsigned int i = 0; i < pStream->GetFaceCount(); i++) {
					i1 = *pI++; i2 = *pI++; i3 = *pI++;
					if (pMesh->m_FlipFlag) {
						t1 = i1; t2 = i2; t3 = i3;
					}
					else {
						t1 = i3; t2 = i2; t3 = i1;
					}
					if (notfirst)
						fprintf(fd, ",\n");
					else
						notfirst = true;
					fprintf(fd, "  3;%d,%d,%d;", t1 + vCnt, t2 + vCnt, t3 + vCnt);
				}
			}
			fCnt += pStream->GetFaceCount();
			pStream = (CStream*)pStream->Next;
		}
		(pMesh->m_lpVB1)->Unlock();
		(pMesh->m_lpIB)->Unlock();
		//if (pMesh->Next != NULL) fprintf(fd, ",\n");
		vCnt += pMesh->m_NumVertices;
		pMesh = (CMesh*)pMesh->Next;
	}
	fprintf(fd, ";\n");
	return true;
}

bool CModel::outputNormal(FILE *fd) {
	int numVer = totalVertex();
	int numFace = totalFace();

	// 法線数出力
	fprintf(fd, " %d;\n", numVer);
	// 法線ベクトル出力
	bool notfirst = false;
	CMesh *pMesh = (CMesh*)m_Meshs.Top();
	while (pMesh != NULL) {
		CUSTOMVERTEX	*pV1;
		(pMesh->m_lpVB1)->Lock(0, pMesh->m_VBSize, (void**)&pV1, D3DLOCK_DISCARD);
		for (unsigned int i = 0; i<pMesh->m_NumVertices; i++, pV1++) {
			if (notfirst)
				fprintf(fd, ",\n");
			else
				notfirst = true;
			fprintf(fd, "        %6.6f;%6.6f;%6.6f;", pV1->n.x, pV1->n.y, pV1->n.z);
		}
		//if (pMesh->Next != NULL) fprintf(fd, ",\n");
		(pMesh->m_lpVB1)->Unlock();
		pMesh = (CMesh*)pMesh->Next;
	}
	fprintf(fd, ";\n");
	return true;
}

bool CModel::outputNormalFace(FILE *fd) {
	int numVer = totalVertex();
	int numFace = totalFace();
	// 面数出力
	fprintf(fd, " %d;\n", numFace);
	// パッチ出力
	int vCnt = 0, fCnt = 0;
	bool notfirst = false;
	CMesh *pMesh = (CMesh*)m_Meshs.Top();
	while (pMesh != NULL) {
		WORD			*pI, *pIndex;
		int				i1, i2, i3, t1, t2, t3;
		CUSTOMVERTEX	*pV;
		int dispCheck = pMesh->GetDispCheck();
		(pMesh->m_lpIB)->Lock(0, pMesh->m_IBSize, (void**)&pIndex, D3DLOCK_DISCARD);
		(pMesh->m_lpVB1)->Lock(0, pMesh->m_VBSize, (void**)&pV, D3DLOCK_DISCARD);
		CStream *pStream = (CStream*)pMesh->m_Streams.Top();
		while (pStream != NULL) {
			int	dispLevel = pStream->GetDispLevel();
			if (g_mPCFlag && dispLevel != 0 && dispLevel < dispCheck) {
				pStream = (CStream*)pStream->Next;
				continue;
			}
			pI = pIndex + pStream->GetIndexStart();
			if (pStream->m_PrimitiveType == D3DPT_TRIANGLESTRIP) {
				i1 = *pI++; i2 = *pI++;
				for (unsigned int i = 0; i < pStream->GetFaceCount(); i++) {
					i3 = *pI++;
					if (i % 2) {
						if (pMesh->m_FlipFlag) {
							t1 = i3; t2 = i2; t3 = i1;
						}
						else {
							t1 = i1; t2 = i2; t3 = i3;
						}
					}
					else {
						if (pMesh->m_FlipFlag) {
							t1 = i1; t2 = i2; t3 = i3;
						}
						else {
							t1 = i3; t2 = i2; t3 = i1;
						}
					}
					if (notfirst)
						fprintf(fd, ",\n");
					else
						notfirst = true;
					fprintf(fd, "  3;%d,%d,%d;", t1 + vCnt, t2 + vCnt, t3 + vCnt);
					i1 = i2; i2 = i3;
				}
			}
			else if (pStream->m_PrimitiveType == D3DPT_TRIANGLELIST) {
				for (unsigned int i = 0; i < pStream->GetFaceCount(); i++) {
					i1 = *pI++; i2 = *pI++; i3 = *pI++;
					if (pMesh->m_FlipFlag) {
						t1 = i1; t2 = i2; t3 = i3;
					}
					else {
						t1 = i3; t2 = i2; t3 = i1;
					}
					if (notfirst)
						fprintf(fd, ",\n");
					else
						notfirst = true;
					fprintf(fd, "  3;%d,%d,%d;", t1 + vCnt, t2 + vCnt, t3 + vCnt);
				}
			}
			fCnt += pStream->GetFaceCount();
			pStream = (CStream*)pStream->Next;
		}
		(pMesh->m_lpVB1)->Unlock();
		(pMesh->m_lpIB)->Unlock();
		//if (pMesh->Next != NULL) fprintf(fd, ",\n");
		vCnt += pMesh->m_NumVertices;
		pMesh = (CMesh*)pMesh->Next;
	}
	fprintf(fd, ";\n");
	return true;
}

bool CModel::outputTexCoord(FILE *fd) {
	int numVer = totalVertex();
	int numFace = totalFace();

	// テクスチャu,v出力
	fprintf(fd, "   MeshTextureCoords {\n");
	// uv数出力
	fprintf(fd, " %d;\n", numVer);
	// テクスチャU,V出力
	bool notfirst = false;
	CMesh *pMesh = (CMesh*)m_Meshs.Top();
	while (pMesh != NULL) {
		CUSTOMVERTEX	*pV1;
		(pMesh->m_lpVB1)->Lock(0, pMesh->m_VBSize, (void**)&pV1, D3DLOCK_DISCARD);
		for (unsigned int i = 0; i<pMesh->m_NumVertices; i++, pV1++) {
			if (notfirst)
				fprintf(fd, ",\n");
			else
				notfirst = true;
			if (i == 15) {
				int hh = 0;
			}
			if (pV1->u < 0.0) pV1->u = 0.0;
			if (pV1->v < 0.0) pV1->v = 0.0;
			if (pV1->u > 1.0) pV1->u = 1.0;
			if (pV1->v > 1.0) pV1->v = 1.0;
			fprintf(fd, "        %4.6f;%4.6f;", pV1->u, pV1->v);
		}
		//if (pMesh->Next != NULL) fprintf(fd, ",\n");
		(pMesh->m_lpVB1)->Unlock();
		pMesh = (CMesh*)pMesh->Next;
	}
	fprintf(fd, ";\n");
	fprintf(fd, "}\n");
	return true;
}

bool CModel::outputVerDup(FILE *fd) {
	int numVer = totalVertex();
	int numFace = totalFace();
	// 頂点重複リスト
	fprintf(fd, " VertexDuplicationIndices {\n");
	fprintf(fd, "  %d;\n", numVer);
	fprintf(fd, "  %d;\n", numVer);
	for (int i = 0; i < numVer; i++) {
		if (i>0) fprintf(fd, ",\n");
		fprintf(fd, " %d", i);
	}
	fprintf(fd, ";\n");
	fprintf(fd, "}\n");
	return true;
}

bool CModel::outputMaterialList(char *FPath, char *FName, FILE *fd) {
	int numFace = totalFace();
	int vCnt = 0, fCnt = 0;
	// meshマテリアルリスト出力
	fprintf(fd, "  MeshMaterialList {\n");
	// マテリアル数出力
	fprintf(fd, "%d;\n", m_Materials.Count);
	// 面数出力
	fprintf(fd, " %d;\n", numFace);
	// マテリアル番号出力
	bool notfirst = false;
	CMesh *pMesh = (CMesh*)m_Meshs.Top();
	while (pMesh != NULL) {
		WORD			*pI, *pIndex;
		int				i1, i2, i3, t1, t2, t3;
		CUSTOMVERTEX	*pV;
		int dispCheck = pMesh->GetDispCheck();
		(pMesh->m_lpIB)->Lock(0, pMesh->m_IBSize, (void**)&pIndex, D3DLOCK_DISCARD);
		(pMesh->m_lpVB1)->Lock(0, pMesh->m_VBSize, (void**)&pV, D3DLOCK_DISCARD);
		CStream *pStream = (CStream*)pMesh->m_Streams.Top();
		while (pStream != NULL) {
			int	dispLevel = pStream->GetDispLevel();
			if (g_mPCFlag && dispLevel != 0 && dispLevel < dispCheck) {
				pStream = (CStream*)pStream->Next;
				continue;
			}
			pI = pIndex + pStream->GetIndexStart();
			if (pStream->m_PrimitiveType == D3DPT_TRIANGLESTRIP) {
				i1 = *pI++; i2 = *pI++;
				for (unsigned int i = 0; i < pStream->GetFaceCount(); i++) {
					i3 = *pI++;
					if (i % 2) {
						if (pMesh->m_FlipFlag) {
							t1 = i3; t2 = i2; t3 = i1;
						}
						else {
							t1 = i1; t2 = i2; t3 = i3;
						}
					}
					else {
						if (pMesh->m_FlipFlag) {
							t1 = i1; t2 = i2; t3 = i3;
						}
						else {
							t1 = i3; t2 = i2; t3 = i1;
						}
					}
					if (notfirst)
						fprintf(fd, ",\n");
					else
						notfirst = true;
					fprintf(fd, "  %d", pStream->m_texNo);
					i1 = i2; i2 = i3;
				}
			}
			else if (pStream->m_PrimitiveType == D3DPT_TRIANGLELIST) {
				for (unsigned int i = 0; i < pStream->GetFaceCount(); i++) {
					i1 = *pI++; i2 = *pI++; i3 = *pI++;
					if (pMesh->m_FlipFlag) {
						t1 = i1; t2 = i2; t3 = i3;
					}
					else {
						t1 = i3; t2 = i2; t3 = i1;
					}
					if (notfirst)
						fprintf(fd, ",\n");
					else
						notfirst = true;
					fprintf(fd, "  %d", pStream->m_texNo);
				}
			}
			fCnt += pStream->GetFaceCount();
			pStream = (CStream*)pStream->Next;
		}
		(pMesh->m_lpVB1)->Unlock();
		(pMesh->m_lpIB)->Unlock();
		vCnt += pMesh->m_NumVertices;
		pMesh = (CMesh*)pMesh->Next;
	}
	fprintf(fd, ";\n");
	outputMaterial(FPath, FName, fd);
	fprintf(fd, "}\n");
	return true;
}

bool CModel::outputMaterial(char *FPath, char *FName, FILE *fd) {
	// material出力
	int count = 0;
	CMaterial *pMaterial = (CMaterial*)m_Materials.Top();
	while (pMaterial != NULL)
	{
		fprintf(fd, " Material {\n");
		fprintf(fd, " 1.000;1.000;1.000;1.000;;\n");
		fprintf(fd, " 0.000;\n");
		fprintf(fd, " 1.000;1.000;1.000;;\n");
		fprintf(fd, " 0.000;0.000;0.000;;\n");
		fprintf(fd, " TextureFilename {\n");
//		fprintf(fd, " \"%s%02d.bmp\";\n", FName, count);
		char texName[256]; strcpy(texName, pMaterial->m_Name); Trim(texName);
		fprintf(fd, " \"%s.bmp\";\n", texName);
		fprintf(fd, "}\n");
		fprintf(fd, "}\n");
		//sprintf(texpath, "%s%02d.bmp", FPath, count);
		//D3DXSaveTextureToFile(texpath, D3DXIFF_BMP, pMaterial->m_pTexture, NULL);
		pMaterial = (CMaterial*)pMaterial->Next;
	}
	return true;
}

int CModel::countBone2Ver(int boneNo) {
	int	 boneCnt = 0;

	CMesh *pMesh = (CMesh*)m_Meshs.Top();
	while (pMesh != NULL) {
		CUSTOMVERTEX	*pV1, *pV2;
		(pMesh->m_lpVB1)->Lock(0, pMesh->m_VBSize, (void**)&pV1, D3DLOCK_DISCARD);
		(pMesh->m_lpVB2)->Lock(0, pMesh->m_VBSize, (void**)&pV2, D3DLOCK_DISCARD);
		for (unsigned int i = 0; i<pMesh->m_NumVertices; i++, pV1++, pV2++) {
			if (pV1->b1>0.f && pMesh->m_pBoneTbl[pV1->indx] == boneNo) {
				boneCnt++;
			} else if (pV2->b1>0.f && pMesh->m_pBoneTbl[pV2->indx] == boneNo) {
				boneCnt++;
			}
		}
		(pMesh->m_lpVB1)->Unlock();
		(pMesh->m_lpVB2)->Unlock();
		pMesh = (CMesh*)pMesh->Next;
	}
	return boneCnt;
}

bool CModel::outputBone2VerNo(FILE *fd,int boneNo) {
	int vCnt = 0, fCnt = 0;
	bool notfirst = false;
	CMesh *pMesh = (CMesh*)m_Meshs.Top();
	while (pMesh != NULL) {
		CUSTOMVERTEX	*pV1, *pV2;
		(pMesh->m_lpVB1)->Lock(0, pMesh->m_VBSize, (void**)&pV1, D3DLOCK_DISCARD);
		(pMesh->m_lpVB2)->Lock(0, pMesh->m_VBSize, (void**)&pV2, D3DLOCK_DISCARD);
		for (unsigned int i = 0; i<pMesh->m_NumVertices; i++, pV1++, pV2++) {
			if (pV1->b1>0.f && pMesh->m_pBoneTbl[pV1->indx] == boneNo) {
				if (notfirst)
					fprintf(fd, ",\n");
				else
					notfirst = true;
				fprintf(fd, "  %5d", i + vCnt);
					//fprintf(fd, "  %5d(X %4.4f Y %4.4f Z %4.4f W %1.6f idx %4d idxG %4d local %4d global %4d)", i + vCnt,
					//pV1->p.x, pV1->p.y, pV1->p.z, pV1->b1, pV1->indx, pMesh->m_pBoneTbl[pV1->indx], i, i + vCnt);
			} else if (pV2->b1>0.f && pMesh->m_pBoneTbl[pV2->indx] == boneNo) {
				if (notfirst)
					fprintf(fd, ",\n");
				else
					notfirst = true;
				fprintf(fd, "  %5d", i + vCnt);
					//fprintf(fd, "  %5d(X %4.4f Y %4.4f Z %4.4f W %1.6f idx %4d idxG %4d local %4d global %4d)", i + vCnt,
					//pV1->p.x, pV1->p.y, pV1->p.z, pV2->b1, pV2->indx, pMesh->m_pBoneTbl[pV2->indx], i, i + vCnt);
			}
		}
		(pMesh->m_lpVB1)->Unlock();
		(pMesh->m_lpVB2)->Unlock();
		vCnt += pMesh->m_NumVertices;
		pMesh = (CMesh*)pMesh->Next;
	}
	fprintf(fd, ";\n");
	return true;
}

bool CModel::outputBone2VerWeight(FILE *fd, int boneNo) {
	int vCnt = 0, fCnt = 0;
	bool notfirst = false;
	CMesh *pMesh = (CMesh*)m_Meshs.Top();
	while (pMesh != NULL) {
		CUSTOMVERTEX	*pV1, *pV2;
		(pMesh->m_lpVB1)->Lock(0, pMesh->m_VBSize, (void**)&pV1, D3DLOCK_DISCARD);
		(pMesh->m_lpVB2)->Lock(0, pMesh->m_VBSize, (void**)&pV2, D3DLOCK_DISCARD);
		for (unsigned int i = 0; i<pMesh->m_NumVertices; i++, pV1++, pV2++) {
			if (pV1->b1>0.f && pMesh->m_pBoneTbl[pV1->indx] == boneNo) {
				if (notfirst)
					fprintf(fd, ",\n");
				else
					notfirst = true;
				if (pV1->b1 < 0.0) pV1->b1 = 0.0;
				if (pV1->b1 > 1.0) pV1->b1 = 1.0;
				fprintf(fd, "  %1.6f", pV1->b1);
					//fprintf(fd, "  %1.6f(X %4.4f Y %4.4f Z %4.4f W %1.6f idx %4d idxG %4d local %4d global %4d)", pV1->b1,
					//pV1->p.x, pV1->p.y, pV1->p.z, pV1->b1, pV1->indx, pMesh->m_pBoneTbl[pV1->indx], i, i + vCnt);
			}
			else if (pV2->b1>0.f && pMesh->m_pBoneTbl[pV2->indx] == boneNo) {
				if (notfirst)
					fprintf(fd, ",\n");
				else
					notfirst = true;
				if (pV2->b1 < 0.0) pV2->b1 = 0.0;
				if (pV2->b1 > 1.0) pV2->b1 = 1.0;
				fprintf(fd, "  %1.6f", pV2->b1);
					//fprintf(fd, "  %1.6f(X %4.4f Y %4.4F Z %4.4f W %1.6f idx %4d idxG %4d local %4d global %4d)", pV2->b1,
					//pV1->p.x, pV1->p.y, pV1->p.z, pV2->b1, pV2->indx, pMesh->m_pBoneTbl[pV2->indx], i, i + vCnt);
			}
		}
		(pMesh->m_lpVB1)->Unlock();
		(pMesh->m_lpVB2)->Unlock();
		vCnt += pMesh->m_NumVertices;
		pMesh = (CMesh*)pMesh->Next;
	}
	fprintf(fd, ";\n");
	return true;
}

//======================================================================
//
//		MQOセーブ
//
//		データをMQOフォーマットで出力します
//======================================================================
bool CModel::saveMQO( char *FPath,char *FName ) 
{
	FILE *fd;
	char *ptr,strmsg[256],path[256];

	if( (ptr=strstr(FPath,".mqo")) ) *ptr = '\0';
	if( (ptr=strstr(FName,".mqo")) ) *ptr = '\0';
	sprintf(path,"%s.mqo",FPath);
	HANDLE hFile = CreateFile(path,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_ARCHIVE,NULL);
	if( hFile!=INVALID_HANDLE_VALUE ) {
		CloseHandle(hFile);
		wsprintf(strmsg, "ファイル %s　を上書きしますか？",path);
		if( MessageBox(NULL, strmsg, "セーブファイル上書き", MB_YESNO | MB_ICONQUESTION ) == IDNO ) return false;
	}
	if( (fd=fopen(path,"w"))==NULL ) return false;
	fprintf(fd,"Metasequoia Document\nFormat Text Ver 1.0\n\nScene {\n");
	fprintf(fd,"	pos 0.0000 0.0000 5000.0000\n");
	fprintf(fd,"	lookat 0.0000 0.0000 0.0000\n");
	fprintf(fd,"	head 0.0000\n");
	fprintf(fd,"	pich 0.0000\n");
	fprintf(fd,"	ortho 1\n");
	fprintf(fd,"	zoom2 2.0000\n");
	fprintf(fd,"	amb 0.250 0.250 0.250\n}\nMaterial ");
	outputTex(FPath,FName,fd);
	outputMesh(fd);
	fprintf(fd,"EOF");
	fclose(fd);
	return true;
}

//======================================================================
//
//		Texture出力
//
//		TextureデータをMQOフォーマットで出力します
//======================================================================
bool CModel::outputTex( char *FPath,char *FName,FILE *fd )
{
	char	texpath[256];
	int	count=0;

	if( fd==NULL ) return false;
	fprintf(fd,"%d {\n",m_Materials.Count);
	CMaterial *pMaterial = (CMaterial*)m_Materials.Top();
	while ( pMaterial != NULL )
	{
		fprintf(fd,"    \"texture%s%02d\" col(1.000 1.000 1.000 1.000)",FName,count);
		fprintf(fd," dif(1.000) amb(0.250) emi(0.250) spc(0.000) power(5.00) tex(\"%s%02d.bmp\")\n",FName,count);
		sprintf(texpath,"%s%02d.bmp",FPath,count);
		D3DXSaveTextureToFile(texpath, D3DXIFF_BMP, pMaterial->m_pTexture, NULL);
		pMaterial = (CMaterial*)pMaterial->Next;
		count++;
	}
	fprintf(fd,"}\n");
	return true;
}

//======================================================================
//
//		メッシュカウント
//
//		パーツ番号を持つメッシュデータをカウントする
//======================================================================
int CModel::countParts( int partsNo )
{
	int numParts=0;

	if( partsNo<1 || partsNo>6 ) return false;
	CMesh *pMesh = (CMesh*)m_Meshs.Top();
	while ( pMesh != NULL )
	{
		if( pMesh->m_PartsNo != partsNo ) {
			pMesh = (CMesh*)pMesh->Next;continue;
		}
		numParts++;
		pMesh = (CMesh*)pMesh->Next;
	}
	return numParts;
}

//======================================================================
//
//		メッシュ出力
//
//		メッシュデータをMQOフォーマットで出力します
//======================================================================
bool CModel::outputMesh( FILE *fd )
{
	char objname[6][32]={
		"sdef:Face","sdef:Head","sdef:Body", "sdef:Hand", "sdef:Leg", "sdef:Foot" };
	if( fd==NULL ) return false;
//	Face出力
	if( countParts( 1 )>0 ) {
		fprintf(fd,"Object \"%s\" {\n",objname[0]);
		fprintf(fd,"    visivle 15\n    locking 0\n    shading 1\n   facet 59.5\n    color 0.898 0.498 0.698\n   color_type 0\n");
		outputVerFace( fd, 1, 2 );
		fprintf(fd,"}\n");
	}
//	Head出力
	if( countParts( 2 )>0 ) {
		fprintf(fd,"Object \"%s\" {\n",objname[1]);
		fprintf(fd,"    visivle 15\n    locking 0\n    shading 1\n   facet 59.5\n    color 0.898 0.498 0.698\n   color_type 0\n");
		outputVerFace( fd, 2, 2 );
		fprintf(fd,"}\n");
	}
//	Body出力
	if( countParts( 3 )>0 ) {
		fprintf(fd,"Object \"%s\" {\n",objname[2]);
		fprintf(fd,"    visivle 15\n    locking 0\n    shading 1\n   facet 59.5\n    color 0.898 0.498 0.698\n   color_type 0\n");
		outputVerFace( fd, 3, 2 );
		fprintf(fd,"}\n");
	}
//	Hands出力
	if( countParts( 4 )>0 ) {
		// 右手出力
		fprintf(fd,"Object \"%sR\" {\n",objname[3]);
		fprintf(fd,"    visivle 15\n    locking 0\n    shading 1\n   facet 59.5\n    color 0.898 0.498 0.698\n   color_type 0\n");
		outputVerFace( fd, 4, 0 );
		fprintf(fd,"}\n");
		// 左手出力
		fprintf(fd,"Object \"%sL\" {\n",objname[3]);
		fprintf(fd,"    visivle 15\n    locking 0\n    shading 1\n   facet 59.5\n    color 0.898 0.498 0.698\n   color_type 0\n");
		outputVerFace( fd, 4, 1 );
		fprintf(fd,"}\n");
	}
//	Legs出力
	if( countParts( 5 )>0 ) {
		fprintf(fd,"Object \"%s\" {\n",objname[4]);
		fprintf(fd,"    visivle 15\n    locking 0\n    shading 1\n   facet 59.5\n    color 0.898 0.498 0.698\n   color_type 0\n");
		outputVerFace( fd, 5, 2 );
		fprintf(fd,"}\n");
	}
//	foots出力
	if( countParts( 6 )>0 ) {
		// 右手出力
		fprintf(fd,"Object \"%sR\" {\n",objname[5]);
		fprintf(fd,"    visivle 15\n    locking 0\n    shading 1\n   facet 59.5\n    color 0.898 0.498 0.698\n   color_type 0\n");
		outputVerFace( fd, 6, 0 );
		fprintf(fd,"}\n");
		// 左手出力
		fprintf(fd,"Object \"%sL\" {\n",objname[5]);
		fprintf(fd,"    visivle 15\n    locking 0\n    shading 1\n   facet 59.5\n    color 0.898 0.498 0.698\n   color_type 0\n");
		outputVerFace( fd, 6, 1 );
		fprintf(fd,"}\n");
	}
	return true;
}

//======================================================================
//
//		Vertex及びFace出力
//
//		Vertex及びFaceをMQOフォーマットで出力します
//======================================================================
bool CModel::outputVerFace( FILE *fd,int partsNo, int type )
{
	int verNum=0,faceNum=0,vCnt,fCnt;

	if( fd==NULL ) return false;
	if( partsNo<1 || partsNo>6 ) return false;
	CMesh *pMesh = (CMesh*)m_Meshs.Top();
	while ( pMesh != NULL )
	{
		if( pMesh->m_PartsNo != partsNo ) {
			pMesh = (CMesh*)pMesh->Next;continue;
		}
		if( type==0 && pMesh->m_FlipFlag==true ) {
			pMesh = (CMesh*)pMesh->Next;continue;
		}
		if( type==1 && pMesh->m_FlipFlag==false ) {
			pMesh = (CMesh*)pMesh->Next;continue;
		}
		verNum  += pMesh->m_NumVertices;
		faceNum += pMesh->m_NumFaces;
		pMesh = (CMesh*)pMesh->Next;
	}
	vCnt=0;
	fprintf(fd,"vertex %d {\n",verNum);
	pMesh = (CMesh*)m_Meshs.Top();
	while ( pMesh != NULL )
	{
		if( pMesh->m_PartsNo != partsNo ) {
			pMesh = (CMesh*)pMesh->Next;continue;
		}
		if( type==0 && pMesh->m_FlipFlag==true ) {
			pMesh = (CMesh*)pMesh->Next;continue;
		}
		if( type==1 &&  pMesh->m_FlipFlag==false ) {
			pMesh = (CMesh*)pMesh->Next;continue;
		}
		int			indx;
		D3DXVECTOR4	pos,p1,p2;
		D3DXMATRIX	mat;
		CUSTOMVERTEX	*pV1,*pV2;
		D3DXMatrixIdentity(&mat);
		(pMesh->m_lpVB1)->Lock( 0,pMesh->m_VBSize,(void**)&pV1,D3DLOCK_DISCARD );
		(pMesh->m_lpVB2)->Lock( 0,pMesh->m_VBSize,(void**)&pV2,D3DLOCK_DISCARD );
		for( unsigned int i=0 ; i<pMesh->m_NumVertices ; i++,pV1++,pV2++ ) {
			indx = pV1->indx;if( indx>pMesh->m_mBoneNum || indx <0 ) indx = 0; 
			mat = m_Bones[pMesh->m_pBoneTbl[indx]].m_mWorld;
			p1.x = pV1->p.x;p1.y = pV1->p.y;p1.z = pV1->p.z;p1.w = pV1->b1;
			D3DXVec4Transform(&p1,&p1,&mat);
			indx = pV2->indx;if( indx>pMesh->m_mBoneNum || indx <0 ) indx = 0; 
			mat = m_Bones[pMesh->m_pBoneTbl[indx]].m_mWorld;
			p2.x = pV2->p.x;p2.y = pV2->p.y;p2.z = pV2->p.z;p2.w = pV2->b1;
			D3DXVec4Transform(&p2,&p2,&mat);
			//pos=(p1+p2)*500.f;
			pos=(p1+p2)*100.f;
			fprintf(fd, "        %4.4f %4.4f %4.4f\n", pos.x, pos.y, pos.z);
		}
		(pMesh->m_lpVB2)->Unlock();
		(pMesh->m_lpVB1)->Unlock();
		vCnt+= pMesh->m_NumVertices;
		pMesh = (CMesh*)pMesh->Next;
	}
	fprintf(fd,"\t}\n");
	fCnt = vCnt=0;
	fprintf(fd,"face %d {\n",faceNum);
	pMesh = (CMesh*)m_Meshs.Top();
	while ( pMesh != NULL )
	{
		if( pMesh->m_PartsNo != partsNo ) {
			pMesh = (CMesh*)pMesh->Next;continue;
		}
		if( type==0 && pMesh->m_FlipFlag==true ) {
			pMesh = (CMesh*)pMesh->Next;continue;
		}
		if( type==1 &&  pMesh->m_FlipFlag==false ) {
			pMesh = (CMesh*)pMesh->Next;continue;
		}
		WORD			*pI,*pIndex;
		int				i1,i2,i3,t1,t2,t3;
		CUSTOMVERTEX	*pV;
		(pMesh->m_lpIB)->Lock( 0,pMesh->m_IBSize,(void**)&pIndex,D3DLOCK_DISCARD );
		(pMesh->m_lpVB1)->Lock( 0,pMesh->m_VBSize,(void**)&pV,D3DLOCK_DISCARD );
		CStream *pStream = (CStream*)pMesh->m_Streams.Top();
		while( pStream!=NULL ) {
			pI = pIndex + pStream->GetIndexStart();
			if( pStream->m_PrimitiveType==D3DPT_TRIANGLESTRIP ) {
				i1=*pI++;i2=*pI++;
				for( unsigned int i=0 ; i<pStream->GetFaceCount() ; i++ ) {
					i3 = *pI++;
					if( i%2 ) {
						if( pMesh->m_FlipFlag ) {
							t1 = i3; t2 = i2; t3 = i1;
						} else {
							t1 = i1; t2 = i2; t3 = i3;
						}
					} else {
						if( pMesh->m_FlipFlag ) {
							t1 = i1; t2 = i2; t3 = i3;
						} else {
							t1 = i3; t2 = i2; t3 = i1;
						}
					}
					fprintf(fd,"\t\t3 V(%3d %3d %3d) M(%2d) UV(%1.5f %1.5f %1.5f %1.5f %1.5f %1.5f)\n",
					t1+vCnt,t2+vCnt,t3+vCnt,pStream->m_texNo,
					(pV+t1)->u,(pV+t1)->v,(pV+t2)->u,
					(pV+t2)->v,(pV+t3)->u,(pV+t3)->v);			
					i1 = i2; i2 = i3;
				}
			} else if( pStream->m_PrimitiveType==D3DPT_TRIANGLELIST ) {
				for( unsigned int i=0 ; i<pStream->GetFaceCount() ; i++ ) {
					i1=*pI++;i2=*pI++;i3=*pI++;
					if( pMesh->m_FlipFlag ) {
						t1 = i1; t2 = i2; t3 = i3;
					} else {
						t1 = i3; t2 = i2; t3 = i1;
					}
					fprintf(fd,"\t\t3 V(%3d %3d %3d) M(%2d) UV(%1.5f %1.5f %1.5f %1.5f %1.5f %1.5f)\n",
					t1+vCnt,t2+vCnt,t3+vCnt,pStream->m_texNo,
					(pV+t1)->u,(pV+t1)->v,(pV+t2)->u,
					(pV+t2)->v,(pV+t3)->u,(pV+t3)->v);			
				}
			}
			fCnt+= pStream->GetFaceCount();
			pStream = (CStream*)pStream->Next;
		} 
		(pMesh->m_lpVB1)->Unlock();
		(pMesh->m_lpIB)->Unlock();
		vCnt+= pMesh->m_NumVertices;
		//fCnt += pMesh->m_NumFaces;
		pMesh = (CMesh*)pMesh->Next;
	}
	fprintf(fd,"\t}\n");
	return true;
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
	m_mRace				= 0;
	m_mFace				= 0;
	m_mHead				= 0;
	m_mBody				= 0;
	m_mHand				= 0;
	m_mLegs				= 0;
	m_mFoot				= 0;
	m_mRightWeapon		= 0;
	m_mLeftWeapon		= 0;
	m_mRemoteWeapon		= 0;
	m_MotionLevel		= 0;
	m_MotionOffset		= 0;
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
	    pdat = new char[dwSize];
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
	delete pdat;
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
	    pdat = new char[dwSize];
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
	delete pdat;
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
	if( (pFile = new char[dwSize]) == NULL ) return false;
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
	    pdat = new char[dwSize];
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
	delete pdat;
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




