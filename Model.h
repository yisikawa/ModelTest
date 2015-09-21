
#pragma once

//======================================================================
// INCLUDE
//======================================================================
#include <d3d9.h>
#include <d3dx9.h>


//======================================================================
// GLOBAL
//======================================================================

//======================================================================
// TYPE DEFINE
//======================================================================
class CMaterial;
class CMesh;
class CBone;
class CMotionFrame;
class CModel;
#pragma pack(push,2)

typedef struct{
  char				name[16];			//      名前
  BYTE		Ver;						//0x00
  BYTE		nazo;						//0x01
  WORD				Type;				//0x02 &7f==0モデル 1=クロス
  WORD				Flip;				//0x04 0==OFF  ON
  DWORD				OffsetPoly;			//0x06
  WORD				PolyNum;			//0x0A  //PolySuu
  DWORD				OffsetBoneTbl;		//0x0C
  WORD				BoneTblNum;			//0x10
  DWORD				OffsetWeight;		//0x12
  WORD				WeightNum;			//0x16
  DWORD				OffsetBone;			//0x18
  WORD				BoneNum;			//0x1C
  DWORD				OffsetVertex;		//0x1E  
  WORD				VertexNum;			//0x22
  DWORD				OffsetPolyLoad;		//0x24
  WORD				PolyLoadNum;		//0x28
  WORD				PolyLodVtx0Num;		//0x2A
  WORD				PolyLodVtx1Num;		//0x2C
  DWORD				OffsetPolyLod2;		//0x2E
  WORD				PolyLod2Num;		//0x32
} DAT2AHeader;

typedef struct
{
	short i1,i2,i3;
	float u1,v1;
	float u2,v2;
	float u3,v3;
} TEXLIST;
typedef struct
{
	short i;
	float u,v;
} TEXLIST2;

typedef struct
{
	float x,y,z;     //座標
	float hx,hy,hz;  //法線ベクトル
} MODELVERTEX1;

typedef struct  //これで確定
{
	float x1,x2,y1,y2,z1,z2;
	float w1,w2;
	float hx1,hx2;  //法線ベクトル
	float hy1,hy2;  //法線ベクトル
	float hz1,hz2;  //法線ベクトル
} MODELVERTEX2;

typedef struct
{
	float x,y,z;     //座標
} CLOTHVERTEX1;

typedef struct
{
	float x1,x2;
	float y1,y2;
	float z1,z2;
	float w1,w2;
} CLOTHVERTEX2;


typedef struct 
{
	D3DXVECTOR3 p;       // vertex position
	float       b1;
	DWORD		indx;
	D3DXVECTOR3 n;       // vertex normal
	float		u,v;
} CUSTOMVERTEX ;

typedef struct
{
	D3DXVECTOR3 p;       // vertex position
	float       b1;
	unsigned char idx[4];
	D3DXVECTOR3 n;       // vertex normal
	float		u, v;
} CUSTOMVERTEX2;

#pragma pack(pop)

#define FVF_BLENDVERTEX		(D3DFVF_XYZB2 | D3DFVF_LASTBETA_UBYTE4 | D3DFVF_NORMAL | D3DFVF_TEX1)

typedef struct
{
  int		no;
  int		idx_qtx,idx_qty,idx_qtz,idx_qtw;
  float		qtx,qty,qtz,qtw;
  int		idx_tx,idx_ty,idx_tz;
  float		tx,ty,tz;
  int		idx_sx,idx_sy,idx_sz;
  float		sx,sy,sz;
} DAT2B;

//-----------------------------------------------------
// 回転キー
//-----------------------------------------------------
typedef struct {
	unsigned long	Time;
	D3DXQUATERNION	Quat;
} KEYROTATION;

//-----------------------------------------------------
// 拡大縮小キー
//-----------------------------------------------------
typedef struct {
	unsigned long	Time;
	D3DXVECTOR3		Scale;	
} KEYSCALING;

//-----------------------------------------------------
// 移動キー
//-----------------------------------------------------
typedef struct {
	unsigned long	Time;
	D3DXVECTOR3		Pos;	
} KEYTRANSLATION;

//-----------------------------------------------------
// 行列キー
//-----------------------------------------------------
typedef struct {
	unsigned long	Time;
	D3DXMATRIX		Matrix;
} KEYMATRIX;

