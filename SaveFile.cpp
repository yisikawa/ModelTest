#define _CRT_SECURE_NO_WARNINGS
#include "Dx.h"
#include "Model.h"

#include <iostream>
#include <sstream>
#include <stdio.h>
#include <string>
#include <vector>
#include <unordered_map>

using namespace std;
using namespace fbxsdk;

#define SAFE_RELEASE(p)		if ( (p) != NULL ) { (p)->Release(); (p) = NULL; }
#define SAFE_DELETES(p)		if ( (p) != NULL ) { delete [] (p); (p) = NULL; }
#define SAFE_DELETE(p)		if ( (p) != NULL ) { delete (p); (p) = NULL; }
#define PAI					(3.1415926535897932384626433832795f)
#define PAI2				(3.1415926535897932384626433832795f*2.0f)
// グローバル
extern	CPC* pPC;
extern	CNPC* pNPC;
extern	bool		g_mPCFlag;
// 検索対象文字列
int Trim(char* s);
char* strrstr(const char* string, const char* pattern);// 検索対象文字列を指定する文字列で検索する
char* strcpynosp(char* string1, char* string2);//空白を除いて文字列をコピーする

 //======================================================================
//		FBXファイルセーブ
//		データをFBXフォーマットで出力します
//======================================================================
void CModel::OptimizeVertices(void) {
	int numVer = totalVertex();
	if (m_vertexRemap.size() == numVer && m_weldedVertices.size() > 0) {
		return;
	}
	m_vertexRemap.assign(numVer, -1);
	m_weldedVertices.clear();

	std::unordered_map<long long, std::vector<int>> spatialHash;
	const float GRID_SIZE = 0.01f;
	const float THRESHOLD_SQ = 0.0001f * 0.0001f;

	// === Phase 1: 頂点の結合（法線はゼロで初期化） ===
	unsigned int originalNo = 0;
	CMesh* pMesh = (CMesh*)m_Meshs.Top();
	while (pMesh != NULL) {
		CUSTOMVERTEX* pV1, * pV2;
		(pMesh->m_lpVB1)->Lock(0, pMesh->m_VBSize, (void**)&pV1, D3DLOCK_DISCARD);
		(pMesh->m_lpVB2)->Lock(0, pMesh->m_VBSize, (void**)&pV2, D3DLOCK_DISCARD);
		for (unsigned int i = 0; i < pMesh->m_NumVertices; i++, pV1++, pV2++, originalNo++) {
			D3DXVECTOR3 vert(pV1->p.x, pV1->p.y, pV1->p.z);

			if (fabs(vert.x) > 10000.0 || fabs(vert.y) > 10000.0 || fabs(vert.z) > 10000.0) {
				vert.x = 0; vert.y = 0; vert.z = 0;
				pV1->u = 0; pV1->v = 0;
			}

			float u = (pV1->u < 0.0f) ? 0.0f : (pV1->u > 1.0f ? 1.0f : pV1->u);
			float v = (pV1->v < 0.0f) ? 0.0f : (pV1->v > 1.0f ? 1.0f : pV1->v);

			float b1_1 = pV1->b1;
			int global_bone_1 = (b1_1 > 0.0f) ? pMesh->m_pBoneTbl[pV1->indx] : -1;
			float b1_2 = pV2->b1;
			int global_bone_2 = (b1_2 > 0.0f) ? pMesh->m_pBoneTbl[pV2->indx] : -1;

			long long hx = (long long)std::floor(vert.x / GRID_SIZE);
			long long hy = (long long)std::floor(vert.y / GRID_SIZE);
			long long hz = (long long)std::floor(vert.z / GRID_SIZE);

			int foundIndex = -1;
			// 隣接セルも検索して境界付近の頂点の結合漏れを防止
			for (long long dx = -1; dx <= 1 && foundIndex == -1; dx++) {
				for (long long dy = -1; dy <= 1 && foundIndex == -1; dy++) {
					for (long long dz = -1; dz <= 1 && foundIndex == -1; dz++) {
						long long hashKey = ((hx + dx) * 73856093LL) ^ ((hy + dy) * 19349663LL) ^ ((hz + dz) * 83492791LL);
						if (spatialHash.count(hashKey)) {
							for (int idx : spatialHash[hashKey]) {
								const WELDED_VERTEX& uvtx = m_weldedVertices[idx];
								// 位置・UV・ウェイトのみで判定（法線判定なし）
								if (D3DXVec3LengthSq(&(vert - uvtx.p)) <= THRESHOLD_SQ) {
									if (fabs(u - uvtx.u) <= 0.0001f && fabs(v - uvtx.v) <= 0.0001f) {
										D3DXVECTOR3 norm(pV1->n.x, pV1->n.y, pV1->n.z);
										if (fabs(b1_1 - uvtx.weights[0]) <= 0.0001f && fabs(b1_2 - uvtx.weights[1]) <= 0.0001f &&
											global_bone_1 == uvtx.globalBones[0] && global_bone_2 == uvtx.globalBones[1]) {
											if (D3DXVec3LengthSq(&(norm - uvtx.n)) <= 0.0001f) {
												foundIndex = idx;
												break;
											}
										}
									}
								}
							}
						}
					}
				}
			}

			if (foundIndex == -1) {
				foundIndex = (int)m_weldedVertices.size();
				WELDED_VERTEX newVtx;
				newVtx.p = vert;
				newVtx.n = D3DXVECTOR3(0, 0, 0); // 法線はゼロで初期化（Phase 2で計算）
				newVtx.u = u;
				newVtx.v = v;
				newVtx.weights[0] = b1_1;
				newVtx.weights[1] = b1_2;
				newVtx.globalBones[0] = global_bone_1;
				newVtx.globalBones[1] = global_bone_2;
				newVtx.originalIndex = originalNo;
				m_weldedVertices.push_back(newVtx);

				long long hashKey = (hx * 73856093LL) ^ (hy * 19349663LL) ^ (hz * 83492791LL);
				spatialHash[hashKey].push_back(foundIndex);
			}
			m_vertexRemap[originalNo] = foundIndex;
		}
		(pMesh->m_lpVB1)->Unlock();
		(pMesh->m_lpVB2)->Unlock();
		pMesh = (CMesh*)pMesh->Next;
	}

	// === Phase 2: 面のジオメトリから法線を計算して各頂点に加算 ===
	// Blenderの「面から法線を計算」と同等の処理
	int vCnt = 0;
	pMesh = (CMesh*)m_Meshs.Top();
	while (pMesh != NULL) {
		WORD* pI, * pIndex;
		int i1, i2, i3, t1, t2, t3;
		int dispCheck = pMesh->GetDispCheck();
		(pMesh->m_lpIB)->Lock(0, pMesh->m_IBSize, (void**)&pIndex, D3DLOCK_DISCARD);
		CStream* pStream = (CStream*)pMesh->m_Streams.Top();
		while (pStream != NULL) {
			int dispLevel = pStream->GetDispLevel();
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
						if (pMesh->m_FlipFlag) { t1 = i3; t2 = i2; t3 = i1; }
						else { t1 = i1; t2 = i2; t3 = i3; }
					} else {
						if (pMesh->m_FlipFlag) { t1 = i1; t2 = i2; t3 = i3; }
						else { t1 = i3; t2 = i2; t3 = i1; }
					}
					int w1 = m_vertexRemap[t1 + vCnt];
					int w2 = m_vertexRemap[t2 + vCnt];
					int w3 = m_vertexRemap[t3 + vCnt];
					// 退化三角形をスキップ
					if (w1 != w2 && w2 != w3 && w1 != w3) {
						D3DXVECTOR3 edge1 = m_weldedVertices[w2].p - m_weldedVertices[w1].p;
						D3DXVECTOR3 edge2 = m_weldedVertices[w3].p - m_weldedVertices[w1].p;
						D3DXVECTOR3 faceNormal;
						D3DXVec3Cross(&faceNormal, &edge1, &edge2);
						// 面法線を各頂点に加算
						m_weldedVertices[w1].n += faceNormal;
						m_weldedVertices[w2].n += faceNormal;
						m_weldedVertices[w3].n += faceNormal;
					}
					i1 = i2; i2 = i3;
				}
			} else if (pStream->m_PrimitiveType == D3DPT_TRIANGLELIST) {
				for (unsigned int i = 0; i < pStream->GetFaceCount(); i++) {
					i1 = *pI++; i2 = *pI++; i3 = *pI++;
					if (pMesh->m_FlipFlag) { t1 = i1; t2 = i2; t3 = i3; }
					else { t1 = i3; t2 = i2; t3 = i1; }
					int w1 = m_vertexRemap[t1 + vCnt];
					int w2 = m_vertexRemap[t2 + vCnt];
					int w3 = m_vertexRemap[t3 + vCnt];
					if (w1 != w2 && w2 != w3 && w1 != w3) {
						D3DXVECTOR3 edge1 = m_weldedVertices[w2].p - m_weldedVertices[w1].p;
						D3DXVECTOR3 edge2 = m_weldedVertices[w3].p - m_weldedVertices[w1].p;
						D3DXVECTOR3 faceNormal;
						D3DXVec3Cross(&faceNormal, &edge1, &edge2);
						m_weldedVertices[w1].n += faceNormal;
						m_weldedVertices[w2].n += faceNormal;
						m_weldedVertices[w3].n += faceNormal;
					}
				}
			}
			pStream = (CStream*)pStream->Next;
		}
		(pMesh->m_lpIB)->Unlock();
		vCnt += pMesh->m_NumVertices;
		pMesh = (CMesh*)pMesh->Next;
	}

	// === Phase 3: 加算された法線を正規化して平均法線を得る ===
	for (size_t i = 0; i < m_weldedVertices.size(); i++) {
		D3DXVec3Normalize(&m_weldedVertices[i].n, &m_weldedVertices[i].n);
	}
}

