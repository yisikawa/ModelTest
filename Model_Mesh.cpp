#include "Dx.h"
#include "Model.h"

// DEFINE
#define SAFE_RELEASE(p)		if ( (p) != NULL ) { (p)->Release(); (p) = NULL; }
#define SAFE_DELETES(p)		if ( (p) != NULL ) { delete [] (p); (p) = NULL; }
#define SAFE_DELETE(p)		if ( (p) != NULL ) { delete (p); (p) = NULL; }

// Model_Common.cpp で定義されているユーティリティ関数の宣言
extern int Trim(char *s);
extern bool convert_path(char *path, const char *target);
extern void convert_texture_path(char *path);
extern HRESULT CreateVB(LPDIRECT3DVERTEXBUFFER9 *lpVB, DWORD size, DWORD Usage, DWORD fvf);
extern HRESULT CreateIB(LPDIRECT3DINDEXBUFFER9 *lpIB, DWORD size, DWORD Usage);

// 他のファイルで定義されているグローバル変数の宣言
extern bool g_mPCFlag;

// Model_Common.cpp で定義されている定数・変数の宣言
extern const D3DXMATRIX matrixMirrorX;
extern const D3DXMATRIX matrixMirrorY;
extern const D3DXMATRIX matrixMirrorZ;
extern int g_mBLCnvTbl[9][10];

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
	m_NumElement	= 0;
	m_Level			= 0;
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
	memset(&m_Name, 0x00, sizeof(m_Name));
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
	memset(&m_Name, 0x00, sizeof(m_Name));
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
	m_DispCheck				= 0;
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

	m_pBoneTbl = new WORD[BoneTblNum]();
	m_pBoneTblNum = new DWORD[BoneTblNum]();
//	memset(m_pBoneTbl,0,BoneTblNum*sizeof(WORD));
//	memset(m_pBoneTblNum,0,BoneTblNum*sizeof(DWORD));
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
				if( (WORD)bidx == m_pBoneTbl[jj] ) {
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

HRESULT CMesh::LoadMesh
( char *pFile, CModel *pModel, unsigned long FVF,int Flip  )
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
	pUVidx		= new int[NumVertices*10](); //初期化（）
	pUTemp		= new float[NumVertices*10]();
	pVTemp		= new float[NumVertices*10]();
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
			strncpy(TexName, pPoly + addpos + 2, 16); TexName[16] = '\0';
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