//======================================================================
// リスト用基底クラス
//======================================================================
typedef class CListBase
{
	friend class CList;

protected:
	CList			*pParentList;
	long			ReferenceCount;

public:
	CListBase		*Prev;
	CListBase		*Next;

	CListBase();
	virtual ~CListBase();

	virtual long Release( void );
	virtual void AddRef( void );
}
CListBase, *LPCListBase;


//==========================================================================
// リスト管理クラス
//==========================================================================
typedef class CList
{
protected:
	LPCListBase		ListTop;
	LPCListBase		ListEnd;

public:
	unsigned long	Count;
	CList();
	~CList();

	void Init( void );
	LPCListBase Top( void );
	LPCListBase End( void );
	void InsertTop( LPCListBase t );
	void InsertEnd( LPCListBase t );
	void InsertPrev( LPCListBase pTarget, LPCListBase pIt );
	void InsertNext( LPCListBase pTarget, LPCListBase pIt );
	void Erase( LPCListBase t );
	void Release( void );
	long Size( void );
	LPCListBase Data( long no );
}
CList, *LPCList;

//======================================================================
// ストリームタイプクラス
//======================================================================
typedef class CStream : public CListBase
{
protected:
	unsigned long		m_IndexStart;
	unsigned long		m_FaceCount;
	short				m_DispLevel;

public:
	D3DPRIMITIVETYPE	m_PrimitiveType;
	int					m_texNo,m_NumElement,m_Level;
	int					m_MinIdx,m_MaxIdx;
	CMaterial			*m_pMaterial;
	CStream();
	virtual ~CStream();

	virtual	void SetDispLevel( short DispLevel ) { m_DispLevel = DispLevel; }
	virtual	int	 GetDispLevel( void ) { return (int)m_DispLevel; }
	virtual void SetData( D3DPRIMITIVETYPE PrimitiveType,unsigned long index_start, unsigned long face_count );
	virtual D3DPRIMITIVETYPE GetPrimitiveType( void );	
	virtual unsigned long GetIndexStart( void );
	virtual unsigned long GetFaceCount( void );
}
CStream, LPCStream;

//======================================================================
// マテリアルクラス
//======================================================================
typedef class CMaterial : public CListBase
{
protected:
//	IDirect3DTexture9	*m_pTexture;
	D3DMATERIAL9		m_Material;
	unsigned long		m_IndexStart;
	unsigned long		m_FaceCount;

public:
	char				m_Name[18];
	IDirect3DTexture9	*m_pTexture;
	CMaterial();
	virtual ~CMaterial();
	virtual void SetName( char *pName ) { strcpy(m_Name,pName); }
	virtual char* GetName(void) { return m_Name; }
	virtual void SetData( unsigned long index_start, unsigned long face_count );
	virtual void SetMaterial( D3DMATERIAL9 *pMat );
	virtual void SetTexture( IDirect3DTexture9 *pTex );
	virtual D3DMATERIAL9 *GetMaterial( void );
	virtual IDirect3DTexture9 *GetTexture( void );
	virtual unsigned long GetIndexStart( void );
	virtual unsigned long GetFaceCount( void );
}
CMaterial, LPCMaterial;