bool CModel::outputFBXVertex(FbxMesh * pfbxMesh)
{
	D3DXMATRIX rootMatrix, mat;

	D3DXMatrixRotationY(&mat, (float)(PAI / 2.));
	D3DXMatrixRotationX(&rootMatrix, (float)PAI);
	rootMatrix *= mat;

	FbxGeometryElementNormal* normalElement = pfbxMesh->CreateElementNormal();
	normalElement->SetMappingMode(FbxGeometryElement::eByControlPoint); // 頂点ごとに法線を設定
	normalElement->SetReferenceMode(FbxGeometryElement::eDirect); // 法線を直接指定
	FbxGeometryElementUV* uvElement = pfbxMesh->CreateElementUV("UVSet");
	uvElement->SetMappingMode(FbxGeometryElement::eByControlPoint); // 頂点ごとにUVを設定
	uvElement->SetReferenceMode(FbxGeometryElement::eDirect); // UVを直接指定

	OptimizeVertices();

	pfbxMesh->InitControlPoints((int)m_weldedVertices.size());
	for (size_t i = 0; i < m_weldedVertices.size(); i++) {
		D3DXVECTOR3 vert = m_weldedVertices[i].p;
		D3DXVec3TransformCoord(&vert, &vert, &rootMatrix);

		D3DXVECTOR3 norm = m_weldedVertices[i].n;
		D3DXVec3TransformNormal(&norm, &norm, &rootMatrix);

		pfbxMesh->SetControlPointAt(FbxVector4(vert.x, vert.y, vert.z), (int)i);
		normalElement->GetDirectArray().Add(FbxVector4(norm.x, norm.y, norm.z));
		uvElement->GetDirectArray().Add(FbxVector2(m_weldedVertices[i].u, 1.0 - m_weldedVertices[i].v));
	}
	return true;
}
bool CModel::outputFBXFace(FbxMesh* pfbxMesh, FbxLayerElementMaterial* pMaterialElement)
{

	int numVer = totalVertex();
	int numFace = totalFace();
	// パッチ出力
	int vCnt = 0, fCnt = 0;
	CMesh* pMesh = (CMesh*)m_Meshs.Top();
	while (pMesh != NULL) {
		WORD* pI, * pIndex;
		int				i1, i2, i3, t1, t2, t3;
		CUSTOMVERTEX* pV;
		int dispCheck = pMesh->GetDispCheck();
		(pMesh->m_lpIB)->Lock(0, pMesh->m_IBSize, (void**)&pIndex, D3DLOCK_DISCARD);
		(pMesh->m_lpVB1)->Lock(0, pMesh->m_VBSize, (void**)&pV, D3DLOCK_DISCARD);
		CStream* pStream = (CStream*)pMesh->m_Streams.Top();
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
					pfbxMesh->BeginPolygon();
					pfbxMesh->AddPolygon(m_vertexRemap[t1 + vCnt]);
					pfbxMesh->AddPolygon(m_vertexRemap[t2 + vCnt]);
					pfbxMesh->AddPolygon(m_vertexRemap[t3 + vCnt]);
					pfbxMesh->EndPolygon();
					pMaterialElement->GetIndexArray().Add(pStream->m_texNo); // materials[texNo] を割り当てる
					//fprintf(fd, "  3;%d,%d,%d;", t1 + vCnt, t2 + vCnt, t3 + vCnt);
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
					pfbxMesh->BeginPolygon();
					pfbxMesh->AddPolygon(m_vertexRemap[t1 + vCnt]);
					pfbxMesh->AddPolygon(m_vertexRemap[t2 + vCnt]);
					pfbxMesh->AddPolygon(m_vertexRemap[t3 + vCnt]);
					pfbxMesh->EndPolygon();
					pMaterialElement->GetIndexArray().Add(pStream->m_texNo); // materials[texNo] を割り当てる
					// fprintf(fd, "  3;%d,%d,%d;", t1 + vCnt, t2 + vCnt, t3 + vCnt);
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
	return true;

}

FbxAMatrix D3DXM2FbxAM(const D3DXMATRIX& d3dxMatrix)
{
	FbxAMatrix fbxMatrix;

	// D3DXMatrix は row-major (行優先)、FbxAMatrix は column-major (列優先)
	// データ型も D3DXMATRIX は float, FbxAMatrix は double

	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			fbxMatrix.mData[i][j] = (double)d3dxMatrix.m[j][i]; // 転置しながらコピー
		}
	}

	return fbxMatrix;
}

D3DXVECTOR3 D3DXMat2Trans(const D3DXMATRIX& d3dxMatrix)
{
	D3DXVECTOR3 translation(0.,0.,0.);
	// 平行移動成分の抽出
	translation.x = d3dxMatrix.m[3][0];
	translation.y = d3dxMatrix.m[3][1];
	translation.z = d3dxMatrix.m[3][2];
	return translation;
}
D3DXVECTOR3 D3DXMat2Scale(const D3DXMATRIX& d3dxMatrix)
{
	float sx = sqrt(d3dxMatrix._11 * d3dxMatrix._11 + d3dxMatrix._12 * d3dxMatrix._12 + d3dxMatrix._13 * d3dxMatrix._13);
	float sy = sqrt(d3dxMatrix._21 * d3dxMatrix._21 + d3dxMatrix._22 * d3dxMatrix._22 + d3dxMatrix._23 * d3dxMatrix._23);
	float sz = sqrt(d3dxMatrix._31 * d3dxMatrix._31 + d3dxMatrix._32 * d3dxMatrix._32 + d3dxMatrix._33 * d3dxMatrix._33);
	return D3DXVECTOR3(sx, sy, sz);
}

D3DXVECTOR3 D3DXMat2Euler(const D3DXMATRIX& mat)
{
	D3DXVECTOR3 x(mat._11, mat._12, mat._13);
	D3DXVECTOR3 y(mat._21, mat._22, mat._23);
	D3DXVECTOR3 z(mat._31, mat._32, mat._33);
	D3DXVec3Normalize(&x, &x);
	D3DXVec3Normalize(&y, &y);
	D3DXVec3Normalize(&z, &z);

	D3DXMATRIX rot;
	D3DXMatrixIdentity(&rot);
	rot._11 = x.x; rot._12 = x.y; rot._13 = x.z;
	rot._21 = y.x; rot._22 = y.y; rot._23 = y.z;
	rot._31 = z.x; rot._32 = z.y; rot._33 = z.z;

	D3DXQUATERNION q;
	D3DXQuaternionRotationMatrix(&q, &rot);
	D3DXQuaternionNormalize(&q, &q);

	fbxsdk::FbxAMatrix fbxRot;
	fbxRot.SetQ(fbxsdk::FbxQuaternion(q.x, q.y, q.z, q.w));
	fbxsdk::FbxVector4 fbxEuler = fbxRot.GetR();
	
	return D3DXVECTOR3((float)fbxEuler[0], (float)fbxEuler[1], (float)fbxEuler[2]);
}




bool CModel::outputFBXBone(FbxNode* pRootNode,FbxScene* pScene,FbxMesh* pMesh)
{

	D3DXQUATERNION q(0., 0., 0., 1.);
	FbxQuaternion qq(0., 0., 0., 1.);
	FbxAMatrix fMat,rMat;
	FbxMatrix aMat;	aMat.SetIdentity();
	D3DXVECTOR3 t, s, r;
	//FbxNode* pMeshNode = pRootNode->GetChild(0);
	// バインドポーズ
	FbxPose* bindPose = FbxPose::Create(pScene, "BindPose");
	bindPose->SetIsBindPose(true);

	std::vector<FbxNode*> BoneArrys;
	// スキニングの設定
	FbxSkin* pSkin = FbxSkin::Create(pScene, "SkinDeformer");
	pMesh->AddDeformer(pSkin);
	// ルートボーンの作成
	FbxNode* pRBoneNode = FbxNode::Create(pScene, "Scene_Root"); 
	FbxSkeleton* pRBoneAttr = FbxSkeleton::Create(pScene, "RootBoneSkel");
	pRBoneAttr->SetSkeletonType(FbxSkeleton::eLimbNode); // ルートボーンタイプを設定
	//pRBoneAttr->SetSkeletonType(FbxSkeleton::eRoot); // ルートボーンタイプを設定
	pRBoneNode->SetNodeAttribute(pRBoneAttr);
	pRBoneNode->LclRotation.Set(FbxDouble3(-180., -90., 0.) );
	//pRBoneNode->LclScaling.Set(FbxDouble3(100., 100., 100.));
	pRootNode->AddChild(pRBoneNode);
	bindPose->Add(pRBoneNode, aMat);
	//bindPose->Add(pRBoneNode, pRBoneNode->EvaluateGlobalTransform());
	rMat = pRBoneNode->EvaluateLocalTransform();
	for (int i = 0; i < m_nBone; i++) {
		sprintf(m_Bones[i].m_Name,"Bone%03d", i);
		FbxNode* pCBoneNode = FbxNode::Create(pScene,m_Bones[i].m_Name); BoneArrys.push_back(pCBoneNode);
		FbxSkeleton* pCBoneAttribute = FbxSkeleton::Create(pScene, (string(m_Bones[i].m_Name)+"_Skel").c_str());
		pCBoneAttribute->SetSkeletonType(FbxSkeleton::eLimbNode); // リムノードタイプを設定
		pCBoneNode->SetNodeAttribute(pCBoneAttribute);
		pCBoneNode->RotationOrder.Set(fbxsdk::FbxEuler::eEulerXYZ);
		pCBoneNode->RotationOrder.Set(fbxsdk::FbxEuler::eEulerXYZ);
		t = D3DXMat2Trans(m_Bones[i].m_mTransform); s = D3DXMat2Scale(m_Bones[i].m_mTransform); r = D3DXMat2Euler(m_Bones[i].m_mTransform);
		pCBoneNode->LclTranslation.Set(FbxVector4(t.x, t.y,t.z));
		pCBoneNode->LclScaling.Set(FbxVector4(s.x, s.y, s.z));
		pCBoneNode->LclRotation.Set(FbxVector4(r.x, r.y, r.z));
		if (m_Bones[i].m_mParent >= 0) {
			(i == 0) ? pRBoneNode->AddChild(pCBoneNode) : BoneArrys[m_Bones[i].m_mParent]->AddChild(pCBoneNode);
		}
		bindPose->Add(pCBoneNode, pCBoneNode->EvaluateGlobalTransform());
		// 子ボーンのクラスタ
		if (countBone2Ver(i) <= 0) continue;
		t = D3DXMat2Trans(m_Bones[i].m_mInvTrans); s = D3DXMat2Scale(m_Bones[i].m_mInvTrans);
		D3DXQuaternionRotationMatrix(&q, &m_Bones[i].m_mInvTrans); qq.Set(q.x, q.y, q.z, q.w);
		fMat.SetTQS(FbxVector4(t.x, t.y, t.z), qq, FbxVector4(s.x, s.y, s.z));
		FbxCluster* pCBoneCluster = FbxCluster::Create(pScene, (string(m_Bones[i].m_Name) + "_Skin").c_str());
		pCBoneCluster->SetLink(pCBoneNode);
		pCBoneCluster->SetLinkMode(FbxCluster::eTotalOne);
		SetFBXBone2VerNo(pCBoneCluster,i);
		pCBoneCluster->SetTransformMatrix(rMat.Inverse() * pCBoneNode->EvaluateGlobalTransform() * fMat);
		pCBoneCluster->SetTransformLinkMatrix(pCBoneNode->EvaluateGlobalTransform());
		pSkin->AddCluster(pCBoneCluster);
	}
	pScene->AddPose(bindPose);
	return true;
}

