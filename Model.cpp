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
// �O���[�o��
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
extern	D3DXMATRIX	g_mViewLight;					// ���C�g���猩���ꍇ�̃r���[�}�g���b�N�X
extern	bool		g_mDispWire,g_mDispIdl,g_mDispBone;
extern	int			g_mDispBoneNo,g_mShlBoneNoR,g_mShlBoneNoL;

// �񓁗��R���o�[�g�e�[�u��
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

//�q�������A�q�������A�G�����A�G�����A�^�����A�^�����A�~�X���A�K���J
//�e�푰�̊�{���[�V�����̃t�@�C���i���o�[
int g_mBaseMotionFileNo[] = {27*0x80+82, 32*0x80+58, 37*0x80+31, 42*0x80+4, 46*0x80+93, 46*0x80+93, 51*0x80+89, 56*0x80+59};
//�e�푰�̐퓬���[�V�����̍ŏ��̃t�@�C���i���o�[
int g_mMotionBFileNo[] = {32*0x80+13, 36*0x80+117, 41*0x80+84, 46*0x80+57, 51*0x80+19, 51*0x80+19, 56*0x80+14, 60*0x80+112};
//�e�푰�̐퓬���[�V�����̃t�@�C����
int g_mmotionBnum[] = {9, 8, 10, 6, 6, 6, 9, 8};
//�e�푰�̕Ў蓁�̕���INFO
int g_msinobiINFO[] = {7, 6 ,3, 5, 3, 3, 3, 4};
//�e�푰�̗��蓁�̕���INFO
int g_mkatanaINFO[] = {-1, -1, -1, 7, -1, -1, -1, 8};
//�e�푰�̕Ў蓁�̐퓬���[�V�����̃t�@�C���i���o�[�@�G���E�K���͂��̎��ɗ��蓁���[�V����������
int g_mMotionB2FileNo[] = {98*0x80+55, 98*0x80+86, 98*0x80+117, 99*0x80+20, 99*0x80+55, 99*0x80+55, 99*0x80+86, 99*0x80+117};
//�e�푰�̕Ў蓁INFO�ȍ~�̕���Ńt�@�C���ԍ����J�艺���鐔
int g_mmotionB2num[] = {1, 1, 1, 2, 1, 1, 1, 2};
//�e�푰�̃G���[�V�����̍ŏ��̃t�@�C���i���o�[
int g_mMotionEFileNo[] = {32*0x80+40, 37*0x80+13, 41*0x80+114, 46*0x80+75, 51*0x80+37, 51*0x80+71, 56*0x80+41, 61*0x80+8};
//�e�푰�̃G���[�V�����̃t�@�C���̐�
int g_mmotionEnum = 6;
//�e�푰�̃`���R�{�E�ނ胂�[�V�����̍ŏ��̃t�@�C���i���o�[
int g_mMotionAFileNo[] = {90*0x80+14, 90*0x80+20, 90*0x80+26, 90*0x80+32, 90*0x80+38, 90*0x80+38, 90*0x80+44, 90*0x80+50};
//�e�푰�̃`���R�{�E�ނ胂�[�V�����̃t�@�C���̐�
int g_mmotionAnum = 2;
//�e�푰�̓񓁗����蕐��U�����[�V�����̍ŏ��̃t�@�C���i���o�[
int g_mMotionBLFileNo[] = {108*0x80+100, 110*0x80+19, 111*0x80+66, 112*0x80+113, 114*0x80+20, 114*0x80+20, 115*0x80+61, 116*0x80+108};
//�e�푰�̓񓁗����蕐��U�����[�V�����̃t�@�C����
int g_mmotionBLnum[] = {4, 4, 4, 2, 3, 3, 4, 3};
//�e�푰�̃E�F�|���o�b�V���H���[�V�����̍ŏ��̃t�@�C���i���o�[
int g_mMotionBAFileNo[] = {95*0x80+5, 95*0x80+14, 95*0x80+23, 95*0x80+32, 95*0x80+41, 95*0x80+41, 95*0x80+50, 95*0x80+59};
//�e�푰�̃E�F�|���o�b�V���H���[�V�����̃t�@�C���̐�
int g_mmotionBAnum = 3;
//�e�푰�̂�͂��E���Esit�G�����[�V�����̍ŏ��̃t�@�C���i���o�[
int g_mMotionTuruFileNo[] = {123*0x80+13, 123*0x80+22, 123*0x80+31, 123*0x80+40, 123*0x80+49, 123*0x80+58, 123*0x80+67, 123*0x80+76};
//�e�푰�̂�͂��E���Esit�G�����[�V�����̃t�@�C����
int g_mmotionTurunum = 3;


int Trim(char *s) {
	int i;
	int count = 0;

	/* ��|�C���^��? */
	if (s == NULL) { /* yes */
		return -1;
	}

	/* �����񒷂��擾���� */
	i = strlen(s);

	/* �������珇�ɋ󔒂łȂ��ʒu��T�� */
	while (--i >= 0 && s[i] == ' ') count++;

	/* �I�[�i��������t������ */
	s[i + 1] = '\0';

	/* �擪���珇�ɋ󔒂łȂ��ʒu��T�� */
	i = 0;
	while (s[i] != '\0' && s[i] == ' ') i++;
	strcpy(s, &s[i]);

	return i + count;
}