//======================================================================
// メッシュクラス
//======================================================================
typedef class CMesh : public CListBase
{
	friend class CModel;

protected:
	bool					m_FlipFlag;
	short					m_PartsNo;
	short					m_DispLevel;
	short					m_DispCheck;
//	CBone					*m_pParent;
	ID3DXBuffer				*m_pBoneTransforms;		// add


	WORD					m_mBoneNum;
	WORD					*m_pBoneTbl;
	DWORD					*m_pBoneTblNum;

public:
	unsigned long			m_NumVertices,m_NumFaces,m_NumIndex,m_VBSize,m_IBSize,m_FVF;
	LPDIRECT3DVERTEXBUFFER9 m_lpVB1,m_lpVB2;
	LPDIRECT3DINDEXBUFFER9	m_lpIB;
	int						m_Stnum;
	CList					m_Streams;
	char					m_Name[18];
	CMesh();
	virtual ~CMesh();

	virtual	void SetFlipFlag( bool FlipFlag ) { m_FlipFlag = FlipFlag; }
	virtual bool GetFlipFlag( void ) { return m_FlipFlag; }
	virtual void SetPartsNo( short PartsNo ) { m_PartsNo = PartsNo; }
	virtual	int	 GetPartsNo( void ) { return (int)m_PartsNo; }
	virtual void SetDispLevel( short DispLevel ) { m_DispLevel = DispLevel; }
	virtual	int	 GetDispLevel( void ) { return (int)m_DispLevel; }
	virtual void SetDispCheck( short DispCheck ) { m_DispCheck = DispCheck; }
	virtual	int	 GetDispCheck( void ) { return (int)m_DispCheck; }
	virtual	void Conv1VertexC1(CUSTOMVERTEX *pV,int No1,int No2, WORD Type,
				  WORD *pBone, CLOTHVERTEX1 *pVertexC,float u,float v,int Flip);
	virtual	void Conv1VertexC2(CUSTOMVERTEX *pV,int No1, int No2, WORD Type,
				  WORD *pBone, CLOTHVERTEX2 *pVertexC2,float u,float v,int Flip);
	virtual void Conv1Vertex1(CUSTOMVERTEX *pV,int No1,int No2, WORD Type,
				  WORD *pBone, MODELVERTEX1 *pVertex, float u, float v, int Flip);
	virtual	void Conv1Vertex2(CUSTOMVERTEX *pV,int No1,int No2, WORD Type,
				  WORD *pBone, MODELVERTEX2 *pVertex2, float u, float v, int Flip);
	virtual	void Conv2VertexC1(CUSTOMVERTEX *pV,int No1,int No2, WORD Type,
				  WORD *pBone, CLOTHVERTEX1 *pVertexC,float u,float v,int Flip);
	virtual	void Conv2VertexC2(CUSTOMVERTEX *pV,int No1, int No2, WORD Type,
				  WORD *pBone, CLOTHVERTEX2 *pVertexC2,float u,float v,int Flip);
	virtual	void Conv2Vertex1(CUSTOMVERTEX *pV,int No1,int No2, WORD Type,
				  WORD *pBone, MODELVERTEX1 *pVertex, float u, float v, int Flip);
	virtual	void Conv2Vertex2(CUSTOMVERTEX *pV,int No1,int No2, WORD Type,
				  WORD *pBone, MODELVERTEX2 *pVertex2, float u, float v, int Flip);
	virtual	void SetMeshBone( int Num1, int Num2, WORD Type, WORD BoneTblNum, WORD *pBone, WORD *pBoneTbl, int Flip );
	virtual HRESULT LoadMesh( char *pFile, CModel *pModel, unsigned long FVF, int Flip );
	virtual bool prtVertex(FILE *fd);
	virtual bool prtFace(FILE *fd);
	virtual bool prtNormal(FILE *fd);
	virtual bool prtTexCoord(FILE *fd);
	virtual bool prtMaterialList(FILE *fd);
	virtual bool prtMaterial(char *FPath, char *FName, FILE *fd);
	virtual int  cntNumFace(void);
	virtual int  cntNumMaterial(void);
	virtual int  cntBone2Ver(int boneNo);
	virtual bool prtBone2VerNo(FILE *fd, int boneNo);
	virtual bool prtBone2VerWeight(FILE *fd, int boneNo);
}
CMesh, *LPCMesh;


//======================================================================
// モーション要素クラス
//======================================================================
typedef class CMotionElement : public CListBase
{
	friend class CModel;

protected:
	// モーションデータ
	int				m_BoneNo;
	unsigned long	m_RotationKeyNum;
	unsigned long	m_ScalingKeyNum;
	unsigned long	m_TranslateKeyNum;
	unsigned long	m_MatrixKeyNum;
	KEYROTATION*	m_pRotationKeys;
	KEYSCALING*		m_pScalingKeys;
	KEYTRANSLATION*	m_pTranslateKeys;
	KEYMATRIX*		m_pMatrixKeys;

public:
	CMotionElement();
	virtual ~CMotionElement();
	virtual	void InitData( void );
	virtual void LoadMotion( int No, int Keys,
					float Times, DAT2B *bDat, float *fDat);
	virtual void AddMotion( int No, int Keys,
					float Times, DAT2B *bDat, float *fDat);
	virtual	float		GetFinalTime( void );
	virtual D3DXMATRIX *GetMotionMatrix(float time, D3DXMATRIX *iMatrix);
	virtual D3DXMATRIX *GetMotionMatrix2(float time);
	virtual D3DXMATRIX *GetAnimationMatrix(int keyNo, D3DXMATRIX *iMatrix);
	virtual D3DXMATRIX *GetMotionScaling(float time);
	virtual D3DXMATRIX *GetMotionTranslate( float time );
	virtual D3DXMATRIX *GetMotionRotation( float time );
	virtual int			SetBoneNo(int BoneNo);
	virtual int			GetBoneNo(void);
	virtual	bool		CopyMotionElement( CMotionElement *pAnimElement);
	virtual	bool		AddMotionElement( CMotionElement *pAnimElement);
}
CMotionElement, LPCMotionElement;