bool CModel::SetFBXBone2VerNo(FbxCluster* pCBCluster, int boneNo) {
	int indNum = 0;
	int vCnt = 0;
	CMesh* pMesh = (CMesh*)m_Meshs.Top();
	std::vector<bool> addedMap(m_vertexRemap.size(), false);
	while (pMesh != NULL) {
		CUSTOMVERTEX* pV1, * pV2;
		(pMesh->m_lpVB1)->Lock(0, pMesh->m_VBSize, (void**)&pV1, D3DLOCK_DISCARD);
		(pMesh->m_lpVB2)->Lock(0, pMesh->m_VBSize, (void**)&pV2, D3DLOCK_DISCARD);
		for (unsigned int i = 0; i < pMesh->m_NumVertices; i++, pV1++, pV2++) {
			int uniqueInd = m_vertexRemap[i + vCnt];
			if (addedMap[uniqueInd]) continue;

			if (pV1->b1 > 0.f && pMesh->m_pBoneTbl[pV1->indx] == boneNo) {
				(pV1->b1 < 0.0) ? pV1->b1 = 0.: pV1->b1 = pV1->b1;
				(pV1->b1 > 1.0) ? pV1->b1 = 1.: pV1->b1 = pV1->b1;
				pCBCluster->AddControlPointIndex(uniqueInd, pV1->b1);
				addedMap[uniqueInd] = true;
			}
			else if (pV2->b1 > 0.f && pMesh->m_pBoneTbl[pV2->indx] == boneNo) {
				(pV2->b1 < 0.0) ? pV2->b1 = 0. : pV2->b1 = pV2->b1;
				(pV2->b1 > 1.0) ? pV2->b1 = 1. : pV2->b1 = pV2->b1;
				pCBCluster->AddControlPointIndex(uniqueInd, pV2->b1);
				addedMap[uniqueInd] = true;
			}
		}
		(pMesh->m_lpVB1)->Unlock();
		(pMesh->m_lpVB2)->Unlock();
		vCnt += pMesh->m_NumVertices;
		pMesh = (CMesh*)pMesh->Next;
	}
	return true;
}

bool CModel::outputFBXAnimation(FbxScene* pScene) 
{
	D3DXQUATERNION q(0., 0., 0., 1.);
	FbxQuaternion qq(0., 0., 0., 1.);
	FbxAMatrix fMat;
	D3DXVECTOR3 t, s, r;

	// アニメーションセット出力
	string motionName;

	if (g_mPCFlag) {
		motionName = string(pPC->GetMotionName());
	}
	else {
		motionName = string(pNPC->GetMotionName());
	}
	FbxAnimStack* pAnimStack = FbxAnimStack::Create(pScene, motionName.c_str());
	FbxAnimLayer* pAnimLayer = FbxAnimLayer::Create(pScene, motionName.c_str());
	pAnimStack->AddMember(pAnimLayer);
    FbxTime fbxStart, fbxEnd;
    fbxStart.SetSecondDouble(0.0);
    fbxEnd.SetSecondDouble(0.0);

	// ノード名→FbxNodeのマップを事前構築（O(N2)→O(N)に改善）
	std::unordered_map<std::string, FbxNode*> nodeMap;
	for (int j = 0; j < pScene->GetNodeCount(); j++) {
		FbxNode* pN = pScene->GetNode(j);
		if (pN) {
			nodeMap[std::string(pN->GetName())] = pN;
		}
	}

	FbxNode* pNode;
	for (int i = 0; i < m_nBone; i++) {
		// マップによるO(1)検索
		auto it = nodeMap.find(std::string(m_Bones[i].m_Name));
		if (it == nodeMap.end()) continue;	// 見つからなければスキップ
		pNode = it->second;

		// 位置アニメーションカーブ
		FbxAnimCurve* curveTX = pNode->LclTranslation.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_X, true);
		FbxAnimCurve* curveTY = pNode->LclTranslation.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_Y, true);
		FbxAnimCurve* curveTZ = pNode->LclTranslation.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_Z, true);
		// 回転アニメーションカーブ
		FbxAnimCurve* curveRX = pNode->LclRotation.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_X, true);
		FbxAnimCurve* curveRY = pNode->LclRotation.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_Y, true);
		FbxAnimCurve* curveRZ = pNode->LclRotation.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_Z, true);
		// スケールアニメーションカーブ
		FbxAnimCurve* curveSX = pNode->LclScaling.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_X, true);
		FbxAnimCurve* curveSY = pNode->LclScaling.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_Y, true);
		FbxAnimCurve* curveSZ = pNode->LclScaling.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_Z, true);

		int tboneNo = i;
		int keyNum = m_MotionArray[tboneNo].m_RotationKeyNum;
		int tkeyNum = keyNum;
		pNode->RotationOrder.Set(fbxsdk::FbxEuler::eEulerXYZ);
		// キーフレーム数0のフォールバックに安全チェックを追加
		if (keyNum <= 0) {
			tboneNo = 0;
			tkeyNum = m_MotionArray[tboneNo].m_RotationKeyNum;
			// ボーン0にもキーがない、またはキーデータがNULLの場合はスキップ
			if (tkeyNum <= 0 || m_MotionArray[tboneNo].m_pTranslateKeys == NULL) {
				continue;
			}
		}

		// KeyModifyBegin はループの外で1回だけ呼ぶ
		curveTX->KeyModifyBegin(); curveTY->KeyModifyBegin(); curveTZ->KeyModifyBegin();
		curveRX->KeyModifyBegin(); curveRY->KeyModifyBegin(); curveRZ->KeyModifyBegin();
		curveSX->KeyModifyBegin(); curveSY->KeyModifyBegin(); curveSZ->KeyModifyBegin();

		// アニメーションの前の値をバインドポーズの回転で初期化
		D3DXVECTOR3 bindR = D3DXMat2Euler(m_Bones[i].m_mTransform);
		FbxVector4 lastEuler(bindR.x, bindR.y, bindR.z, 0.0); // 【修正】直前のオイラー角を保存

		for (int k = 0; k < tkeyNum; k++) {
			FbxTime fbxTime;
			fbxTime.SetSecondDouble(m_MotionArray[tboneNo].m_pTranslateKeys[k].Time / 3000.); // 時間を秒単位で設定
                    if (fbxTime > fbxEnd) fbxEnd = fbxTime;
			D3DXMATRIX iMatrix;
			if (keyNum > 0) {
				// GetAnimationMatrix のNULLチェック
				D3DXMATRIX* pAnimMat = m_MotionArray[tboneNo].GetAnimationMatrix(k, &m_Bones[tboneNo].m_mTransform);
				if (pAnimMat != NULL) {
					iMatrix = m_Bones[tboneNo].m_mTransform;
					iMatrix *= *pAnimMat;
				} else {
					// アニメーション行列が取得できない場合はバインドポーズをそのまま使用
					iMatrix = m_Bones[tboneNo].m_mTransform;
				}
			} else {
				iMatrix = m_Bones[i].m_mTransform;
			}
			t = D3DXMat2Trans(iMatrix); s = D3DXMat2Scale(iMatrix);
			
			// 回転成分抽出のための正規化
			D3DXMATRIX matNorm = iMatrix;
			D3DXVec3Normalize((D3DXVECTOR3*)&matNorm.m[0][0], (D3DXVECTOR3*)&matNorm.m[0][0]);
			D3DXVec3Normalize((D3DXVECTOR3*)&matNorm.m[1][0], (D3DXVECTOR3*)&matNorm.m[1][0]);
			D3DXVec3Normalize((D3DXVECTOR3*)&matNorm.m[2][0], (D3DXVECTOR3*)&matNorm.m[2][0]);
			D3DXQuaternionRotationMatrix(&q, &matNorm); 
			
			qq.Set(q.x, q.y, q.z, q.w);
			fMat.SetTQS(FbxVector4(t.x, t.y, t.z), qq, FbxVector4(s.x, s.y, s.z));

			// KeyInsert の戻り値をインデックスとして使用
			// 位置キーフレーム設定
			FbxVector4 val = fMat.GetT();
			int idxTX = curveTX->KeyInsert(fbxTime);
			curveTX->KeySetValue(idxTX, (float)val[0]);
			curveTX->KeySetInterpolation(idxTX, FbxAnimCurveDef::eInterpolationLinear);
			int idxTY = curveTY->KeyInsert(fbxTime);
			curveTY->KeySetValue(idxTY, (float)val[1]);
			curveTY->KeySetInterpolation(idxTY, FbxAnimCurveDef::eInterpolationLinear);
			int idxTZ = curveTZ->KeyInsert(fbxTime);
			curveTZ->KeySetValue(idxTZ, (float)val[2]);
			curveTZ->KeySetInterpolation(idxTZ, FbxAnimCurveDef::eInterpolationLinear);

			// 回転キーフレーム設定
			D3DXVECTOR3 angst = D3DXMat2Euler(iMatrix);
			val.Set(angst.x, angst.y, angst.z, 0.0);
			{
				FbxVector4 v1 = val;
				FbxVector4 v2(v1[0] + 180.0, 180.0 - v1[1], v1[2] + 180.0, 0.0);
				for (int j = 0; j < 3; j++) {
					while (v1[j] - lastEuler[j] > 180.01) v1[j] -= 360.0;
					while (v1[j] - lastEuler[j] < -180.01) v1[j] += 360.0;
					while (v2[j] - lastEuler[j] > 180.01) v2[j] -= 360.0;
					while (v2[j] - lastEuler[j] < -180.01) v2[j] += 360.0;
				}
				double d1 = fabs(v1[0] - lastEuler[0]) + fabs(v1[1] - lastEuler[1]) + fabs(v1[2] - lastEuler[2]);
				double d2 = fabs(v2[0] - lastEuler[0]) + fabs(v2[1] - lastEuler[1]) + fabs(v2[2] - lastEuler[2]);
				if (d2 < d1) val = v2;
				else val = v1;
				//for (int axis = 0; axis < 3; axis++) {
				//	double diff = val[axis] - lastEuler[axis];
				//	if (fabs(diff) > 45.0) {
				//		char buf[512];
				//		sprintf_s(buf, "FBX_ANIM_CHECK: Bone[%03d](%s) Frame[%d] Axis[%d] Jump: %f -> %f (Delta: %f)\n", tboneNo, m_Bones[tboneNo].m_Name, k, axis, lastEuler[axis], val[axis], diff);
				//		OutputDebugStringA(buf);
				//	}
				//}
			}
			lastEuler = val;

			int idxRX = curveRX->KeyInsert(fbxTime);
			curveRX->KeySetValue(idxRX, (float)val[0]);
			curveRX->KeySetInterpolation(idxRX, FbxAnimCurveDef::eInterpolationLinear);
			int idxRY = curveRY->KeyInsert(fbxTime);
			curveRY->KeySetValue(idxRY, (float)val[1]);
			curveRY->KeySetInterpolation(idxRY, FbxAnimCurveDef::eInterpolationLinear);
			int idxRZ = curveRZ->KeyInsert(fbxTime);
			curveRZ->KeySetValue(idxRZ, (float)val[2]);
			curveRZ->KeySetInterpolation(idxRZ, FbxAnimCurveDef::eInterpolationLinear);

			// スケールキーフレーム設定（補間設定も有効化して統一）
			val = fMat.GetS();
			int idxSX = curveSX->KeyInsert(fbxTime);
			curveSX->KeySetValue(idxSX, (float)val[0]);
			curveSX->KeySetInterpolation(idxSX, FbxAnimCurveDef::eInterpolationLinear);
			int idxSY = curveSY->KeyInsert(fbxTime);
			curveSY->KeySetValue(idxSY, (float)val[1]);
			curveSY->KeySetInterpolation(idxSY, FbxAnimCurveDef::eInterpolationLinear);
			int idxSZ = curveSZ->KeyInsert(fbxTime);
			curveSZ->KeySetValue(idxSZ, (float)val[2]);
			curveSZ->KeySetInterpolation(idxSZ, FbxAnimCurveDef::eInterpolationLinear);
		}

		// KeyModifyEnd はループの外で1回だけ呼ぶ
		curveTX->KeyModifyEnd(); curveTY->KeyModifyEnd(); curveTZ->KeyModifyEnd();
		curveRX->KeyModifyEnd(); curveRY->KeyModifyEnd(); curveRZ->KeyModifyEnd();
		curveSX->KeyModifyEnd(); curveSY->KeyModifyEnd(); curveSZ->KeyModifyEnd();

		FbxAnimCurve* pRotationCurves[3] = { curveRX, curveRY, curveRZ };
		FbxAnimCurveFilterUnroll unrollFilter;
		unrollFilter.Apply(pRotationCurves, 3);
	}

    pAnimStack->LocalStart.Set(fbxStart);
    pAnimStack->LocalStop.Set(fbxEnd);
	return true;
}