char * // ������ւ̃|�C���^
strrstr
(
const char *string, // �����Ώە�����
const char *pattern // �������镶����
)
{
	// ������I�[�ɒB����܂Ō������J��Ԃ��B
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
// ���_�t�H�[�}�b�g
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
//		�R���X�g���N�^
//======================================================================
CListBase::CListBase()
{
	ReferenceCount = 1;
	Prev = Next = NULL;
	pParentList = NULL;
}

//======================================================================
//		�f�X�g���N�^
//======================================================================
CListBase::~CListBase()
{
	if ( pParentList != NULL )
	{
		pParentList->Erase( this );
	}
}

//======================================================================
//		�J��
//======================================================================
long CListBase::Release( void )
{
	long ref = ReferenceCount - 1;

	// �Q�Ƃ��Ȃ��Ȃ�����j��
	if ( --ReferenceCount == 0 ) delete this;

	return ref;
}

//======================================================================
//		�Q�ƃJ�E���^�C���N�������g
//======================================================================
void CListBase::AddRef( void )
{
	ReferenceCount++;
}

//------------------------------------------------------------------------------------------//																						//
//									CList													//																						//
//------------------------------------------------------------------------------------------//
//======================================================================
//		�R���X�g���N�^
//======================================================================
CList::CList()
{
	Init();
}
//======================================================================
//		�f�X�g���N�^
//======================================================================
CList::~CList()
{
	Release();
}

//======================================================================
//		������
//======================================================================
void CList::Init( void )
{
	ListTop = NULL;
	ListEnd = NULL;
	Count = 0;
}

//======================================================================
//		�擪�擾
//======================================================================
LPCListBase CList::Top( void )
{
	return ListTop;
}

//======================================================================
//		�I�[�擾
//======================================================================
LPCListBase CList::End( void )
{
	return ListEnd;
}

//======================================================================
//		���X�g���
//======================================================================
void CList::Release( void )
{
	LPCListBase p = ListTop;
	while ( p != NULL )
	{
		// p �̎������O�Ɏ擾�ip �� Release() ���̂���Ă�\�������j
		LPCListBase pp = p->Next;
		// ���
		p->Release();
		// ��
		p = pp;
	}
	Init();
}

//======================================================================
//		���X�g�̐擪�ɑ}��
//======================================================================
void CList::InsertTop( LPCListBase p )
{
	//------------------------------------------------
	// ���̃��X�g�ɓo�^����Ă�Ƃ��͂����炩��ؒf
	//------------------------------------------------
	if ( p->pParentList != NULL ) p->pParentList->Erase( p );
	p->pParentList = this;

	//------------------------------------------------
	// �ڑ�
	//------------------------------------------------
	p->Prev = NULL;
	p->Next = ListTop;
	ListTop = p;
	if ( p->Next != NULL ) p->Next->Prev = p;
	if ( ListEnd == NULL ) ListEnd = p;
	Count++;
}

//======================================================================
//		���X�g�̏I�[�ɑ}��
//======================================================================
void CList::InsertEnd( LPCListBase p )
{
	//------------------------------------------------
	// ���̃��X�g�ɓo�^����Ă�Ƃ��͂����炩��ؒf
	//------------------------------------------------
	if ( p->pParentList != NULL ) p->pParentList->Erase( p );
	p->pParentList = this;

	//------------------------------------------------
	// �ڑ�
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
//		�^�[�Q�b�g�̑O�ɂɑ}��
//======================================================================
void CList::InsertPrev( LPCListBase pTarget, LPCListBase pIt )
{
	if( pIt == NULL ) return;
	if( pTarget==NULL ) {
		InsertTop( pIt );return;
	}
	//------------------------------------------------
	// ���̃��X�g�ɓo�^����Ă�Ƃ��͂����炩��ؒf
	//------------------------------------------------
	if ( pIt->pParentList != NULL ) pIt->pParentList->Erase( pIt );
	pIt->pParentList = this;

	//------------------------------------------------
	// �ڑ�
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
//		�^�[�Q�b�g�̎��ɑ}��
//======================================================================
void CList::InsertNext( LPCListBase pTarget,LPCListBase pIt )
{
	if(  pIt== NULL ) 
		return;
	if( pTarget==NULL ) {
		InsertTop( pIt );return;
	}
	//------------------------------------------------
	// ���̃��X�g�ɓo�^����Ă�Ƃ��͂����炩��ؒf
	//------------------------------------------------
	if ( pIt->pParentList != NULL ) pIt->pParentList->Erase( pIt );
	pIt->pParentList = this;

	//------------------------------------------------
	// �ڑ�
	//------------------------------------------------

	pIt->Prev = pTarget;
	pIt->Next = pTarget->Next;
	pTarget->Next = pIt;
	if( ListEnd == pTarget ) 
		ListEnd = pIt;
	Count++;
}
//======================================================================
//		���X�g����폜
//======================================================================
void CList::Erase( LPCListBase p )
{
	if ( p->pParentList != NULL ) p->pParentList = NULL;

	BYTE flag = 0x00;
	if ( p->Prev == NULL ) flag |= 0x01;		// �O�ɉ����Ȃ��Ƃ�
	if ( p->Next == NULL ) flag |= 0x02;		// ��ɉ����Ȃ��Ƃ�

	//============================================
	//	�Y������f�[�^�̍폜
	//============================================
	switch ( flag )
	{
	///////////////////////////////////// �O��ɉ�������Ƃ�
	case 0x00:
		p->Prev->Next = p->Next;
		p->Next->Prev = p->Prev;
		break;
	///////////////////////////////////// �O�ɉ����Ȃ��Ƃ�
	case 0x01:
		ListTop = p->Next;
		ListTop->Prev = NULL;
		break;
	///////////////////////////////////// ��ɉ����Ȃ��Ƃ�
	case 0x02:
		ListEnd = ListEnd->Prev;
		p->Prev->Next = NULL;
		break;
	///////////////////////////////////// �O��ɉ����Ȃ��Ƃ�
	case 0x03:
		ListTop = NULL;
		ListEnd = NULL;
		break;
	}
	Count--;
}

//======================================================================
//		����̃f�[�^���o��
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
//		�T�C�Y�擾
//======================================================================
long CList::Size( void )
{
	return Count;
}

//------------------------------------------------------------------------------------------//																					//
//									CStream													//																				//
//------------------------------------------------------------------------------------------//


//======================================================================
//		�R���X�g���N�^
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
//		�f�X�g���N�^
//======================================================================
CStream::~CStream()
{
}

//======================================================================
//		�C���f�b�N�X�f�[�^�ݒ�
//======================================================================
void CStream::SetData( D3DPRIMITIVETYPE PrimitiveType, unsigned long index_start, unsigned long face_count )
{
	m_PrimitiveType	= PrimitiveType;
	m_IndexStart	= index_start;
	m_FaceCount		= face_count;
}

//======================================================================
//		�v���~�e�B�u�^�C�v�擾
//======================================================================
D3DPRIMITIVETYPE CStream::GetPrimitiveType( void )
{
	return m_PrimitiveType;
}

//======================================================================
//		�C���f�b�N�X�J�n�ʒu�擾
//======================================================================
unsigned long CStream::GetIndexStart( void )
{
	return m_IndexStart;
}

//======================================================================
//		�C���f�b�N�X�g�p���擾
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
//		�R���X�g���N�^
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
//		�f�X�g���N�^
//======================================================================
CMaterial::~CMaterial()
{
	SAFE_RELEASE( m_pTexture );
}

//======================================================================
//		�C���f�b�N�X�f�[�^�ݒ�
//======================================================================
void CMaterial::SetData( unsigned long index_start, unsigned long face_count )
{
	m_IndexStart	= index_start;
	m_FaceCount		= face_count;
}

//======================================================================
//		�}�e���A���f�[�^�ݒ�
//======================================================================
void CMaterial::SetMaterial( D3DMATERIAL9 *pMat )
{
	m_Material = *pMat;
}

//======================================================================
//		�e�N�X�`���ݒ�
//======================================================================
void CMaterial::SetTexture( IDirect3DTexture9 *pTex )
{
	SAFE_RELEASE( m_pTexture );
	m_pTexture = pTex;
	if ( m_pTexture != NULL ) m_pTexture->AddRef();
}

//======================================================================
//		�C���f�b�N�X�J�n�ʒu�擾
//======================================================================
unsigned long CMaterial::GetIndexStart( void )
{
	return m_IndexStart;
}

//======================================================================
//		�C���f�b�N�X�g�p���擾
//======================================================================
unsigned long CMaterial::GetFaceCount( void )
{
	return m_FaceCount;
}

//======================================================================
//		�}�e���A���f�[�^�擾
//======================================================================
D3DMATERIAL9 *CMaterial::GetMaterial( void )
{
	return &m_Material;
}

//======================================================================
//		�e�N�X�`���擾
//======================================================================
IDirect3DTexture9 *CMaterial::GetTexture( void )
{
	return m_pTexture;
}

//------------------------------------------------------------------------------------------//																				//
//									CMesh													//																				//
//------------------------------------------------------------------------------------------//
//======================================================================
//		�R���X�g���N�^
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
//		�f�X�g���N�^
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
//  �{�[���e�[�u���̍쐬
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
//		���b�V���ǂݍ���
//======================================================================

HRESULT CMesh::LoadMesh( char *pFile, CModel *pModel, unsigned long FVF,int Flip  )
{
	HRESULT hr						= D3D_OK; 

	//==============================================================
	// ���b�V���̐���
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
	//  �{�[���e�[�u���̍쐬
	//========================================================

	SetMeshBone( Num1, Num2, pcp->Type, pcp->BoneTblNum, pBone, pBoneTbl, Flip );

	//==============================================================
	// Mesh Data inport
	//  Vertex Inport
	CUSTOMVERTEX*	pV;
	if( FAILED( m_lpVB1->Lock( 0,					// �o�b�t�@�̍ŏ�����f�[�^���i�[����B
						m_VBSize,					// ���[�h����f�[�^�̃T�C�Y�B
						(void**)&pV,				// �Ԃ����C���f�b�N�X �f�[�^�B
						D3DLOCK_DISCARD ) ) )       // �f�t�H���g �t���O�����b�N�ɑ���B
		return E_FAIL;
	if( pcp->Type&0x7f ) {	// �N���X
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
	if( FAILED( m_lpVB2->Lock( 0,					// �o�b�t�@�̍ŏ�����f�[�^���i�[����B
						m_VBSize,					// ���[�h����f�[�^�̃T�C�Y�B
						(void**)&pV,				// �Ԃ����C���f�b�N�X �f�[�^�B
						D3DLOCK_DISCARD ) ) )       // �f�t�H���g �t���O�����b�N�ɑ���B
		return E_FAIL;
	if( pcp->Type&0x7f ) {	// �N���X
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

 	if( FAILED( m_lpIB->Lock( 0,                 // �o�b�t�@�̍ŏ�����f�[�^���i�[����B
						m_IBSize, // ���[�h����f�[�^�̃T�C�Y�B
						(void**)&pIndex, // �Ԃ����C���f�b�N�X �f�[�^�B
						D3DLOCK_DISCARD ) ) )            // �f�t�H���g �t���O�����b�N�ɑ���B
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
			// �e�N�X�`�����̂̐ݒ�
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

	//���_���o��
	fprintf(fd, " %d;\n", m_NumVertices);
	// ���_���W�o��
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
	
	// �ʐ��o��
	fprintf(fd, " %d;\n", m_NumFaces);
	// �ʃf�[�^�o��
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

	//���_���o��
	fprintf(fd, " %d;\n", m_NumVertices);
	// ���_���W�o��
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
	// �e�N�X�`�����W���o��
	fprintf(fd, " %d;\n", m_NumVertices);
	// �e�N�X�`�����W�f�[�^�o��
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

	// �}�e���A�����o��
//	fprintf(fd, " %d;\n", numTex);
	fprintf(fd, " 1;\n");
	// �ʐ��o��
	fprintf(fd, "  %d;\n", m_NumFaces);
	// �}�e���A���ԍ��o��
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
//		�R���X�g���N�^
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
//		�f�X�g���N�^
//======================================================================
CBone::~CBone()
{
}

//======================================================================
//		�R���X�g���N�^
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
//		�ϊ��s��ǂݍ���
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
//		���O�ݒ�
//======================================================================
void CBone::SetName( char *name )
{
	memset( m_Name, 0x00, sizeof(m_Name) );
	strcpy( m_Name, name );
}

//======================================================================
//		���O�`�F�b�N
//======================================================================
bool CBone::CheckName( char *name )
{
	return (strcmp( m_Name, name ) == 0);
}

//======================================================================
//		�A�j���[�V�����ݒ�
//======================================================================
void CBone::SetMotion( CMotionElement *pMotion )
{
	m_pMotion = pMotion;
}

//======================================================================
//		���O�擾
//======================================================================
char *CBone::GetName( void )
{
	return m_Name;
}

//======================================================================
//		�s��擾
//======================================================================
D3DXMATRIX *CBone::GetMatrix( void )
{
	return &m_mWorld;
}

//======================================================================
//
//		�t���[���ǂݍ���
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
//		char *		: �G���[���b�Z�[�W�ւ̃|�C���^�i�G���[���Ȃ����NULL
//
//======================================================================
char *CBone::LoadBone( char *pFile )
{
	// �{�[���\����
	typedef struct {
		unsigned char	parent_num,term;
		float			qx,qy,qz,qw;
		float			x,y,z;
	} BONE;
	// ���[�J���ϐ�
	char ErrMsg[1024]						= "";
	char *err								= ErrMsg;
	char Name[256]							= "";

	//==============================================================
	// �y�A�����g
	//==============================================================
	m_mParent = (int)*(unsigned char*)(pFile+0);
	m_mTerm   = *(unsigned char*)(pFile+1)?true:false;

	//====================================================
	// �t���[�������擾
	//====================================================
	SetName( Name );

	//====================================================
	// ���̊K�w�ɍs��
	//====================================================
	LoadTransformMatrix( pFile+2 );
	err = NULL;
	return err;
}

//------------------------------------------------------------------------------------------//																					//
//								CMotionFrame												//																				//
//------------------------------------------------------------------------------------------//
//======================================================================
//		�R���X�g���N�^
//======================================================================
CMotionFrame::CMotionFrame()
{
	memset( m_Name, 0x00, sizeof(m_Name) );
}

//======================================================================
//		�f�X�g���N�^
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
//		������
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
//		�A�j���[�V�������̂̐ݒ�
//======================================================================

int CMotionFrame::SetMotionName( char *MotionName )
{
	strncpy(m_Name,MotionName,4);m_Name[4]='\0';
	return 0;
}
//======================================================================
//		�A�j���[�V�������̂̎擾
//======================================================================

char *CMotionFrame::GetMotionName(void)
{
	return m_Name;
}

//======================================================================
//		�A�j���[�V�����Z�b�g�ǂ݂���
//======================================================================
void CMotionFrame::LoadMotionSet( char *pFile )
{
	//====================================================
	// �t���[�������擾
	//====================================================

	DAT2B *dat							= NULL;
	int   Element						= 0;
	int   Keys							= 0;
	float Times							= 0.f;
	float *fdat							= NULL;

	//====================================================
	// �f�[�^�擾
	//====================================================
	SetMotionName( pFile );
	Element = (int)*((short*)(pFile+18));
	Keys    = (int)*((short*)(pFile+20));
    Times   = *((float*)(pFile+22));

	dat = (DAT2B *)(pFile+26);
	fdat = (float*)(pFile+26);
	//====================================================
	// ���̊K�w�ɍs��
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
//		�R���X�g���N�^
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
//		�f�X�g���N�^
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
//		�N���X������
//======================================================================
CMotionElement::~CMotionElement()
{
	SAFE_DELETES( m_pRotationKeys );
	SAFE_DELETES( m_pScalingKeys );
	SAFE_DELETES( m_pTranslateKeys );
	SAFE_DELETES( m_pMatrixKeys );
}
//======================================================================
//		�{�[���ԍ��̃Z�b�g
//======================================================================

int CMotionElement::SetBoneNo(int BoneNo )
{
	m_BoneNo = BoneNo;
	return 0;
}

//======================================================================
//		�{�[���ԍ��̎擾
//======================================================================

int CMotionElement::GetBoneNo(void)
{
	return m_BoneNo;
}
//======================================================================
//		�A�j���[�V�����ǂݍ���
//======================================================================
void CMotionElement::LoadMotion(int No, int Keys,
					float Times,DAT2B *bDat,float *fDat)	
{
	//-----------------------------------------------------
	// ��]
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
	// �X�P�[��
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
	// �ړ�
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
//		�A�j���[�V�������Ԏ擾
//======================================================================
float CMotionElement::GetFinalTime( void )	
{
	float Time = 0.f;

	if( m_pRotationKeys ) Time = (float)m_pRotationKeys[m_RotationKeyNum-1].Time;
	return Time;
}

//======================================================================
//		�A�j���[�V�����ǉ�
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
	// ��]
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
	// �X�P�[��
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
	// �ړ�
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
//		�R�s�[�@MotionElement
//======================================================================
bool CMotionElement::CopyMotionElement( CMotionElement *pAnimElement )
{
	unsigned int i;

	if( m_pRotationKeys ) SAFE_DELETES( m_pRotationKeys );
	if( m_pScalingKeys ) SAFE_DELETES( m_pScalingKeys );
	if( m_pTranslateKeys ) SAFE_DELETES( m_pTranslateKeys );

	//-----------------------------------------------------
	// ��]
	//-----------------------------------------------------
	m_RotationKeyNum = pAnimElement->m_RotationKeyNum;
	m_pRotationKeys = new KEYROTATION [ m_RotationKeyNum ];
	for ( i = 0; i < m_RotationKeyNum ; i++ ) {
		m_pRotationKeys[i]		= pAnimElement->m_pRotationKeys[i];
	}
	//-----------------------------------------------------
	// �X�P�[��
	//-----------------------------------------------------
	m_ScalingKeyNum = pAnimElement->m_ScalingKeyNum;
	m_pScalingKeys = new KEYSCALING [ m_ScalingKeyNum ];
	for ( i = 0; i < m_ScalingKeyNum ; i++ ) {
		m_pScalingKeys[i]		= pAnimElement->m_pScalingKeys[i];
	}
	//-----------------------------------------------------
	// �ړ�
	//-----------------------------------------------------
	m_TranslateKeyNum = pAnimElement->m_TranslateKeyNum;
	m_pTranslateKeys = new KEYTRANSLATION [ m_TranslateKeyNum ];
	for ( i = 0; i < m_TranslateKeyNum ; i++ ) {
		m_pTranslateKeys[i]	= pAnimElement->m_pTranslateKeys[i];
	}
	return true;
}

//======================================================================
//		�ǉ�MotionElement
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
	// ��]
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
	// �X�P�[��
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
	// �ړ�
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
//		�g��k���L�[�擾
//======================================================================
D3DXMATRIX *CMotionElement::GetMotionScaling( float time )
{
	static D3DXMATRIX Matrix;

	unsigned long KeyNum = m_ScalingKeyNum;
	KEYSCALING *pKeys = m_pScalingKeys;

	float fTime = (float)fmod( time, (float)pKeys[KeyNum-1].Time );

	// ���݂̎��Ԃ��ǂ̃L�[�ߕӂɂ��邩�H
	unsigned long i = 0;
	for ( i = 0; i < KeyNum; i++ ) if ( fTime <= (float)pKeys[i].Time ) break;

	// �O��̃L�[��ݒ�
	unsigned long Key0 = (i == 0) ? (0) : (i - 1);
	unsigned long Key1 = (i == KeyNum) ? (0) : (i);

	// �O�̃t���[�����Ԃƌ��̃t���[������
	float fTime1 = (float)pKeys[Key0].Time;
	float fTime2 = (float)pKeys[Key1].Time;

	// ���`�⊮
	float LerpValue = 0;
	if ( fTime1 != fTime2 ) LerpValue = (fTime - fTime1)  / (fTime2 - fTime1);

	D3DXVECTOR3 vScale;
	D3DXVec3Lerp( &vScale, &pKeys[Key0].Scale, &pKeys[Key1].Scale, LerpValue );
	D3DXMatrixScaling( &Matrix, vScale.x, vScale.y, vScale.z );

	return &Matrix;
}

//======================================================================
//		�ړ��L�[�擾
//======================================================================
D3DXMATRIX *CMotionElement::GetMotionTranslate( float time )
{
	static D3DXMATRIX Matrix;

	unsigned long KeyNum = m_TranslateKeyNum;
	KEYTRANSLATION *pKeys = m_pTranslateKeys;

	float fTime = (float)fmod( time, (float)pKeys[KeyNum-1].Time );

	// ���݂̎��Ԃ��ǂ̃L�[�ߕӂɂ��邩�H
	unsigned long i = 0;
	for ( i = 0; i < KeyNum; i++ ) if ( fTime <= (float)pKeys[i].Time ) break;

	// �O��̃L�[��ݒ�
	unsigned long Key0 = (i == 0) ? (0) : (i - 1);
	unsigned long Key1 = (i == KeyNum) ? (0) : (i);

	// �O�̃t���[�����Ԃƌ��̃t���[������
	float fTime1 = (float)pKeys[Key0].Time;
	float fTime2 = (float)pKeys[Key1].Time;

	// ���`�⊮
	float LerpValue = 0;
	if ( fTime1 != fTime2 ) LerpValue = (fTime - fTime1)  / (fTime2 - fTime1);

	D3DXVECTOR3 vPos;
	D3DXVec3Lerp( &vPos, &pKeys[Key0].Pos, &pKeys[Key1].Pos, LerpValue );
	D3DXMatrixTranslation( &Matrix, vPos.x, vPos.y, vPos.z );

	return &Matrix;
}

//======================================================================
//		��]�L�[�擾
//======================================================================
D3DXMATRIX *CMotionElement::GetMotionRotation( float time )
{
	static D3DXMATRIX Matrix;

	unsigned long KeyNum = m_RotationKeyNum;
	KEYROTATION *pKeys = m_pRotationKeys;

	float fTime = (float)fmod( time, (float)pKeys[KeyNum-1].Time );

	// ���݂̎��Ԃ��ǂ̃L�[�ߕӂɂ��邩�H
	unsigned long i = 0;
	for ( i = 0; i < KeyNum; i++ ) if ( fTime <= (float)pKeys[i].Time ) break;

	// �O��̃L�[��ݒ�
	unsigned long Key0 = (i == 0) ? (0) : (i - 1);
	unsigned long Key1 = (i == KeyNum) ? (0) : (i);

	// �O�̃t���[�����Ԃƌ��̃t���[������
	float fTime1 = (float)pKeys[Key0].Time;
	float fTime2 = (float)pKeys[Key1].Time;

	// ���`�⊮
	float LerpValue = 0;
	if ( fTime1 != fTime2 ) LerpValue = (fTime - fTime1)  / (fTime2 - fTime1);

	// �N�I�[�^�j�I���Ԃŕ⊮
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
//		Animation Matrix ����
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
//		Matrix ����
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

	// ���݂̎��Ԃ��ǂ̃L�[�ߕӂɂ��邩�H
	unsigned long i = 0;
	for (i = 0; i < KeyNum; i++) if (fTime <= (float)pKeys1[i].Time) break;

	// �O��̃L�[��ݒ�
	unsigned long Key0 = (i == 0) ? (0) : (i - 1);
	unsigned long Key1 = (i == KeyNum) ? (0) : (i);

	// �O�̃t���[�����Ԃƌ��̃t���[������
	float fTime1 = (float)pKeys1[Key0].Time;
	float fTime2 = (float)pKeys1[Key1].Time;

	// ���`�⊮
	float LerpValue = 0;
	if (fTime1 != fTime2) LerpValue = (fTime - fTime1) / (fTime2 - fTime1);

	// �N�I�[�^�j�I���Ԃŕ⊮
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
//		Matrix ����2
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

	// ���݂̎��Ԃ��ǂ̃L�[�ߕӂɂ��邩�H
	unsigned long i = 0;
	for (i = 0; i < KeyNum; i++) if (fTime <= (float)pKeys1[i].Time) break;

	// �O��̃L�[��ݒ�
	unsigned long Key0 = (i == 0) ? (0) : (i - 1);
	unsigned long Key1 = (i == KeyNum) ? (0) : (i);

	// �O�̃t���[�����Ԃƌ��̃t���[������
	float fTime1 = (float)pKeys1[Key0].Time;
	float fTime2 = (float)pKeys1[Key1].Time;

	// ���`�⊮
	float LerpValue = 0;
	if (fTime1 != fTime2) LerpValue = (fTime - fTime1) / (fTime2 - fTime1);

	// �N�I�[�^�j�I���Ԃŕ⊮
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
//		�R���X�g���N�^
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
//		�f�X�g���N�^
//======================================================================
CData::~CData()
{
	m_Materials.Release();
}

//======================================================================
//
//		�e�N�X�`���̓ǂݍ���
//
//	input
//		char *filename			: �ǂݍ��݃t�@�C�����i���\�[�X���ł���
//		unsigned long FVF		: ���b�V����FVF
//
//	output
//		�G���[������ւ̃|�C���^�B
//		����I���̏ꍇ��NULL�B
//
//======================================================================
HRESULT CData::LoadTextureFromFile( char *FileName  )
{
	HRESULT hr							= S_OK;

	//====================================================
	// �t�@�C�����������Ɏ�荞��
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
	// �e�N�X�`���̓ǂݍ���
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
			// �e�N�X�`��
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
	// �I��
	delete pdat;
	return hr;
}


//======================================================================
//
//		���f���̍s�񏉊���
//
//		�s������������܂��B
//
//======================================================================
void CData::InitTransform( void )
{
	D3DXMatrixIdentity( &m_mRootTransform );
}
 

//======================================================================
//
//		�s��擾
//
//		���f���̍s��Ɏ擾���܂��B
//
//	input
//		D3DXMATRIX &mat		: �擾�����s����i�[����s��
//
//======================================================================
void CData::GetMatrix( D3DXMATRIX &mat )
{
	mat = m_mRootTransform;
}

//======================================================================
//
//		�s��ݒ�
//
//		���f���̍s��ɔC�ӂ̍s��ɐݒ�B
//
//	input
//		D3DXMATRIX &mat		: �ݒ肷��s��
//
//======================================================================
void CData::SetMatrix( D3DXMATRIX &mat )
{
	m_mRootTransform = mat;
}

//======================================================================
//
//		�s���Z
//
//		���f���̍s��ɔC�ӂ̍s�����Z���܂��B
//
//	input
//		D3DXMATRIX &mat		: ��Z����s��
//
//======================================================================
void CData::MulMatrix( D3DXMATRIX &mat )
{
	m_mRootTransform *= mat;
}

//======================================================================
//
//		�ړ�
//
//		���f�����ړ������܂��B
//
//	input
//		float px		: �w�������ւ̈ړ���
//		float py		: �x�������ւ̈ړ���
//		float pz		: �y�������ւ̈ړ���
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
//		�g��k��
//
//		�g��k�������܂��B
//		�g��k���̓��f���̖@���ɂ��e�����o�܂��̂ŁA
//		���̏ꍇ�̓����_�����O���ɖ@���̐��K�����K�v�ł��B
//
//	input
//		float sx		: �w�����̊g�嗦
//		float sy		: �x�����̊g�嗦
//		float sz		: �y�����̊g�嗦
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
//		�w����]
//
//		�w���ŉ�]�����܂��B
//
//	input
//		float rot			: ��]�p�x�i���W�A���Ŏw��
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
//		�x����]
//
//		�x���ŉ�]�����܂��B
//
//	input
//		float rot			: ��]�p�x�i���W�A���Ŏw��
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
//		�y����]
//
//		�y���ŉ�]�����܂��B
//
//	input
//		float rot			: ��]�p�x�i���W�A���Ŏw��
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
//		�w��ʒu�𒆐S�ɉ�]
//
//		�w��ʒu��Y���ŉ�]�����܂��B
//
//	input
//		float pos			: ��]���S�ʒu
//		float rot			: ��]�p�x�i���W�A���Ŏw��
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
//		���̈ʒu�𒆐S�ɉ�]
//
//		���̈ʒu��Y���ŉ�]�����܂��B
//
//	input
//		float rot			: ��]�p�x�i���W�A���Ŏw��
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
//		�w���~���[
//
//		�w���Ń~���[�����܂��B
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
//		�x���~���[
//
//		�x���Ń~���[�����܂��B
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
//		�y���~���[
//
//		�y���Ń~���[�����܂��B
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
//		����
//
//		���݂̃��[���h��Ԉʒu���� at �����𒍎����܂��B
//		���̂Ƃ�������� up �x�N�g���Ŏw�肵�܂��B
//
//	input
//		D3DXVECTOR3 &at		: �����_
//		D3DXVECTOR3 &up		: ������i�ȗ��� { 0, 1, 0 }
//
//	output
//		true ���� / false ���s�i�߂�����
//
//======================================================================
bool CData::LookAt( D3DXVECTOR3 &at, D3DXVECTOR3 &up )
{
	float length;
	D3DXVECTOR3 from, right;

	// ���f���̈ʒu���擾
	GetWorldPosition( from );

	// ���_�̂y�x�N�g�����擾
	D3DXVECTOR3 eye = at - from;
	length = D3DXVec3Length( &eye );
	if ( length < 1e-6f ) return false;
	// ���K��
	eye *= 1.0f/length;

	up -= -D3DXVec3Dot( &up, &eye ) * eye;
	length = D3DXVec3Length( &up );

	// �������A�b�v�x�N�g���̌���
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

	// �x�y�тw�x�N�g�����擾
	up *= 1.0f/length;
	D3DXVec3Cross( &right, &up, &eye );

	// �I�u�W�F�N�g�𒍎�
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
//		���[���h��Ԃł̈ʒu
//
//		���[���h��Ԃł̈ʒu���擾���܂��B
//
//	input
//		D3DXVECTOR3 &pos		: �ʒu���i�[����x�N�g��
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
//		X�������o���܂��B
//
//	input
//		D3DXVECTOR3 &pos		: X�����i�[����x�N�g��
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
//		Y�������o���܂��B
//
//	input
//		D3DXVECTOR3 &pos		: Y�����i�[����x�N�g��
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
//		Z�������o���܂��B
//
//	input
//		D3DXVECTOR3 &pos		: Z�����i�[����x�N�g��
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
//		�X�N���[���ł̈ʒu
//
//		�J�����̑O���ɂ���ꍇ��X,Y���W��0�`1���i�[�����B
//		�J�����̌���ɂ���ꍇ�̒l�͕s��B
//
//	input
//		D3DXVECTOR4 &pos		: �ʒu���i�[����x�N�g��
//
//	output
//		true �J�����̑O / false �J�����̌��
//
//======================================================================
bool CData::GetScreenPosition( D3DXVECTOR4 &pos )
{
	// �I�u�W�F�N�g�~�r���[�~���e�̃}�g���N�X���擾
	D3DXMATRIX mat;
	GetMatrix( mat );
	D3DXMATRIX mTransform = mat * g_mView * g_mProjection;

	// �X�N���[�����W�̎Z�o
	float x = mTransform._41;
	float y = mTransform._42;
	float z = mTransform._43;
	float w = mTransform._44;
	float sz = z / w;

	// �I�u�W�F�N�g���J�����̑O���ɂ���ꍇ
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
//		�R���X�g���N�^
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
//		�f�X�g���N�^
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
//		�f�[�^�̏�����
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
//		�f�[�^�̏�����
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
//		�{�[���̓ǂݍ���
//
//	input
//		char *filename			: �ǂݍ��݃t�@�C�����i���\�[�X���ł���
//		unsigned long FVF		: ���b�V����FVF
//
//	output
//		�G���[������ւ̃|�C���^�B
//		����I���̏ꍇ��NULL�B
//
//======================================================================
HRESULT CModel::LoadBoneFromFile( char *FileName )
{
	HRESULT hr							= S_OK;
	
	//====================================================
	// �t�@�C�����������Ɏ�荞��
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
	// �{�[���̓ǂݍ���
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
	// �I��
	delete pdat;
	return hr;
}

//======================================================================
//
//		���b�V���̓ǂݍ���
//
//	input
//		char *filename			: �ǂݍ��݃t�@�C�����i���\�[�X���ł���
//		unsigned long FVF		: ���b�V����FVF
//
//	output
//		�G���[������ւ̃|�C���^�B
//		����I���̏ꍇ��NULL�B
//
//======================================================================
HRESULT CModel::LoadMeshFromFile( char *FileName, unsigned long FVF,int PartsNo )
{
#pragma pack(push,2)
	typedef struct{
	  char		name[16];			//      ���O
	  BYTE		Ver;				//0x00
	  BYTE		nazo;				//0x01
	  WORD		Type;				//0x02 &7f==0���f�� 1=�N���X
	  WORD		Flip;				//0x04 0==OFF  ON
	} DAT2AH ;
#pragma pack(pop)

	DAT2AH	*pHeader;
	HRESULT hr							= S_OK;

	//====================================================
	// �t�@�C�����������Ɏ�荞��
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
	// ���b�V���̓ǂݍ���
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
	// ���b�V���̓ǂݍ���
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
	// �I��
	delete pdat;
	return hr;
}
//======================================================================
//
//		���b�V���C���t�H�̓ǂݍ���
//
//	input
//		char *filename			: �ǂݍ��݃t�@�C�����i���\�[�X���ł���
//		int  Offset				: �擪����̃I�t�Z�b�g
//
//	output
//		�l�B
//		�C���t�H���Ȃ��ꍇ�͂O�B
//
//======================================================================
int CModel::LoadInfoFromFile( char *FileName, int Offset )
{
	//====================================================
	// �t�@�C�����������Ɏ�荞��
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
	// ���b�V���̓ǂݍ���
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
//		���[�V�����̓ǂݍ���
//
//	input
//		char *filename			: �ǂݍ��݃t�@�C�����i���\�[�X���ł���
//		unsigned long FVF		: ���b�V����FVF
//
//	output
//		�G���[������ւ̃|�C���^�B
//		����I���̏ꍇ��NULL�B
//
//======================================================================
HRESULT CModel::LoadMotionFromFile( char *FileName )
{
	HRESULT hr							= S_OK;

	//====================================================
	// �t�@�C�����������Ɏ�荞��
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
	// ���[�V�����̓ǂݍ���
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
	// �I��
	delete pdat;
	return hr;
}

//======================================================================
//
//		���[�V������3���������E�ݒ�i��v������̂��ׂāj
//
//	input
//		char *MotionName		: �������[�V������
//
//	output
//		����I���̏ꍇ��OK�B
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
//		���[�V������4���������E�ݒ�i4�������S��v�̂݁j
//
//	input
//		char *MotionName		: �������[�V������
//
//	output
//		����I���̏ꍇ��OK�B
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
//		���f���̍s�񏉊���
//
//		�S�t���[���̍s������������܂��B
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
//		���f���̍s��X�V
//		�S�t���[���̍s����Z�o���܂��B
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
//		���f���̍s��X�V
//		�S�t���[���̍s����Z�o���܂��B
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
//		�t���[���̍ő厞�Ԃ��Z�o
//
//		�S�t���[���̍s����Z�o���܂��B
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
//		�����_�����O
//
//		���f���f�[�^��e�q�֌W���܂߂ă����_�����O���܂��B
//
//	output
//		�����_�����O�����|���S����
//
//======================================================================
unsigned long CModel::Rendering( void )
{
	int				DispCheck;
	unsigned long	count = 0;

	GetDevice()->SetRenderState(D3DRS_LIGHTING, FALSE);   // ���d�����񂷁I
	//GetDevice()->SetRenderState(D3DRS_AMBIENT, 0xff030303);   // ���̒���������Ɣ����Ƃ炷
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
	// ���_����
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
		// ���_�o�b�t�@1���f�o�C�X�ɐݒ�
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
		// ���_�o�b�t�@1���f�o�C�X�ɐݒ�
		//---------------------------------------------------------
		GetDevice()->SetStreamSource(0, pMesh->m_lpVB2, 0, D3DXGetFVFVertexSize(pMesh->m_FVF));
		//---------------------------------------------------------
		// �C���f�b�N�X�o�b�t�@���f�o�C�X�ɐݒ�
		//---------------------------------------------------------
		GetDevice()->SetIndices(pMesh->m_lpIB);

		CStream		*pStream;
		//---------------------------------------------------------
		// �T�[�t�F�C�X���ƂɃ����_�����O
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
			// c14 : �}�e���A��
			//
			CMaterial *pMaterial = (CMaterial*)pStream->m_pMaterial;

			// �f�o�C�X�Ƀe�N�X�`���ݒ�
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
//		�����_�����O
//
//		�{�[���f�[�^�������_�����O���܂��B
//======================================================================
void CModel::BoneRendering( void )
{
	D3DXVECTOR3		pos[3];
	D3DXMATRIX		mat;

//	// �ϊ��s��
	GetDevice()->SetRenderState(D3DRS_LIGHTING, FALSE);   // ���d�����񂷁I
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
//		X�t�@�C���Z�[�u
//		�f�[�^��MQO�t�H�[�}�b�g�ŏo�͂��܂�
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
		wsprintf(strmsg, "�t�@�C�� %s�@���㏑�����܂����H", path);
		if (MessageBox(NULL, strmsg, "�Z�[�u�t�@�C���㏑��", MB_YESNO | MB_ICONQUESTION) == IDNO) return false;
	}
	if ((fd = fopen(path, "w")) == NULL) return false;
	fprintf(fd, "xof 0303txt 0032\n\n");
	fprintf(fd, "AnimTickPerSecond {\n\t\t3000;\n\t}\n");
	fprintf(fd, " Frame Scene_Root {\n");
	outputMatrix(fd, &rootMatrix);
	fprintf(fd, " Frame body {\n");
	outputMatrix(fd, &lmatrix); 
#if 0
	outputMultiMeshX(FPath, FName, fd); // �X�L���}���`���b�V���@���i�����p
#else
	fprintf(fd, " Mesh {\n"); // �X�L���������b�V���p
	outputMeshX(FPath, FName, fd);// �X�L���������b�V���p 
	outputSkinX(fd);// �X�L���������b�V���p
	fprintf(fd, "}\n");// �X�L���������b�V���p
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
	//�@���o��
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
		// �ʐ��J�E���g
		int dispCheck = pMesh->GetDispCheck();
		int numFace = pMesh->cntNumFace();
		// �t���[���o��
		fprintf(fd, " Frame Parts%03d {\n", partsNo);
		outputMatrix(fd, &lmatrix);
		// ���b�V�����o��
		fprintf(fd, " Mesh Parts%03d {\n", partsNo);
		//fprintf(fd, " Mesh %s {\n", pMesh->m_Name);
		pMesh->prtVertex(fd);
		pMesh->prtFace(fd);
		//�@���o��
		fprintf(fd, "MeshNormals {\n");
		// �@�����_�o��
		pMesh->prtNormal(fd);
		// �@���ʐ��o��
		pMesh->prtFace(fd);
		fprintf(fd, "     }\n");
		// �e�N�X�`�����W�o��
		pMesh->prtTexCoord(fd);

		//�@�}�e���A���o��
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
		fprintf(fd, "}\n"); // ���b�V���I��
		fprintf(fd, "}\n"); // �t���[���I��
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

	// ���_���A�ʐ��J�E���g
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

	// ���_���A�ʐ��J�E���g
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
	//���_���o��
	fprintf(fd, " %d;\n", numVer);
	// ���_���W�o��
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
	// �ʐ��o��
	fprintf(fd, " %d;\n", numFace);
	// �p�b�`�o��
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

	// �@�����o��
	fprintf(fd, " %d;\n", numVer);
	// �@���x�N�g���o��
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
	// �ʐ��o��
	fprintf(fd, " %d;\n", numFace);
	// �p�b�`�o��
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

	// �e�N�X�`��u,v�o��
	fprintf(fd, "   MeshTextureCoords {\n");
	// uv���o��
	fprintf(fd, " %d;\n", numVer);
	// �e�N�X�`��U,V�o��
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
	// ���_�d�����X�g
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
	// mesh�}�e���A�����X�g�o��
	fprintf(fd, "  MeshMaterialList {\n");
	// �}�e���A�����o��
	fprintf(fd, "%d;\n", m_Materials.Count);
	// �ʐ��o��
	fprintf(fd, " %d;\n", numFace);
	// �}�e���A���ԍ��o��
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
	// material�o��
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
//		MQO�Z�[�u
//
//		�f�[�^��MQO�t�H�[�}�b�g�ŏo�͂��܂�
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
		wsprintf(strmsg, "�t�@�C�� %s�@���㏑�����܂����H",path);
		if( MessageBox(NULL, strmsg, "�Z�[�u�t�@�C���㏑��", MB_YESNO | MB_ICONQUESTION ) == IDNO ) return false;
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
//		Texture�o��
//
//		Texture�f�[�^��MQO�t�H�[�}�b�g�ŏo�͂��܂�
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
//		���b�V���J�E���g
//
//		�p�[�c�ԍ��������b�V���f�[�^���J�E���g����
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
//		���b�V���o��
//
//		���b�V���f�[�^��MQO�t�H�[�}�b�g�ŏo�͂��܂�
//======================================================================
bool CModel::outputMesh( FILE *fd )
{
	char objname[6][32]={
		"sdef:Face","sdef:Head","sdef:Body", "sdef:Hand", "sdef:Leg", "sdef:Foot" };
	if( fd==NULL ) return false;
//	Face�o��
	if( countParts( 1 )>0 ) {
		fprintf(fd,"Object \"%s\" {\n",objname[0]);
		fprintf(fd,"    visivle 15\n    locking 0\n    shading 1\n   facet 59.5\n    color 0.898 0.498 0.698\n   color_type 0\n");
		outputVerFace( fd, 1, 2 );
		fprintf(fd,"}\n");
	}
//	Head�o��
	if( countParts( 2 )>0 ) {
		fprintf(fd,"Object \"%s\" {\n",objname[1]);
		fprintf(fd,"    visivle 15\n    locking 0\n    shading 1\n   facet 59.5\n    color 0.898 0.498 0.698\n   color_type 0\n");
		outputVerFace( fd, 2, 2 );
		fprintf(fd,"}\n");
	}
//	Body�o��
	if( countParts( 3 )>0 ) {
		fprintf(fd,"Object \"%s\" {\n",objname[2]);
		fprintf(fd,"    visivle 15\n    locking 0\n    shading 1\n   facet 59.5\n    color 0.898 0.498 0.698\n   color_type 0\n");
		outputVerFace( fd, 3, 2 );
		fprintf(fd,"}\n");
	}
//	Hands�o��
	if( countParts( 4 )>0 ) {
		// �E��o��
		fprintf(fd,"Object \"%sR\" {\n",objname[3]);
		fprintf(fd,"    visivle 15\n    locking 0\n    shading 1\n   facet 59.5\n    color 0.898 0.498 0.698\n   color_type 0\n");
		outputVerFace( fd, 4, 0 );
		fprintf(fd,"}\n");
		// ����o��
		fprintf(fd,"Object \"%sL\" {\n",objname[3]);
		fprintf(fd,"    visivle 15\n    locking 0\n    shading 1\n   facet 59.5\n    color 0.898 0.498 0.698\n   color_type 0\n");
		outputVerFace( fd, 4, 1 );
		fprintf(fd,"}\n");
	}
//	Legs�o��
	if( countParts( 5 )>0 ) {
		fprintf(fd,"Object \"%s\" {\n",objname[4]);
		fprintf(fd,"    visivle 15\n    locking 0\n    shading 1\n   facet 59.5\n    color 0.898 0.498 0.698\n   color_type 0\n");
		outputVerFace( fd, 5, 2 );
		fprintf(fd,"}\n");
	}
//	foots�o��
	if( countParts( 6 )>0 ) {
		// �E��o��
		fprintf(fd,"Object \"%sR\" {\n",objname[5]);
		fprintf(fd,"    visivle 15\n    locking 0\n    shading 1\n   facet 59.5\n    color 0.898 0.498 0.698\n   color_type 0\n");
		outputVerFace( fd, 6, 0 );
		fprintf(fd,"}\n");
		// ����o��
		fprintf(fd,"Object \"%sL\" {\n",objname[5]);
		fprintf(fd,"    visivle 15\n    locking 0\n    shading 1\n   facet 59.5\n    color 0.898 0.498 0.698\n   color_type 0\n");
		outputVerFace( fd, 6, 1 );
		fprintf(fd,"}\n");
	}
	return true;
}

//======================================================================
//
//		Vertex�y��Face�o��
//
//		Vertex�y��Face��MQO�t�H�[�}�b�g�ŏo�͂��܂�
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
//		�R���X�g���N�^
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
//		�f�X�g���N�^
//
//======================================================================
CPC::~CPC()
{
}
//======================================================================
//
//		�{�[���̃J�E���g(�t�@�C�����L�����̔���j
//
//======================================================================
int CPC::CountBoneFromFile( char *FileName )
{
	//====================================================
	// �t�@�C�����������Ɏ�荞��
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
	// �{�[���̃J�E���g
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
//		�e�N�X�`���̃J�E���g(�t�@�C�����L�����̔���j
//
//======================================================================
int CPC::CountTextureFromFile( char *FileName )
{
	//====================================================
	// �t�@�C�����������Ɏ�荞��
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
	// �e�N�X�`���̃J�E���g
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
			SetType( Data );		// �^�C�v
			break;
		case 0x20:
			SetRace( Data );		// �{�[��
			break;
		case 0x30:
			SetFace( Data );		// ��
			break;
		case 0x40:
			SetHead( Data );		// ��
			break;
		case 0x50:
			SetBody( Data );		// ��
			break;
		case 0x60:
			SetHand( Data );		// ��
			break;
		case 0x70:
			SetLegs( Data );		// �r
			break;
		case 0x80:
			SetFoot( Data );		// ��
			break;
		case 0x90:
			SetRightWeapon( Data );		// �E�蕐��
			break;
		case 0xa0:
			SetLeftWeapon( Data );		// ���蕐��
			break;
		case 0xb0:
			SetRemoteWeapon( Data );		// ���u����
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
//		���f�������ǂݍ���
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
//		���f���ǂݍ���
//
//======================================================================
bool CPC::LoadPCParts()
{
	unsigned long FVF = (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1);
	HRESULT hr;
	char FileName[512];


	//==============================================================
	// �f�[�^������
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
	//  �t�@�C���̃��[�h
	//==============================================================
	// �{�[���̃��[�h
	GetFileNameFromDno(FileName,GetRace());
	if( CountBoneFromFile( FileName ) <=0 ) return false;
	hr = LoadBoneFromFile( FileName );


	// ��@�e�N�X�`���A���b�V���̃��[�h
	GetFileNameFromDno(FileName,GetFace());
	if( CountTextureFromFile( FileName ) > 0 ) {
		hr = LoadTextureFromFile( FileName );
		hr = LoadMeshFromFile( FileName, FVF, 1 );
	}
	// �h��@���@�e�N�X�`���A���b�V���̃��[�h
	GetFileNameFromDno(FileName,GetHead());
	if( CountTextureFromFile( FileName ) > 0 ) {
		hr = LoadTextureFromFile( FileName );
		hr = LoadMeshFromFile( FileName, FVF, 2 );
	}
	// �h��@���@�e�N�X�`���A���b�V���̃��[�h
	GetFileNameFromDno(FileName,GetBody());
	if( CountTextureFromFile( FileName ) > 0 ) {
		hr = LoadTextureFromFile( FileName );
		hr = LoadMeshFromFile( FileName, FVF, 3 );
		SetInfoBody( LoadInfoFromFile( FileName, 0x19) );
	}
	// �h��@��@�e�N�X�`���A���b�V���̃��[�h
	GetFileNameFromDno(FileName,GetHand());
	if( CountTextureFromFile( FileName ) > 0 ) {
		hr = LoadTextureFromFile( FileName );
		hr = LoadMeshFromFile( FileName, FVF, 4 );
	}
	// �h��@�r�@�e�N�X�`���A���b�V���̃��[�h
	GetFileNameFromDno(FileName,GetLegs());
	if( CountTextureFromFile( FileName ) > 0 ) {
		hr = LoadTextureFromFile( FileName );
		hr = LoadMeshFromFile( FileName, FVF, 5 );
	}
	// �h��@���@�e�N�X�`���A���b�V���̃��[�h
	GetFileNameFromDno(FileName,GetFoot());
	if( CountTextureFromFile( FileName ) > 0 ) {
		hr = LoadTextureFromFile( FileName );
		hr = LoadMeshFromFile( FileName, FVF, 6 );
	}
	//����@�E��@�@�e�N�X�`���A���b�V���̃��[�h
	GetFileNameFromDno(FileName,GetRightWeapon());
	if( CountTextureFromFile( FileName ) > 0 ) {
		hr = LoadTextureFromFile( FileName );
		hr = LoadMeshFromFile( FileName, FVF, 0 );
	}
	SetInfoRWeapon( LoadInfoFromFile( FileName , 0x13 ) );
	SetInfoRWeapon2( LoadInfoFromFile( FileName , 0x16 ) );
	//����@����@�@�e�N�X�`���A���b�V���̃��[�h
	GetFileNameFromDno(FileName,GetLeftWeapon());
	if( CountTextureFromFile( FileName ) > 0 ) {
		hr = LoadTextureFromFile( FileName );
		hr = LoadMeshFromFile( FileName, FVF, 0 );
	}
	SetInfoLWeapon( LoadInfoFromFile( FileName , 0x15 ) );
	SetInfoLWeapon2( LoadInfoFromFile( FileName , 0x13 ) );
	//���u�@�@�@�e�N�X�`���A���b�V���̃��[�h
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
//		���f���ǂݍ���
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
//		�R���X�g���N�^
//
//======================================================================
CNPC::CNPC()
{
	m_mBody				= 0;
}

//======================================================================
//
//		�f�X�g���N�^
//
//======================================================================
CNPC::~CNPC()
{
}

//======================================================================
//
//		�{�[���̃J�E���g(�t�@�C�����L�����̔���j
//
//======================================================================
int CNPC::CountBoneFromFile( char *FileName )
{
	//====================================================
	// �t�@�C�����������Ɏ�荞��
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
	// �{�[���̃J�E���g
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
//		���[�V�����ǂݍ���
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
//		NPC�f�[�^�ǂݍ���
//
//======================================================================
bool CNPC::LoadNPC()
{
	unsigned long FVF = (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1);
	unsigned long FVF1 = (D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_DIFFUSE|D3DFVF_TEX1);
	unsigned long FVF2 = (D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1);
	char FileName[512];

	//==============================================================
	// �f�[�^������
	//==============================================================
	SetTime( 0.f );
	InitPartsData();
	InitMotionData();
	if( (GetBody()&0xffff) == 0 ) return false;

	//==============================================================
	//  �t�@�C���̃��[�h
	//==============================================================
	GetFileNameFromDno(FileName,GetBody());
	if( CountBoneFromFile( FileName ) <= 0 ) return false;
	//==============================================================
	//  �t�@�C���̃��[�h
	//==============================================================
	HRESULT hr;
	// �{�[���̃��[�h
	hr = LoadBoneFromFile( FileName );
	if( hr ) return false;
	//�@�e�N�X�`���A���b�V���̃��[�h
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