//======================================================================
// モーションフレームクラス
//======================================================================
typedef class CMotionFrame : public CListBase
{
	friend class CModel;

protected:
	//char			m_Name[6];
public:
	char			m_Name[6];
	CList			m_MotionEList;
	CMotionFrame();
	virtual ~CMotionFrame();
	virtual int			SetMotionName( char *MotionName );
	virtual char*		GetMotionName( void );
	virtual	void		LoadMotionSet( char *pFile );
	virtual	void		InitData( void );
}
CMotionFrame, LPCMotionFrame;

//======================================================================
// ボーンクラス
//======================================================================
typedef class CBone : public CListBase
{
	friend class CModel;

protected:
	CBone					*m_pParent;
	int						m_mParent;
	char					m_Name[18];
	CMotionElement			*m_pMotion;

	virtual void LoadTransformMatrix( char *pFile );

public:
	bool					m_mTerm;
	D3DXMATRIX				m_mWorld,m_mWorldX;
	D3DXMATRIX				m_mTransform;
	D3DXMATRIX				m_mInvTrans;
	D3DXMATRIX				m_mInvTransX;
	D3DXQUATERNION			vecQ;
	D3DXVECTOR3				vecT, vecR, vecS;
	CBone();
	virtual ~CBone();
	virtual	void InitData( void );
	virtual char *GetName( void );
	virtual D3DXMATRIX *GetMatrix( void );
	virtual char *LoadBone( char *pFile );
	virtual void SetName( char *name );
	virtual bool CheckName( char *name );
	virtual void SetMotion( CMotionElement *pMotion );
}
CBone, *LPCBone;



//======================================================================
// ベースデータクラス
//======================================================================
typedef class CData : public CListBase
{
protected:
	float						m_Time;
	IDirect3DVertexDeclaration9	*m_VertexFormat;
	unsigned long				m_VertexSize;
	IDirect3DVertexShader9		*m_hVertexShader,*m_rVertexShader;

	D3DXMATRIX					m_mRootTransform;

public:
	CList						m_Materials;
	CData();
	virtual ~CData();
	virtual	void	AddTime( float addTime ) { m_Time += addTime; }
	virtual	float	GetTime() { return m_Time; }
	virtual	void	SetTime( float Time ) { m_Time = Time; }
	virtual HRESULT LoadTextureFromFile( char *filename );

	virtual void	InitTransform( void );
	virtual void	GetMatrix( D3DXMATRIX &mat );
	virtual void	SetMatrix( D3DXMATRIX &mat );
	virtual void	MulMatrix( D3DXMATRIX &mat );

	virtual void	Translation( float px, float py, float pz );
	virtual void	Scaling( float sx, float sy, float sz );
	virtual void	RotationX( float rot );
	virtual void	RotationY( float rot );
	virtual void	RotationZ( float rot );
	virtual void	RotationDir( float rot );
	virtual void	RotationCenter( D3DXVECTOR3 pos, float rot );
	virtual void	MirrorX( void );
	virtual void	MirrorY( void );
	virtual void	MirrorZ( void );

	virtual bool	LookAt( D3DXVECTOR3 &at, D3DXVECTOR3 &up );

	virtual void	GetWorldPosition( D3DXVECTOR3 &pos );
	virtual void	GetXAxis( D3DXVECTOR3 &pos );
	virtual void	GetYAxis( D3DXVECTOR3 &pos );
	virtual void	GetZAxis( D3DXVECTOR3 &pos );
	virtual bool	GetScreenPosition( D3DXVECTOR4 &pos );
}
CData, *LPCData;

typedef struct
{
  char id[16];
  float fTransX,fTransY,fTransZ;
  float fRotX,fRotY,fRotZ;
  float fScaleX,fScaleY,fScaleZ;
  float fa,fb,fc,fd;
  long  fe,ff,fg,fh,fi,fj,fk,fl;
} TEMPOBJINFO;