bool CModel::saveFBX(char* FPath, char* FName)
{
	D3DXVECTOR3 pIn, pOut;
	char* ptr, fpath[256], texpath[256], texName[256];

	if ((ptr = strstr(FPath, ".fbx"))) *ptr = '\0';
	if ((ptr = strstr(FName, ".fbx"))) *ptr = '\0';

	// FBXマネージャーの作成
	FbxManager* fbxManager = FbxManager::Create();
	if (!fbxManager) {
		FBXSDK_printf("Error: FbxManager creation failed.\n");
		return false;
	}

	// IOSettingsの作成
	FbxIOSettings* ios = FbxIOSettings::Create(fbxManager, IOSROOT);
	fbxManager->SetIOSettings(ios);

	// FBXシーンの作成
	FbxScene* fbxScene = FbxScene::Create(fbxManager, "");
	if (!fbxScene) {
		FBXSDK_printf("Error: FbxScene creation failed.\n");
		fbxManager->Destroy();
		return false;
	}
	// Coordinate conversion (DirectX LHS to FBX RHS)
	//FbxAxisSystem(FbxAxisSystem::eDirectX).ConvertScene(fbxScene);
	// GlobalSettingsを取得
	FbxGlobalSettings& globalSettings = fbxScene->GetGlobalSettings();
	FbxSystemUnit systemUnit(100.);
	globalSettings.SetSystemUnit(systemUnit);
	FbxTime::EMode newTimeMode = FbxTime::eFrames30;
	globalSettings.SetTimeMode(newTimeMode);

	//SceneInfoを取得
	FbxDocumentInfo* sceneInfo = fbxScene->GetSceneInfo();
	sceneInfo->mTitle.Append("ewhM:Scene",strlen("ewhM:Scene"));
	sceneInfo->mKeywords.Append("ewhModelViewer", strlen("ewhModelViewer"));
	sceneInfo->mAuthor.Append("ewhM dev",strlen("ewhM dev"));
	sceneInfo->mRevision.Append("rev.1.0", strlen("rev.1.0"));
	sceneInfo->mComment.Append("Thanks", strlen("Thanks"));

	// メッシュノードの作成
	FbxNode* rootNode = fbxScene->GetRootNode();// ルートノードの取得
	FbxNode* meshNode = FbxNode::Create(fbxScene, "ewhNode");
	FbxMesh* cubeMesh = FbxMesh::Create(fbxScene, "ewhMesh");	// --- メッシュの作成 ---
	meshNode->SetNodeAttribute(cubeMesh);
	rootNode->AddChild(meshNode);
	strcpy(fpath, FPath);
	if ((ptr = strrstr(fpath, FName))) *ptr = '\0';
	// マテリアル属性　設定
	// マテリアル作成
	CMaterial* pMaterial = (CMaterial*)m_Materials.Top();
	while (pMaterial != NULL)
	{
		//strcpy(texName, pMaterial->m_Name);
		strcpynosp(texName, pMaterial->m_Name);
		Trim(texName);
		sprintf(texpath, "%s%s.bmp", fpath, texName);
		FbxSurfacePhong* material = FbxSurfacePhong::Create(fbxScene, ("Mat_"+string(texName)).c_str());
		material->Ambient.Set(FbxDouble3(0., 0., 0.));
		material->Diffuse.Set(FbxDouble3(1., 1., 1.));
		material->Specular.Set(FbxDouble3(0., 0., 0.));
		material->ShadingModel.Set("Phong");
		FbxFileTexture* texture = FbxFileTexture::Create(fbxScene, ("Tex_" + string(texName)).c_str());
		texture->SetFileName((string(texName)+".bmp").c_str()); // テクスチャファイルパス設定
		texture->SetRelativeFileName((string(texName) + ".bmp").c_str());
        texture->SetTextureUse(FbxTexture::eStandard);
        texture->SetMappingType(FbxTexture::eUV);
        texture->SetMaterialUse(FbxFileTexture::eModelMaterial); 

        // マテリアルのDiffuseチャンネルにテクスチャを接続
        material->Diffuse.ConnectSrcObject(texture);
        //material->DiffuseFactor.Set(1.0); // Diffuse Factor を 1.0 に設定 (テクスチャをそのまま表示)
		// マテリアルをメッシュノードにアサイン
		meshNode->AddMaterial(material);
		//fbxScene->AddMaterial(material);
		D3DXSaveTextureToFile(texpath, D3DXIFF_BMP, pMaterial->m_pTexture, NULL);
		pMaterial = (CMaterial*)pMaterial->Next;

	}

	// メッシュへのマテリアル割り当て
	// FbxLayer* pLayer = cubeMesh->GetLayer(0);
	FbxLayer* pLayer = cubeMesh->GetLayer(0);
	if (!pLayer)
	{
		cubeMesh->CreateLayer();
		pLayer = cubeMesh->GetLayer(0);
	}

	FbxLayerElementMaterial* pMaterialElement = FbxLayerElementMaterial::Create(cubeMesh, "Material");
	pMaterialElement->SetMappingMode(FbxLayerElement::eByPolygon); // ポリゴンごとにマテリアルを割り当てる
	pMaterialElement->SetReferenceMode(FbxLayerElement::eIndexToDirect); // インデックスで直接マテリアルを参照
	//頂点の出力
	outputFBXVertex(cubeMesh);
	//面の出力
	outputFBXFace(cubeMesh, pMaterialElement);

	pLayer->SetMaterials(pMaterialElement);
	//	ボーン出力
	outputFBXBone(rootNode, fbxScene, cubeMesh);
	std::vector<std::string> strlist;
	strlist.clear();
	//　アニメーション出力
	//outputFBXAnimation(fbxScene);
	char nameback[8]; strcpy(nameback, m_MotionName);
	if (g_mPCFlag) {
		CMotionFrame* pMotionFrame = (CMotionFrame*)pPC->m_motions.Top();
		int cnt = 0;
		while (pMotionFrame) {
			char mName[6]; strncpy(mName, (char*)pMotionFrame->m_Name, 3); mName[3] = '\0';
			int k = 0;
			for (; k < (int)strlist.size(); k++) {
				if (strlist[k] == mName) break;
			}
			if (k >= (int)strlist.size()) {
				strlist.push_back(mName);
				pPC->SetMotionName(mName);
				pPC->LoadPCMotion();
				outputFBXAnimation(fbxScene);
			}
			pMotionFrame = (CMotionFrame*)pMotionFrame->Next;
		}
		pPC->SetMotionName(nameback);
		pPC->LoadPCMotion();
	}
	else {
		CMotionFrame* pMotionFrame = (CMotionFrame*)pNPC->m_motions.Top();
		int cnt = 0;
		while (pMotionFrame) {
			char mName[6]; strncpy(mName, (char*)pMotionFrame->m_Name, 3); mName[3] = '\0';
			int k = 0;
			for (; k < (int)strlist.size(); k++) {
				if (strlist[k] == mName) break;
			}
			if (k >= (int)strlist.size()) {
				strlist.push_back(mName);
				pNPC->SetMotionName(mName);
				pNPC->LoadNPCMotion();
				outputFBXAnimation(fbxScene);
			}
			pMotionFrame = (CMotionFrame*)pMotionFrame->Next;
		}
		pNPC->SetMotionName(nameback);
		pNPC->LoadNPCMotion();
	}
	// --- FBXファイルのエクスポート ---
	int fileFormat=0,lFormatIndex=0, lFormatCount=0;
	lFormatCount = fbxManager->GetIOPluginRegistry()->GetWriterFormatCount();
	if (MessageBox(NULL, "FBX asciiで出力しますか?\nasciiはblenderにインポートできません", "FBX ascii or FBXbinary出力", MB_YESNO | MB_ICONQUESTION) == IDYES) {
		for (lFormatIndex = 0; lFormatIndex < lFormatCount; lFormatIndex++)
		{
			if (fbxManager->GetIOPluginRegistry()->WriterIsFBX(lFormatIndex))
			{
				FbxString lDesc = fbxManager->GetIOPluginRegistry()->GetWriterFormatDescription(lFormatIndex);
				const char* lASCII = "ascii";
				if (lDesc.Find(lASCII) >= 0)
				{
					fileFormat = lFormatIndex;
					break;
				}
			}
		}
	}
	FbxExporter* fbxExporter = FbxExporter::Create(fbxManager, "output");
	if (!fbxExporter->Initialize((string(FPath) + ".fbx").c_str(), fileFormat, fbxManager->GetIOSettings())) {
		FBXSDK_printf("Error: FbxExporter initialization failed for file '%s'.\n", (string(FPath) + ".fbx").c_str());
		FBXSDK_printf("       Error description: %s\n", fbxExporter->GetStatus().GetErrorString());
		fbxExporter->Destroy();
		fbxScene->Destroy();
		fbxManager->Destroy();
		return false;
	}

	if (!fbxExporter->Export(fbxScene)) {
		FBXSDK_printf("Error: FbxExporter export failed.\n");
		fbxExporter->Destroy();
		fbxScene->Destroy();
		fbxManager->Destroy();
		return false;
	}

	// FBXシーンとマネージャーの破棄
	fbxScene->Destroy();
	fbxManager->Destroy();
	return true;
}
//======================================================================
//		Xファイルセーブ
//		データをDirectXフォーマットで出力します
//======================================================================
void PrintTab(FILE* fd, int indentLevel) {
	for (int i = 0; i < indentLevel; ++i) {
		fprintf(fd, "\t");
	}
}

