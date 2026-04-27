// INCLUDE
#define _CRT_SECURE_NO_WARNINGS
#include <vector>
#include <sstream>
#include <iostream>
#include <string>
#include <stdio.h>
#include "Dx.h"
#include "Render.h"
#include "Model.h"
//Function
DWORD _GetFileNameFromIDsub(DWORD dwV,DWORD dwID);
BOOL GetFileNameFromDno(LPSTR filename,DWORD dwID);
BOOL GetFileNameFromDir(LPSTR filename,char *DataName );
DWORD	ConvertStr2Dno( char* DataName );
// DEFINE
#define SAFE_RELEASE(p)		if ( (p) != NULL ) { (p)->Release(); (p) = NULL; }
#define SAFE_DELETES(p)		if ( (p) != NULL ) { delete [] (p); (p) = NULL; }
#define SAFE_DELETE(p)		if ( (p) != NULL ) { delete (p); (p) = NULL; }
#define PAI					(3.1415926535897932384626433832795f)
#define PAI2				(3.1415926535897932384626433832795f*2.0f)

extern const D3DXMATRIX matrixMirrorX(-1.0f,0,0,0,  0, 1.0f,0,0,  0,0, 1.0f,0,  0,0,0,1.0f);
extern const D3DXMATRIX matrixMirrorY( 1.0f,0,0,0,  0,-1.0f,0,0,  0,0, 1.0f,0,  0,0,0,1.0f);
extern const D3DXMATRIX matrixMirrorZ( 1.0f,0,0,0,  0, 1.0f,0,0,  0,0,-1.0f,0,  0,0,0,1.0f);
// グローバル
extern	CPC			*pPC;
extern	CNPC		*pNPC;
extern	bool		g_mPCFlag;
		int			g_mPCMotion = 0; // 0:default 1:attack 2:emotion 3:WS
extern	BOOL		g_mIsUseSoftware;
extern	LIGHTDATA	g_mLight,g_mLightbase;
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
	for (const char *p = string; NULL != (p = strstr(p, pattern)); ++p)
	{
		last = p;
		if ('\0' == *p)
			return (char *)last;
	}
	return (char *)last;
}//strrstr

char* // 文字列へのポインタ
strcpynosp
(
	char* string1, // コピー先文字列
	char* string2 // コピー元文字列
)
{
	char* p = string1;
	// 文字列終端に達するまでコピーを繰り返す。
	for (int i = 0; i < strlen(string2); i++)
	{
		if (string2[i] != ' ')
		{
			*p++ = string2[i];
		}
	}
	if( *p != '\0' ) *p='\0';
	return (char*)string1;
}//strcpynosp

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

//------------------------------------------------------------------------------------------//																						//
//									CData													//																				//
//------------------------------------------------------------------------------------------//
//======================================================================
//		コンストラクタ
//======================================================================
CData::CData()
{
	m_Time					= 0;
	m_pInputLayout			= NULL;
	m_VertexSize			= 0;
	m_pVertexShader			= NULL;
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

bool convert_path(char* src, const char* base) {
	// 検索の目印となる文字列（ゲームのデータフォルダ名）
	static const char* search_term = "ROM";
	// 新しいベースディレクトリ
	static const char* new_base = base;
	// "ROM\" が出現する位置を検索
	const char* relative_path = strstr(src, search_term);
	if (relative_path != NULL) {
		// 新しいベースパスと相対パスを結合
		// snprintfを使うことでバッファオーバーフローを防ぎつつ結合可能です
		snprintf(src, strlen(src), "%s%s", new_base, relative_path);
		return true;
	}
	else {
		return false;
	}
}


HRESULT CData::LoadTextureFromFile( char *FileName  )
{
	HRESULT hr							= S_OK;

	//====================================================
	// ファイルをメモリに取り込む
	//====================================================
	char *pdat=NULL,path[512];
	int dwSize;
	unsigned long	cnt;

	strcpy(path, FileName);
	if (strlen(g_texPath) > 0) {
		convert_path(path, g_texPath);
	}
	HANDLE	hFile = CreateFile(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_ARCHIVE, NULL);
	if( hFile!=INVALID_HANDLE_VALUE ){
		dwSize = GetFileSize(hFile,NULL);
	    pdat = new char[dwSize]();
	    ReadFile(hFile,pdat,dwSize,&cnt,NULL);
	    CloseHandle(hFile);
		hr = 0;
	}
	else {
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
			UINT xx = *(UINT*)(pdat+pos+33+4);
			UINT yy = *(UINT*)(pdat+pos+33+8);
			DWORD fourCC = *(DWORD*)(pdat+pos+33+0x28);

			D3D11_TEXTURE2D_DESC texDesc = {};
			texDesc.Width            = xx;
			texDesc.Height           = yy;
			texDesc.MipLevels        = 1;
			texDesc.ArraySize        = 1;
			texDesc.SampleDesc.Count = 1;
			texDesc.Usage            = D3D11_USAGE_DEFAULT;
			texDesc.BindFlags        = D3D11_BIND_SHADER_RESOURCE;

			D3D11_SUBRESOURCE_DATA initData = {};
			BYTE *pTempBuf = nullptr;

			if( fourCC == 'DXT3' ){
				texDesc.Format           = DXGI_FORMAT_BC2_UNORM;
				initData.pSysMem         = pdat+pos+33+0x28+12;
				initData.SysMemPitch     = ((xx+3)/4) * 16;
			} else if( fourCC == 'DXT1' ){
				texDesc.Format           = DXGI_FORMAT_BC1_UNORM;
				initData.pSysMem         = pdat+pos+33+0x28+12;
				initData.SysMemPitch     = ((xx+3)/4) * 8;
			} else {
				texDesc.Format           = DXGI_FORMAT_B8G8R8A8_UNORM;
				pTempBuf = new BYTE[xx * yy * 4];
				BYTE  *idx = (BYTE* )(pdat+pos+33+0x28+0x400);
				DWORD *pal = (DWORD*)(pdat+pos+33+0x28);
				DWORD *pp  = (DWORD*)pTempBuf;
				for( DWORD jy=0; jy<yy; jy++ )
					for( DWORD jx=0; jx<xx; jx++ )
						pp[(yy-jy-1)*xx+jx] = pal[idx[jy*xx+jx]];
				initData.pSysMem         = pTempBuf;
				initData.SysMemPitch     = xx * 4;
			}

			ID3D11Texture2D *pTex2D = nullptr;
			hr = GetDevice()->CreateTexture2D( &texDesc, &initData, &pTex2D );
			delete[] pTempBuf;
			if( FAILED(hr) ) break;

			ID3D11ShaderResourceView *pSRV = nullptr;
			hr = GetDevice()->CreateShaderResourceView( pTex2D, nullptr, &pSRV );
			pTex2D->Release();
			if( FAILED(hr) ) break;

			pMaterial->SetTexture( pSRV );
			pSRV->Release();
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