typedef struct {
	D3DXVECTOR3		mPos[8];	//　不明
	DWORD			mInf[8];	// 不明
} WALLINFO;


//======================================================================
// モデルクラス
//======================================================================
typedef class CModel : public CData
{
protected:
	char					m_MotionName[64];
	CList					m_Meshs;

public:
	CList					m_motions;
	int						m_nMesh, m_nBone;
	CBone					m_Bones[128];
	CMotionElement			m_MotionArray[128];
	CModel();
	virtual ~CModel();
	virtual int		GetBoneParentNo( int bnum ) { if( bnum>=0 && bnum<m_nBone ) return m_Bones[bnum].m_mParent;return -1; }
	virtual void	ClearMotion( void );
	virtual void	SetMotionName( char *MotionName ) { strcpy(m_MotionName,MotionName); }
	virtual char*	GetMotionName( void ) { return m_MotionName; }
	virtual void	InitPartsData(void);
	virtual void	InitMotionData(void);
	virtual HRESULT LoadBoneFromFile( char *filename );
	virtual HRESULT LoadMeshFromFile( char *filename, unsigned long FVF, int PartsNo );
	virtual	int		LoadInfoFromFile( char *FileName, int Offset );
	virtual HRESULT LoadMotionFromFile( char *filename );
	virtual bool	SetMotion3FromList( int Type, char *MotionName );
	virtual bool	SetMotion4FromList( int Type, char *MotionName );
	virtual void	InitTransform( void );
	virtual	float	MaxMotionTime( void );
	virtual void	InitialTransform(void);
	virtual void	DynamicTransform(void);
	virtual unsigned long Rendering(void);
	virtual void	BoneRendering(void);
	virtual void	RotateZero( D3DXMATRIX *mat );
	virtual bool	saveMQO(char *FPath, char *FName);
	virtual int		totalVertex(void);
	virtual int		totalFace(void);
	virtual int		countBone2Ver(int boneNo);
	virtual bool	outputBone2VerNo(FILE *fd, int boneNo);
	virtual bool	outputBone2VerWeight(FILE *fd, int boneNo);
	virtual bool	saveX(char *FPath, char *FName);
	virtual bool	outputConvMatrix(FILE *fd, D3DXMATRIX *iMatrix);
	virtual bool	outputMatrix(FILE *fd, D3DXMATRIX *iMatrix);
	virtual bool	outputMatrixSub(FILE *fd, D3DXMATRIX *iMatrix);
	virtual bool	outputMeshX(char *FPath, char *FName, FILE *fd);
	virtual bool	outputMultiMeshX(char *FPath, char *Fname, FILE *fd);
	virtual bool	outputVertex(FILE *fd);
	virtual bool	outputFace(FILE *fd);
	virtual bool	outputNormal(FILE *fd);
	virtual bool	outputNormalFace(FILE *fd);
	virtual bool	outputTexCoord(FILE *fd);
	virtual bool	outputVerDup(FILE *fd);
	virtual bool	outputMaterialList(char *FPath, char *FName, FILE *fd);
	virtual bool	outputMaterial(char *FPath, char *FName, FILE *fd);
	virtual bool	outputSkinX(FILE *fd);
	virtual bool	outputSkinWeights(FILE *fd, int boneNo);
	virtual bool    outputFrameX(FILE *fd);
	virtual int     outputFrameXsub(FILE *fd,int boneNo); 
	virtual bool    outputAnimationSet(FILE *fd,char *nameMotion);
	virtual bool    outputAnimationX(FILE *fd,int boneNo);
	virtual bool	outputTex(char *FPath, char *FName, FILE *fd);
	virtual bool	outputMesh( FILE *fd);
	virtual bool    outputVerFace( FILE *fd, int partsNo, int type );
	virtual int		countParts( int partsNo );
	virtual bool    ConvertMesh(void);
} CModel, *LPCModel;