int XTab = 0;

bool CModel::saveX(char* FPath, char* FName)
{
	FILE* fd;
	char* ptr, strmsg[256], path[256];
	D3DXMATRIX lmatrix;
	D3DXMATRIX rootMatrix,mat;

	D3DXMatrixRotationY(&mat, (float)(PAI / 2.));
	D3DXMatrixRotationZ(&rootMatrix, (float)PAI);
	rootMatrix *= mat;
	D3DXMatrixIdentity(&lmatrix);
	//D3DXMatrixIdentity(&rootMatrix);
	XTab = 0; // タブ変数の初期化

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
	PrintTab(fd, XTab); fprintf(fd, "Frame Scene_Root {\n"); XTab++;
	outputMatrix(fd, &rootMatrix);
	PrintTab(fd, XTab); fprintf(fd, "Frame body {\n"); XTab++;
	outputMatrix(fd, &lmatrix);
#if 0
	outputMultiMeshX(FPath, FName, fd); // スキンマルチメッシュ　部品交換用
#else
	PrintTab(fd, XTab); fprintf(fd, "Mesh {\n");  XTab++;// スキンワンメッシュ用
	outputMeshX(FPath, FName, fd);// スキンワンメッシュ用 
	outputSkinX(fd);// スキンワンメッシュ用
	--XTab; PrintTab(fd, XTab); fprintf(fd, "}\n");// スキンワンメッシュ用
#endif
	--XTab; PrintTab(fd, XTab); fprintf(fd, "}\n");
	outputFrameX(fd);
	--XTab; PrintTab(fd, XTab); fprintf(fd, "}\n");
	std::vector<std::string> strlist;
	strlist.clear();
	//outputAnimationSet(fd, m_MotionName);
	char nameback[8]; strcpy(nameback, m_MotionName);
	if (g_mPCFlag) {
		CMotionFrame* pMotionFrame = (CMotionFrame*)pPC->m_motions.Top();
		int cnt = 0;
		while (pMotionFrame) {
			char mName[6]; strncpy(mName, (char*)pMotionFrame->m_Name, 3); mName[3] = '\0';
			int k = 0;
			for (; k < (int)strlist.size(); k++) {
				if (strlist[k] == mName) break;
			}
			if (k >= (int)strlist.size()) {
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
		CMotionFrame* pMotionFrame = (CMotionFrame*)pNPC->m_motions.Top();
		int cnt = 0;
		while (pMotionFrame) {
			char mName[6]; strncpy(mName, (char*)pMotionFrame->m_Name, 3); mName[3] = '\0';
			int k = 0;
			for (; k < (int)strlist.size(); k++) {
				if (strlist[k] == mName) break;
			}
			if (k >= (int)strlist.size()) {
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

bool CModel::outputConvMatrix(FILE* fd, D3DXMATRIX* iMatrix) {
	if (fd == NULL || iMatrix == NULL) return false;
	D3DXVECTOR3	vec, tvec;

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

bool CModel::outputMatrix(FILE* fd, D3DXMATRIX* iMatrix) {

	if (fd == NULL || iMatrix == NULL) return false;
	PrintTab(fd, XTab); fprintf(fd, "FrameTransformMatrix {\n"); XTab++;
	outputMatrixSub(fd, iMatrix);
	--XTab; PrintTab(fd, XTab); fprintf(fd, " }\n");
	return true;
}

bool CModel::outputMatrixSub(FILE* fd, D3DXMATRIX* iMatrix) {

	if (fd == NULL || iMatrix == NULL) return false;
	PrintTab(fd, XTab); fprintf(fd, "%6.6f,%6.6f,%6.6f,%6.6f,%6.6f,%6.6f,%6.6f,%6.6f,%6.6f,%6.6f,%6.6f,%6.6f,%6.6f,%6.6f,%6.6f,%6.6f;;\n"
		, iMatrix->_11, iMatrix->_12, iMatrix->_13, iMatrix->_14, iMatrix->_21, iMatrix->_22, iMatrix->_23, iMatrix->_24,
		iMatrix->_31, iMatrix->_32, iMatrix->_33, iMatrix->_34, iMatrix->_41, iMatrix->_42, iMatrix->_43, iMatrix->_44);
	return true;
}


bool CModel::outputMeshX(char* FPath, char* FName, FILE* fd) {
	char* ptr, fpath[256], texpath[256];

	if (fd == NULL) return false;
	strcpy(fpath, FPath);
	if ((ptr = strrstr(fpath, FName))) *ptr = '\0';
	CMaterial* pMaterial = (CMaterial*)m_Materials.Top();
	while (pMaterial != NULL)
	{
		char texName[256];
		//strcpy(texName, pMaterial->m_Name);
		strcpynosp(texName, pMaterial->m_Name);
		Trim(texName);
		sprintf(texpath, "%s%s.bmp", fpath, texName);
		D3DXSaveTextureToFile(texpath, D3DXIFF_BMP, pMaterial->m_pTexture, NULL);
		pMaterial = (CMaterial*)pMaterial->Next;

	}
	OptimizeVertices();
	outputVertex(fd);
	outputFace(fd);
	//法線出力
	PrintTab(fd, XTab); fprintf(fd, "MeshNormals {\n"); XTab++;
	outputNormal(fd);
	outputNormalFace(fd);
	--XTab; PrintTab(fd, XTab); fprintf(fd, "}\n");
	outputTexCoord(fd);
	outputVerDup(fd);
	outputMaterialList(FPath, FName, fd);
	return true;
}


bool CModel::outputMultiMeshX(char* FPath, char* FName, FILE* fd) {
	int partsNo = 1;
	char* ptr, fpath[256], texpath[256];
	D3DXMATRIX lmatrix;
	D3DXMatrixIdentity(&lmatrix);

	strcpy(fpath, FPath);
	if ((ptr = strrstr(fpath, FName))) *ptr = '\0';
	CMaterial* pMaterial = (CMaterial*)m_Materials.Top();
	while (pMaterial != NULL)
	{
		char texName[256]; strcpynosp(texName, pMaterial->m_Name); Trim(texName);
		sprintf(texpath, "%s%s.bmp", fpath, texName);
		D3DXSaveTextureToFile(texpath, D3DXIFF_BMP, pMaterial->m_pTexture, NULL);
		pMaterial = (CMaterial*)pMaterial->Next;

	}
	CMesh* pMesh = (CMesh*)m_Meshs.Top();
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

bool CModel::outputSkinX(FILE* fd) {
	if (fd == NULL) return false;
	PrintTab(fd, XTab); fprintf(fd, "XSkinMeshHeader {\n"); XTab++;
	PrintTab(fd, XTab); fprintf(fd, "2;\n");
	PrintTab(fd, XTab); fprintf(fd, "2;\n");
	int cntSkinWeights = 0;
	for (int i = 0; i < m_nBone; i++) {
		int	 boneCnt = countBone2Ver(i);
		if (boneCnt > 0) cntSkinWeights++;
	}
	PrintTab(fd, XTab); fprintf(fd, "%d;\n", cntSkinWeights);
	--XTab; PrintTab(fd, XTab); fprintf(fd, "}\n");
	D3DXMATRIX iMatrix;
	D3DXMatrixIdentity(&iMatrix);
	for (int i = 0; i < m_nBone; i++) {
		int	 boneCnt = countBone2Ver(i);
		if (boneCnt <= 0) continue;
		PrintTab(fd, XTab); fprintf(fd, "SkinWeights {\n"); XTab++;
		PrintTab(fd, XTab); fprintf(fd, "\"Bone%03d\";\n", i); XTab++;
		PrintTab(fd, XTab); fprintf(fd, "%d;\n", boneCnt); XTab++;
		outputSkinWeights(fd, i);
		iMatrix = m_Bones[i].m_mInvTrans;
		outputMatrixSub(fd, &iMatrix);
		XTab -= 3; PrintTab(fd, XTab); fprintf(fd, "}\n");
	}
	return true;
}

bool CModel::outputSkinWeights(FILE* fd, int boneNo) {
	outputBone2VerNo(fd, boneNo);
	outputBone2VerWeight(fd, boneNo);
	return true;
}


bool CModel::outputFrameX(FILE* fd) {
	D3DXMATRIX iMatrix;
	int cnt = 0;
	if (fd == NULL) return false;
	PrintTab(fd, XTab); fprintf(fd, "Frame Bone000 {\n"); XTab++;
	iMatrix = m_Bones[0].m_mTransform;
	//iMatrix = m_mRootTransform;
	outputMatrix(fd, &iMatrix);
	cnt = outputFrameXsub(fd, 0);
	--XTab; PrintTab(fd, XTab); fprintf(fd, "}\n");
	return true;
}

int CModel::outputFrameXsub(FILE* fd, int boneNo) {
	int noChild = 0;
	static int callCnt = 0;
	D3DXMATRIX iMatrix;
	D3DXMatrixIdentity(&iMatrix);
	for (int i = boneNo + 1; i < m_nBone; i++) {
		if (m_Bones[i].m_pParent != NULL &&
			m_Bones[i].m_mParent == boneNo) {
			//int	 boneCnt = countBone2Ver(i);
			//if (boneCnt > 0) {
			PrintTab(fd, XTab); fprintf(fd, "Frame Bone%03d {\n", i); XTab++;
			iMatrix = m_Bones[i].m_mTransform;
			outputMatrix(fd, &iMatrix);
			outputFrameXsub(fd, i);
			--XTab; PrintTab(fd, XTab); fprintf(fd, "}\n");
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

bool CModel::outputAnimationSet(FILE* fd, char* nameMotion) {
	if (fd == NULL) return false;
	PrintTab(fd, XTab); fprintf(fd, "AnimationSet %s {\n", nameMotion); XTab++;
	for (int i = 0; i < m_nBone; i++) {
		outputAnimationX(fd, i);
	}
	--XTab; PrintTab(fd, XTab); fprintf(fd, "}\n");
	return true;
}

bool CModel::outputAnimationX(FILE* fd, int boneNo) {

	if (fd == NULL) return false;
	D3DXMATRIX iMatrix;
	int tboneNo = boneNo;
	int keyNum = m_MotionArray[tboneNo].m_RotationKeyNum;
	int tkeyNum = keyNum;
	if (keyNum <= 0) {
		tboneNo = 0;
		tkeyNum = m_MotionArray[tboneNo].m_RotationKeyNum;
	}
	PrintTab(fd, XTab); fprintf(fd, "Animation {\n"); XTab++;
	PrintTab(fd, XTab); fprintf(fd, "{ Bone%03d }\n", boneNo);XTab++;
	PrintTab(fd, XTab); fprintf(fd, "AnimationKey {\n"); XTab++;
	PrintTab(fd, XTab); fprintf(fd, "4;\n");
	PrintTab(fd, XTab); fprintf(fd, "%d;\n", tkeyNum); XTab++;
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
		PrintTab(fd, XTab);
		fprintf(fd, " %d;16; %6.6f,%6.6f,%6.6f,%6.6f,%6.6f,%6.6f,%6.6f,%6.6f,%6.6f,%6.6f,%6.6f,%6.6f,%6.6f,%6.6f,%6.6f,%6.6f;;",
			(int)(fTime * 0.01), iMatrix._11, iMatrix._12, iMatrix._13, iMatrix._14, iMatrix._21, iMatrix._22, iMatrix._23, iMatrix._24,
			iMatrix._31, iMatrix._32, iMatrix._33, iMatrix._34, iMatrix._41, iMatrix._42, iMatrix._43, iMatrix._44);
	}
	fprintf(fd, ";\n");
	XTab -= 2; PrintTab(fd, XTab); fprintf(fd, "}\n");
	XTab -= 2; PrintTab(fd, XTab); fprintf(fd, "}\n");
	return true;
}

int CModel::totalVertex(void) {
	int numVer = 0, numFace = 0;

	// 頂点数、面数カウント
	CMesh* pMesh = (CMesh*)m_Meshs.Top();
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
	CMesh* pMesh = (CMesh*)m_Meshs.Top();
	while (pMesh != NULL)
	{
		int dispCheck = pMesh->GetDispCheck();
		int fCnt = 0;
		CStream* pStream = (CStream*)pMesh->m_Streams.Top();
		while (pStream != NULL) {
			fCnt += pStream->GetFaceCount();
			int	dispLevel = pStream->GetDispLevel();
			if (g_mPCFlag == false || dispLevel == 0 || dispLevel >= dispCheck) {
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

bool CModel::outputVertex(FILE* fd) {
	int numVer = (int)m_weldedVertices.size();
	//頂点数出力
	PrintTab(fd, XTab); fprintf(fd, "%d;\n", numVer); XTab++;
	// 頂点座標出力
	bool notfirst = false;
	for (size_t i = 0; i < m_weldedVertices.size(); i++) {
		if (notfirst)
			fprintf(fd, ",\n");
		else
			notfirst = true;
		PrintTab(fd, XTab); fprintf(fd, "%6.6f;%6.6f;%6.6f;", m_weldedVertices[i].p.x, m_weldedVertices[i].p.y, m_weldedVertices[i].p.z);
	}
	fprintf(fd, ";\n");--XTab;
	return true;
}


bool CModel::outputFace(FILE* fd) {
	int numVer = totalVertex();
	int numFace = totalFace();
	// 面数出力
	PrintTab(fd, XTab); fprintf(fd, "%d;\n", numFace); XTab++;
	// パッチ出力
	int vCnt = 0, fCnt = 0;
	bool notfirst = false;
	CMesh* pMesh = (CMesh*)m_Meshs.Top();
	while (pMesh != NULL) {
		WORD* pI, * pIndex;
		int				i1, i2, i3, t1, t2, t3;
		CUSTOMVERTEX* pV;
		int dispCheck = pMesh->GetDispCheck();
		(pMesh->m_lpIB)->Lock(0, pMesh->m_IBSize, (void**)&pIndex, D3DLOCK_DISCARD);
		(pMesh->m_lpVB1)->Lock(0, pMesh->m_VBSize, (void**)&pV, D3DLOCK_DISCARD);
		CStream* pStream = (CStream*)pMesh->m_Streams.Top();
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
					PrintTab(fd, XTab); fprintf(fd, "3;%d,%d,%d;", t1 + vCnt, t2 + vCnt, t3 + vCnt);
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
					PrintTab(fd, XTab); fprintf(fd, "3;%d,%d,%d;", m_vertexRemap[t1 + vCnt], m_vertexRemap[t2 + vCnt], m_vertexRemap[t3 + vCnt]);
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
	fprintf(fd, ";\n"); --XTab;
	return true;
}

bool CModel::outputNormal(FILE* fd) {
	int numVer = m_weldedVertices.size();
	// 法線数出力
	PrintTab(fd, XTab); fprintf(fd, "%d;\n", numVer); XTab++;
	// 法線ベクトル出力
	bool notfirst = false;
	for (size_t i = 0; i < m_weldedVertices.size(); i++) {
		if (notfirst)
			fprintf(fd, ",\n");
		else
			notfirst = true;
		PrintTab(fd, XTab); fprintf(fd, "%6.6f;%6.6f;%6.6f;", m_weldedVertices[i].n.x, m_weldedVertices[i].n.y, m_weldedVertices[i].n.z);
	}
	fprintf(fd, ";\n"); --XTab;
	return true;
}

bool CModel::outputNormalFace(FILE* fd) {
	int numFace = totalFace();
	// 面数出力
	PrintTab(fd, XTab); fprintf(fd, "%d;\n", numFace); XTab++;
	// パッチ出力
	int vCnt = 0, fCnt = 0;
	bool notfirst = false;
	CMesh* pMesh = (CMesh*)m_Meshs.Top();
	while (pMesh != NULL) {
		WORD* pI, * pIndex;
		int				i1, i2, i3, t1, t2, t3;
		CUSTOMVERTEX* pV;
		int dispCheck = pMesh->GetDispCheck();
		(pMesh->m_lpIB)->Lock(0, pMesh->m_IBSize, (void**)&pIndex, D3DLOCK_DISCARD);
		(pMesh->m_lpVB1)->Lock(0, pMesh->m_VBSize, (void**)&pV, D3DLOCK_DISCARD);
		CStream* pStream = (CStream*)pMesh->m_Streams.Top();
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
						if (pMesh->m_FlipFlag) { t1 = i3; t2 = i2; t3 = i1; } else { t1 = i1; t2 = i2; t3 = i3; }
					} else {
						if (pMesh->m_FlipFlag) { t1 = i1; t2 = i2; t3 = i3; } else { t1 = i3; t2 = i2; t3 = i1; }
					}
					if (notfirst) fprintf(fd, ",\n"); else notfirst = true;
					PrintTab(fd, XTab); fprintf(fd, "3;%d,%d,%d;", m_vertexRemap[t1 + vCnt], m_vertexRemap[t2 + vCnt], m_vertexRemap[t3 + vCnt]);
					i1 = i2; i2 = i3;
				}
			}
			else if (pStream->m_PrimitiveType == D3DPT_TRIANGLELIST) {
				for (unsigned int i = 0; i < pStream->GetFaceCount(); i++) {
					i1 = *pI++; i2 = *pI++; i3 = *pI++;
					if (pMesh->m_FlipFlag) { t1 = i1; t2 = i2; t3 = i3; } else { t1 = i3; t2 = i2; t3 = i1; }
					if (notfirst) fprintf(fd, ",\n"); else notfirst = true;
					PrintTab(fd, XTab); fprintf(fd, "3;%d,%d,%d;", m_vertexRemap[t1 + vCnt], m_vertexRemap[t2 + vCnt], m_vertexRemap[t3 + vCnt]);
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
	fprintf(fd, ";\n"); --XTab;
	return true;
}

bool CModel::outputTexCoord(FILE* fd) {
	int numVer = m_weldedVertices.size();

	// テクスチャu,v出力
	PrintTab(fd, XTab); fprintf(fd, "MeshTextureCoords {\n"); XTab++;
	// uv数出力
	PrintTab(fd, XTab); fprintf(fd, "%d;\n", numVer); XTab++;
	// テクスチャU,V出力
	bool notfirst = false;
	for (size_t i = 0; i < m_weldedVertices.size(); i++) {
		if (notfirst)
			fprintf(fd, ",\n");
		else
			notfirst = true;
		
		float u = m_weldedVertices[i].u;
		float v = m_weldedVertices[i].v;
		if (u < 0.0f) u = 0.0f;
		if (v < 0.0f) v = 0.0f;
		if (u > 1.0f) u = 1.0f;
		if (v > 1.0f) v = 1.0f;
		PrintTab(fd, XTab); fprintf(fd, "%4.6f;%4.6f;", u, v);
	}
	fprintf(fd, ";\n");
	XTab -= 2; PrintTab(fd, XTab); fprintf(fd, "}\n");
	return true;
}

bool CModel::outputVerDup(FILE* fd) {
	int numVer = m_weldedVertices.size();
	// 頂点重複リスト
	PrintTab(fd, XTab); fprintf(fd, " VertexDuplicationIndices {\n"); XTab++;
	PrintTab(fd, XTab); fprintf(fd, " %d;\n", numVer);
	PrintTab(fd, XTab); fprintf(fd, " %d;\n", numVer); XTab++;
	for (int i = 0; i < numVer; i++) {
		if (i > 0) fprintf(fd, ",\n");
		PrintTab(fd, XTab); fprintf(fd, "%d", i);
	}
	fprintf(fd, ";\n");
	XTab -= 2; PrintTab(fd, XTab); fprintf(fd, "}\n");
	return true;
}

bool CModel::outputMaterialList(char* FPath, char* FName, FILE* fd) {
	int numFace = totalFace();
	int vCnt = 0, fCnt = 0;
	// meshマテリアルリスト出力
	PrintTab(fd, XTab); fprintf(fd, "MeshMaterialList {\n"); XTab++;
	// マテリアル数出力
	PrintTab(fd, XTab); fprintf(fd, "%d;\n", m_Materials.Count); XTab++;
	// 面数出力
	PrintTab(fd, XTab); fprintf(fd, " %d;\n", numFace); XTab++;
	// マテリアル番号出力
	bool notfirst = false;
	CMesh* pMesh = (CMesh*)m_Meshs.Top();
	while (pMesh != NULL) {
		WORD* pI, * pIndex;
		int				i1, i2, i3, t1, t2, t3;
		CUSTOMVERTEX* pV;
		int dispCheck = pMesh->GetDispCheck();
		(pMesh->m_lpIB)->Lock(0, pMesh->m_IBSize, (void**)&pIndex, D3DLOCK_DISCARD);
		(pMesh->m_lpVB1)->Lock(0, pMesh->m_VBSize, (void**)&pV, D3DLOCK_DISCARD);
		CStream* pStream = (CStream*)pMesh->m_Streams.Top();
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
					PrintTab(fd, XTab); fprintf(fd, "%d", pStream->m_texNo);
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
					PrintTab(fd, XTab); fprintf(fd, "%d", pStream->m_texNo);
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
	XTab -= 3; PrintTab(fd, XTab); fprintf(fd, "}\n");
	return true;
}

bool CModel::outputMaterial(char* FPath, char* FName, FILE* fd) {
	// material出力
	int count = 0;
	CMaterial* pMaterial = (CMaterial*)m_Materials.Top();
	while (pMaterial != NULL)
	{
		PrintTab(fd, XTab); fprintf(fd, "Material {\n"); XTab++;
		PrintTab(fd, XTab); fprintf(fd, "1.000;1.000;1.000;1.000;;\n");
		PrintTab(fd, XTab); fprintf(fd, "0.000;\n");
		PrintTab(fd, XTab); fprintf(fd, "1.000;1.000;1.000;;\n");
		PrintTab(fd, XTab); fprintf(fd, "0.000;0.000;0.000;;\n");
		PrintTab(fd, XTab); fprintf(fd, "TextureFilename {\n"); XTab++;
		//		fprintf(fd, " \"%s%02d.bmp\";\n", FName, count);
		char texName[256]; strcpynosp(texName, pMaterial->m_Name); Trim(texName);
		PrintTab(fd, XTab); fprintf(fd, "\"%s.bmp\";\n", texName);
		--XTab; PrintTab(fd, XTab); fprintf(fd, "}\n");
		--XTab; PrintTab(fd, XTab); fprintf(fd, "}\n");
		//sprintf(texpath, "%s%02d.bmp", FPath, count);
		//D3DXSaveTextureToFile(texpath, D3DXIFF_BMP, pMaterial->m_pTexture, NULL);
		pMaterial = (CMaterial*)pMaterial->Next;
	}
	return true;
}

int CModel::countBone2Ver(int boneNo) {
	int	 boneCnt = 0;
	int numVer = m_weldedVertices.size();
	for (int i = 0; i < numVer; i++) {
		const WELDED_VERTEX& vtx = m_weldedVertices[i];
		if ((vtx.weights[0] > 0.f && vtx.globalBones[0] == boneNo) ||
			(vtx.weights[1] > 0.f && vtx.globalBones[1] == boneNo)) {
			boneCnt++;
		}
	}
	return boneCnt;
}

bool CModel::outputBone2VerNo(FILE* fd, int boneNo) {
	bool notfirst = false;
	int numVer = m_weldedVertices.size();
	for (int i = 0; i < numVer; i++) {
		const WELDED_VERTEX& vtx = m_weldedVertices[i];
		if (vtx.weights[0] > 0.f && vtx.globalBones[0] == boneNo) {
			if (notfirst) fprintf(fd, ",\n"); else notfirst = true;
			PrintTab(fd, XTab); fprintf(fd, "%5d", i);
		} else if (vtx.weights[1] > 0.f && vtx.globalBones[1] == boneNo) {
			if (notfirst) fprintf(fd, ",\n"); else notfirst = true;
			PrintTab(fd, XTab); fprintf(fd, "%5d", i);
		}
	}
	fprintf(fd, ";\n");
	return true;
}

bool CModel::outputBone2VerWeight(FILE* fd, int boneNo) {
	bool notfirst = false;
	int numVer = m_weldedVertices.size();
	for (int i = 0; i < numVer; i++) {
		const WELDED_VERTEX& vtx = m_weldedVertices[i];
		if (vtx.weights[0] > 0.f && vtx.globalBones[0] == boneNo) {
			if (notfirst) fprintf(fd, ",\n"); else notfirst = true;
			float weight = vtx.weights[0];
			if (weight < 0.0f) weight = 0.0f;
			if (weight > 1.0f) weight = 1.0f;
			PrintTab(fd, XTab); fprintf(fd, "%1.6f", weight);
		} else if (vtx.weights[1] > 0.f && vtx.globalBones[1] == boneNo) {
			if (notfirst) fprintf(fd, ",\n"); else notfirst = true;
			float weight = vtx.weights[1];
			if (weight < 0.0f) weight = 0.0f;
			if (weight > 1.0f) weight = 1.0f;
			PrintTab(fd, XTab); fprintf(fd, "%1.6f", weight);
		}
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
bool CModel::saveMQO(char* FPath, char* FName)
{
	FILE* fd;
	char* ptr, strmsg[256], path[256];

	if ((ptr = strstr(FPath, ".mqo"))) *ptr = '\0';
	if ((ptr = strstr(FName, ".mqo"))) *ptr = '\0';
	sprintf(path, "%s.mqo", FPath);
	HANDLE hFile = CreateFile(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_ARCHIVE, NULL);
	if (hFile != INVALID_HANDLE_VALUE) {
		CloseHandle(hFile);
		wsprintf(strmsg, "ファイル %s　を上書きしますか？", path);
		if (MessageBox(NULL, strmsg, "セーブファイル上書き", MB_YESNO | MB_ICONQUESTION) == IDNO) return false;
	}
	if ((fd = fopen(path, "w")) == NULL) return false;
	fprintf(fd, "Metasequoia Document\nFormat Text Ver 1.0\n\nScene {\n");
	fprintf(fd, "	pos 0.0000 0.0000 5000.0000\n");
	fprintf(fd, "	lookat 0.0000 0.0000 0.0000\n");
	fprintf(fd, "	head 0.0000\n");
	fprintf(fd, "	pich 0.0000\n");
	fprintf(fd, "	ortho 1\n");
	fprintf(fd, "	zoom2 2.0000\n");
	fprintf(fd, "	amb 0.250 0.250 0.250\n}\nMaterial ");
	outputTex(FPath, FName, fd);
	outputMesh(fd);
	fprintf(fd, "EOF");
	fclose(fd);
	return true;
}

//======================================================================
//
//		Texture出力
//
//		TextureデータをMQOフォーマットで出力します
//======================================================================
bool CModel::outputTex(char* FPath, char* FName, FILE* fd)
{
	char	texpath[256];
	int	count = 0;

	if (fd == NULL) return false;
	fprintf(fd, "%d {\n", m_Materials.Count);
	CMaterial* pMaterial = (CMaterial*)m_Materials.Top();
	while (pMaterial != NULL)
	{
		fprintf(fd, "    \"texture%s%02d\" col(1.000 1.000 1.000 1.000)", FName, count);
		fprintf(fd, " dif(1.000) amb(0.250) emi(0.250) spc(0.000) power(5.00) tex(\"%s%02d.bmp\")\n", FName, count);
		sprintf(texpath, "%s%02d.bmp", FPath, count);
		D3DXSaveTextureToFile(texpath, D3DXIFF_BMP, pMaterial->m_pTexture, NULL);
		pMaterial = (CMaterial*)pMaterial->Next;
		count++;
	}
	fprintf(fd, "}\n");
	return true;
}

//======================================================================
//
//		メッシュカウント
//
//		パーツ番号を持つメッシュデータをカウントする
//======================================================================
int CModel::countParts(int partsNo)
{
	int numParts = 0;

	if (partsNo < 1 || partsNo>6) return false;
	CMesh* pMesh = (CMesh*)m_Meshs.Top();
	while (pMesh != NULL)
	{
		if (pMesh->m_PartsNo != partsNo) {
			pMesh = (CMesh*)pMesh->Next; continue;
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
bool CModel::outputMesh(FILE* fd)
{
	char objname[6][32] = {
		"sdef:Face","sdef:Head","sdef:Body", "sdef:Hand", "sdef:Leg", "sdef:Foot" };
	if (fd == NULL) return false;
	//	Face出力
	if (countParts(1) > 0) {
		fprintf(fd, "Object \"%s\" {\n", objname[0]);
		fprintf(fd, "    visible 15\n    locking 0\n    shading 1\n   facet 59.5\n    color 0.898 0.498 0.698\n   color_type 0\n");
		outputVerFace(fd, 1, 2);
		fprintf(fd, "}\n");
	}
	//	Head出力
	if (countParts(2) > 0) {
		fprintf(fd, "Object \"%s\" {\n", objname[1]);
		fprintf(fd, "    visible 15\n    locking 0\n    shading 1\n   facet 59.5\n    color 0.898 0.498 0.698\n   color_type 0\n");
		outputVerFace(fd, 2, 2);
		fprintf(fd, "}\n");
	}
	//	Body出力
	if (countParts(3) > 0) {
		fprintf(fd, "Object \"%s\" {\n", objname[2]);
		fprintf(fd, "    visible 15\n    locking 0\n    shading 1\n   facet 59.5\n    color 0.898 0.498 0.698\n   color_type 0\n");
		outputVerFace(fd, 3, 2);
		fprintf(fd, "}\n");
	}
	//	Hands出力
	if (countParts(4) > 0) {
		// 右手出力
		fprintf(fd, "Object \"%sR\" {\n", objname[3]);
		fprintf(fd, "    visible 15\n    locking 0\n    shading 1\n   facet 59.5\n    color 0.898 0.498 0.698\n   color_type 0\n");
		outputVerFace(fd, 4, 0);
		fprintf(fd, "}\n");
		// 左手出力
		fprintf(fd, "Object \"%sL\" {\n", objname[3]);
		fprintf(fd, "    visible 15\n    locking 0\n    shading 1\n   facet 59.5\n    color 0.898 0.498 0.698\n   color_type 0\n");
		outputVerFace(fd, 4, 1);
		fprintf(fd, "}\n");
	}
	//	Legs出力
	if (countParts(5) > 0) {
		fprintf(fd, "Object \"%s\" {\n", objname[4]);
		fprintf(fd, "    visible 15\n    locking 0\n    shading 1\n   facet 59.5\n    color 0.898 0.498 0.698\n   color_type 0\n");
		outputVerFace(fd, 5, 2);
		fprintf(fd, "}\n");
	}
	//	foots出力
	if (countParts(6) > 0) {
		// 右手出力
		fprintf(fd, "Object \"%sR\" {\n", objname[5]);
		fprintf(fd, "    visible 15\n    locking 0\n    shading 1\n   facet 59.5\n    color 0.898 0.498 0.698\n   color_type 0\n");
		outputVerFace(fd, 6, 0);
		fprintf(fd, "}\n");
		// 左手出力
		fprintf(fd, "Object \"%sL\" {\n", objname[5]);
		fprintf(fd, "    visible 15\n    locking 0\n    shading 1\n   facet 59.5\n    color 0.898 0.498 0.698\n   color_type 0\n");
		outputVerFace(fd, 6, 1);
		fprintf(fd, "}\n");
	}
	return true;
}

//======================================================================
//
//		Vertex及びFace出力
//
//		Vertex及びFaceをMQOフォーマットで出力します
//======================================================================
bool CModel::outputVerFace(FILE* fd, int partsNo, int type)
{
	int verNum = 0, faceNum = 0, vCnt, fCnt;

	if (fd == NULL) return false;
	if (partsNo < 1 || partsNo>6) return false;
	CMesh* pMesh = (CMesh*)m_Meshs.Top();
	while (pMesh != NULL)
	{
		if (pMesh->m_PartsNo != partsNo) {
			pMesh = (CMesh*)pMesh->Next; continue;
		}
		if (type == 0 && pMesh->m_FlipFlag == true) {
			pMesh = (CMesh*)pMesh->Next; continue;
		}
		if (type == 1 && pMesh->m_FlipFlag == false) {
			pMesh = (CMesh*)pMesh->Next; continue;
		}
		verNum += pMesh->m_NumVertices;
		faceNum += pMesh->m_NumFaces;
		pMesh = (CMesh*)pMesh->Next;
	}
	vCnt = 0;
	fprintf(fd, "vertex %d {\n", verNum);
	pMesh = (CMesh*)m_Meshs.Top();
	while (pMesh != NULL)
	{
		if (pMesh->m_PartsNo != partsNo) {
			pMesh = (CMesh*)pMesh->Next; continue;
		}
		if (type == 0 && pMesh->m_FlipFlag == true) {
			pMesh = (CMesh*)pMesh->Next; continue;
		}
		if (type == 1 && pMesh->m_FlipFlag == false) {
			pMesh = (CMesh*)pMesh->Next; continue;
		}
		int			indx;
		D3DXVECTOR4	pos, p1, p2;
		D3DXMATRIX	mat;
		CUSTOMVERTEX* pV1, * pV2;
		D3DXMatrixIdentity(&mat);
		(pMesh->m_lpVB1)->Lock(0, pMesh->m_VBSize, (void**)&pV1, D3DLOCK_DISCARD);
		(pMesh->m_lpVB2)->Lock(0, pMesh->m_VBSize, (void**)&pV2, D3DLOCK_DISCARD);
		for (unsigned int i = 0; i < pMesh->m_NumVertices; i++, pV1++, pV2++) {
			indx = pV1->indx; if (indx > pMesh->m_mBoneNum || indx < 0) indx = 0;
			mat = m_Bones[pMesh->m_pBoneTbl[indx]].m_mWorld;
			indx = pV1->indx; if (indx > pMesh->m_mBoneNum || indx < 0) indx = 0;
			mat = m_Bones[pMesh->m_pBoneTbl[indx]].m_mWorld;
			D3DXVECTOR3 v1(pV1->p.x, pV1->p.y, pV1->p.z);
			D3DXVec3TransformCoord(&v1, &v1, &mat);
			indx = pV2->indx; if (indx > pMesh->m_mBoneNum || indx < 0) indx = 0;
			mat = m_Bones[pMesh->m_pBoneTbl[indx]].m_mWorld;
			D3DXVECTOR3 v2(pV2->p.x, pV2->p.y, pV2->p.z);
			D3DXVec3TransformCoord(&v2, &v2, &mat);
			pos.x = (v1.x * pV1->b1 + v2.x * pV2->b1) * 100.0f;
			pos.y = (v1.y * pV1->b1 + v2.y * pV2->b1) * 100.0f;
			pos.z = (v1.z * pV1->b1 + v2.z * pV2->b1) * 100.0f;
			fprintf(fd, "        %4.4f %4.4f %4.4f\n", pos.x, pos.y, pos.z);
		}
		(pMesh->m_lpVB2)->Unlock();
		(pMesh->m_lpVB1)->Unlock();
		vCnt += pMesh->m_NumVertices;
		pMesh = (CMesh*)pMesh->Next;
	}
	fprintf(fd, "\t}\n");
	fCnt = vCnt = 0;
	fprintf(fd, "face %d {\n", faceNum);
	pMesh = (CMesh*)m_Meshs.Top();
	while (pMesh != NULL)
	{
		if (pMesh->m_PartsNo != partsNo) {
			pMesh = (CMesh*)pMesh->Next; continue;
		}
		if (type == 0 && pMesh->m_FlipFlag == true) {
			pMesh = (CMesh*)pMesh->Next; continue;
		}
		if (type == 1 && pMesh->m_FlipFlag == false) {
			pMesh = (CMesh*)pMesh->Next; continue;
		}
		WORD* pI, * pIndex;
		int				i1, i2, i3, t1, t2, t3;
		CUSTOMVERTEX* pV;
		(pMesh->m_lpIB)->Lock(0, pMesh->m_IBSize, (void**)&pIndex, D3DLOCK_DISCARD);
		(pMesh->m_lpVB1)->Lock(0, pMesh->m_VBSize, (void**)&pV, D3DLOCK_DISCARD);
		CStream* pStream = (CStream*)pMesh->m_Streams.Top();
		while (pStream != NULL) {
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
					fprintf(fd, "\t\t3 V(%3d %3d %3d) M(%2d) UV(%1.5f %1.5f %1.5f %1.5f %1.5f %1.5f)\n",
						t1 + vCnt, t2 + vCnt, t3 + vCnt, pStream->m_texNo,
						(pV + t1)->u, (pV + t1)->v, (pV + t2)->u,
						(pV + t2)->v, (pV + t3)->u, (pV + t3)->v);
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
					fprintf(fd, "\t\t3 V(%3d %3d %3d) M(%2d) UV(%1.5f %1.5f %1.5f %1.5f %1.5f %1.5f)\n",
						t1 + vCnt, t2 + vCnt, t3 + vCnt, pStream->m_texNo,
						(pV + t1)->u, (pV + t1)->v, (pV + t2)->u,
						(pV + t2)->v, (pV + t3)->u, (pV + t3)->v);
				}
			}
			fCnt += pStream->GetFaceCount();
			pStream = (CStream*)pStream->Next;
		}
		(pMesh->m_lpVB1)->Unlock();
		(pMesh->m_lpIB)->Unlock();
		vCnt += pMesh->m_NumVertices;
		//fCnt += pMesh->m_NumFaces;
		pMesh = (CMesh*)pMesh->Next;
	}
	fprintf(fd, "\t}\n");
	return true;
}