//======================================================================
// PCクラス
//======================================================================
typedef class CPC : public CModel
{
protected:
	int			m_mInfoBody;			// Info 0x19番目
	int			m_mInfoRWeapon;			// Info 0x13番目
	int			m_mInfoRWeapon2;		// Info 0x16番目
	int			m_mInfoLWeapon;			// Info0x15番目
	int			m_mInfoLWeapon2;		// Info 0x13番目
	int			m_mType;	// 種族タイプ 0 から1
	int			m_mRace;	// 種族
	int			m_mFace;	// フェイス
	int			m_mHead;
	int			m_mBody;
	int			m_mHand;
	int			m_mLegs;
	int			m_mFoot;
	int			m_mRightWeapon;
	int			m_mLeftWeapon;
	int			m_mRemoteWeapon;

public:
	int					m_MotionOffset;
	int					m_MotionLevel;
	CPC();
	virtual ~CPC();
	virtual	bool	LoadPCParts( void );
	virtual	bool	LoadPCMotion( void );
	virtual	bool	Get8010Flag( void );
	virtual	bool	LoadDefaultMotion( void );
	virtual	void	SetInventoryInit(void);
	virtual	bool	SetInventory( int Tag, int Data );
	virtual bool	LoadInventory( char *FileName );
	virtual	int		CountBoneFromFile( char *FileName );
	virtual	int		CountTextureFromFile( char* FileName );
	virtual	int		GetType(void) { return m_mType; }
	virtual	int		GetRace(void) { return m_mRace; }
	virtual	int		GetFace(void) { return m_mFace; }
	virtual	int		GetHead(void) { return m_mHead; }
	virtual	int		GetBody(void) { return m_mBody; }
	virtual	int		GetHand(void) { return m_mHand; }
	virtual	int		GetLegs(void) { return m_mLegs; }
	virtual	int		GetFoot(void) { return m_mFoot; }
	virtual	int		GetInfoLWeapon(void) { return m_mInfoLWeapon; }
	virtual	int		GetInfoLWeapon2(void) { return m_mInfoLWeapon2; }
	virtual	int		GetInfoRWeapon(void) { return m_mInfoRWeapon; }
	virtual	int		GetInfoRWeapon2(void) { return m_mInfoRWeapon2; }
	virtual	int		GetInfoBody(void) { return m_mInfoBody; }
	virtual	int		GetRightWeapon(void) { return m_mRightWeapon; }
	virtual	int		GetLeftWeapon(void) { return m_mLeftWeapon; }
	virtual	int		GetRemoteWeapon(void) { return m_mRemoteWeapon; }

	virtual	void	SetInfoLWeapon( int InfoLWeapon ) { m_mInfoLWeapon = InfoLWeapon; }
	virtual	void	SetInfoLWeapon2( int InfoLWeapon2 ) { m_mInfoLWeapon2 = InfoLWeapon2; }
	virtual	void	SetInfoRWeapon( int InfoRWeapon ) { m_mInfoRWeapon = InfoRWeapon; }
	virtual	void	SetInfoRWeapon2( int InfoRWeapon2 ) { m_mInfoRWeapon2 = InfoRWeapon2; }
	virtual	void	SetInfoBody( int InfoBody ) { m_mInfoBody = InfoBody; }
	virtual	void	SetType( int mType ) { m_mType	=	mType; }
	virtual	void	SetRace( int mRace ) { m_mRace	=	mRace; }
	virtual	void	SetFace( int mFace ) { m_mFace	=	mFace; }
	virtual	void	SetHead( int mHead ) { m_mHead	=	mHead; }
	virtual	void	SetBody( int mBody ) { m_mBody	=	mBody; }
	virtual	void	SetHand( int mHand ) { m_mHand	=	mHand; }
	virtual	void	SetLegs( int mLegs ) { m_mLegs	=	mLegs; }
	virtual	void	SetFoot( int mFoot ) { m_mFoot	=	mFoot; }
	virtual	void	SetRightWeapon( int mRightWeapon ) { m_mRightWeapon	=	mRightWeapon; }
	virtual	void	SetLeftWeapon( int mLeftWeapon ) { m_mLeftWeapon	=	mLeftWeapon; }
	virtual	void	SetRemoteWeapon( int mRemoteWeapon ) { m_mRemoteWeapon	=	mRemoteWeapon; }

}
CPC,*LPCPC;

//======================================================================
// NPCクラス
//======================================================================
typedef class CNPC : public CModel
{
protected:
	int			m_mBody;	// 種族
public:
	CNPC();
	virtual ~CNPC();
	virtual	bool	LoadNPC( void );
	virtual	int		CountBoneFromFile( char* FileName );
	virtual	int		GetBody(void) { return m_mBody; }
	virtual	void	SetBody( int mBody ) { m_mBody	=	mBody; }
	virtual bool	LoadNPCMotion();
} CNPC,*LPCNPC;
