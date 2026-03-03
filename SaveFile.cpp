#define _CRT_SECURE_NO_WARNINGS
#include "Dx.h"
#include "Model.h"

#include <iostream>
#include <sstream>
#include <stdio.h>
#include <string>
#include <vector>

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
bool CModel::outputFBXVertex(FbxMesh * pfbxMesh)
{
	D3DXMATRIX rootMatrix, mat;
	D3DXVECTOR3 vert;

	D3DXMatrixRotationY(&mat, (float)(PAI / 2.));
	D3DXMatrixRotationX(&rootMatrix, (float)PAI);
	rootMatrix *= mat;

	FbxGeometryElementNormal* normalElement = pfbxMesh->CreateElementNormal();
	normalElement->SetMappingMode(FbxGeometryElement::eByControlPoint); // 頂点ごとに法線を設定
	normalElement->SetReferenceMode(FbxGeometryElement::eDirect); // 法線を直接指定
	FbxGeometryElementUV* uvElement = pfbxMesh->CreateElementUV("UVSet");
	uvElement->SetMappingMode(FbxGeometryElement::eByControlPoint); // 頂点ごとにUVを設定
	uvElement->SetReferenceMode(FbxGeometryElement::eDirect); // UVを直接指定

	int numVer = totalVertex();	//頂点数出力
	pfbxMesh->InitControlPoints(numVer);
	// 頂点座標出力
	unsigned int no = 0;
	CMesh* pMesh = (CMesh*)m_Meshs.Top();
	while (pMesh != NULL) {
		CUSTOMVERTEX* pV1;
		(pMesh->m_lpVB1)->Lock(0, pMesh->m_VBSize, (void**)&pV1, D3DLOCK_DISCARD);
		for (unsigned int i = 0; i < pMesh->m_NumVertices; i++, pV1++) {
			// if (fabs(pV1->p.x) > 10000.0 || fabs(pV1->p.y) > 10000.0 || fabs(pV1->p.z) > 10000.0) {
// 				pV1->p.x = pV1->p.y = pV1->p.z = pV1->n.x = pV1->n.y = pV1->n.z = 0.;
// 				pV1->u = pV1->v = pV1->b1 = 0.; pV1->indx = 0;
			//}
			vert.x = pV1->p.x; vert.y = pV1->p.y; vert.z = pV1->p.z;
			D3DXVec3TransformCoord(&vert, &vert, &rootMatrix);
			pfbxMesh->SetControlPointAt(FbxVector4(vert.x, vert.y, vert.z), no); no++;
			//pfbxMesh->SetControlPointAt(FbxVector4(pV1->p.x, pV1->p.y, pV1->p.z), no); no++;
			vert.x = pV1->n.x; vert.y = pV1->n.y; vert.z = pV1->n.z;
			D3DXVec3TransformNormal(&vert, &vert, &rootMatrix);
			//normalElement->GetDirectArray().Add(FbxVector4(pV1->n.x, pV1->n.y, pV1->n.z));
			normalElement->GetDirectArray().Add(FbxVector4(vert.x, vert.y, vert.z));
			(pV1->u < 0.0) ? pV1->u = 0.: pV1->u = pV1->u;
			(pV1->u > 1.0) ? pV1->u = 1.: pV1->u = pV1->u;
			(pV1->v < 0.0) ? pV1->v = 0.: pV1->v = pV1->v;
			(pV1->v > 1.0) ? pV1->v = 1.: pV1->v = pV1->v;
			uvElement->GetDirectArray().Add(FbxVector2(pV1->u, (1.-pV1->v)));
		}
		(pMesh->m_lpVB1)->Unlock();
		pMesh = (CMesh*)pMesh->Next;
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
					pfbxMesh->AddPolygon(t1 + vCnt);
					pfbxMesh->AddPolygon(t2 + vCnt);
					pfbxMesh->AddPolygon(t3 + vCnt);
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
					pfbxMesh->AddPolygon(t1 + vCnt);
					pfbxMesh->AddPolygon(t2 + vCnt);
					pfbxMesh->AddPolygon(t3 + vCnt);
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
	D3DXVECTOR3 scale(0., 0., 0.);
	// スケール成分の抽出
	scale.x = D3DXVec3Length(reinterpret_cast<const D3DXVECTOR3*>(&d3dxMatrix.m[0][0]));
	scale.y = D3DXVec3Length(reinterpret_cast<const D3DXVECTOR3*>(&d3dxMatrix.m[1][0]));
	scale.z = D3DXVec3Length(reinterpret_cast<const D3DXVECTOR3*>(&d3dxMatrix.m[2][0]));
	return scale;
}

D3DXVECTOR3 D3DXMat2Euler(const D3DXMATRIX& mat)
{
	FbxAMatrix  rotMat;
	FbxQuaternion qq(0., 0., 0., 1.);
	D3DXVECTOR3 euler(0., 0., 0.);
	D3DXQUATERNION q(0., 0., 0., 1.);

	D3DXQuaternionRotationMatrix(&q, &mat);
	qq.Set(q.x, q.y, q.z, q.w);
	rotMat.SetQ(qq);
	FbxVector4 fbxEuler = rotMat.GetR();
	euler.x = (float)fbxEuler[0];
	euler.y = (float)fbxEuler[1];
	euler.z = (float)fbxEuler[2];
	//// Roll (X軸の回転)
	//double sinr_cosp = 2 * (q.w * q.x + q.y * q.z);
	//double cosr_cosp = 1 - 2 * (q.x * q.x + q.y * q.y);
	//euler.x = (float)(std::atan2(sinr_cosp, cosr_cosp)*180. / PAI);
	//// Pitch (Y軸の回転)
	//double sinp = 2 * (q.w * q.y - q.z * q.x);
	//if (std::abs(sinp) >= 1)
	//	euler.y = (float)(std::copysign(PAI/2., sinp) * 180. / PAI);	// 90度のクランプ
	//else
	//	euler.y = (float)(std::asin(sinp) * 180. / PAI);
	//// Yaw (Z軸の回転)
	//double siny_cosp = 2 * (q.w * q.z + q.x * q.y);
	//double cosy_cosp = 1 - 2 * (q.y * q.y + q.z * q.z);
	//euler.z = (float)(std::atan2(siny_cosp, cosy_cosp) * 180. / PAI);
	return euler;
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
	while (pMesh != NULL) {
		CUSTOMVERTEX* pV1, * pV2;
		(pMesh->m_lpVB1)->Lock(0, pMesh->m_VBSize, (void**)&pV1, D3DLOCK_DISCARD);
		(pMesh->m_lpVB2)->Lock(0, pMesh->m_VBSize, (void**)&pV2, D3DLOCK_DISCARD);
		for (unsigned int i = 0; i < pMesh->m_NumVertices; i++, pV1++, pV2++) {
			if (pV1->b1 > 0.f && pMesh->m_pBoneTbl[pV1->indx] == boneNo) {
				(pV1->b1 < 0.0) ? pV1->b1 = 0.: pV1->b1 = pV1->b1;
				(pV1->b1 > 1.0) ? pV1->b1 = 1.: pV1->b1 = pV1->b1;
				pCBCluster->AddControlPointIndex(i + vCnt, pV1->b1);
			}
			else if (pV2->b1 > 0.f && pMesh->m_pBoneTbl[pV2->indx] == boneNo) {
				(pV2->b1 < 0.0) ? pV2->b1 = 0. : pV2->b1 = pV2->b1;
				(pV2->b1 > 1.0) ? pV2->b1 = 1. : pV2->b1 = pV2->b1;
				pCBCluster->AddControlPointIndex(i + vCnt, pV2->b1);
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
	/*FbxAnimLayer* animLayer = FbxAnimLayer::Create(fbxScene, "BaseAnimationLayer");*/

	if (g_mPCFlag) {
		motionName = string(pPC->GetMotionName());
	}
	else {
		motionName = string(pNPC->GetMotionName());
	}
	FbxAnimStack* pAnimStack = FbxAnimStack::Create(pScene, (motionName + "_S").c_str());
	FbxAnimLayer* pAnimLayer = FbxAnimLayer::Create(pScene, (motionName + "_L").c_str());
	pAnimStack->AddMember(pAnimLayer);

	int j;
	FbxNode* pNode;
	for (int i = 0; i < m_nBone; i++) {
		for (j = 0; j < pScene->GetNodeCount(); j++) {
			pNode = pScene->GetNode(j);
			if (string(m_Bones[i].m_Name) == string(pNode->GetName())) break;
		}
		if (j >= pScene->GetNodeCount()) continue;
		// 位置アニメーションカーブ
		FbxAnimCurve* curveTX = pNode->LclTranslation.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_X, true);
		FbxAnimCurve* curveTY = pNode->LclTranslation.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_Y, true);
		FbxAnimCurve* curveTZ = pNode->LclTranslation.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_Z, true);
		// 回転アニメーションカーブ (Quaternion -> Euler 変換が必要な場合あり)
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
		if (keyNum <= 0) {
			tboneNo = 0;
			tkeyNum = m_MotionArray[tboneNo].m_RotationKeyNum;
		}
		for (int k = 0; k < tkeyNum; k++) {
			FbxTime fbxTime;
			fbxTime.SetSecondDouble(m_MotionArray[tboneNo].m_pTranslateKeys[k].Time /3000.); // 時間を秒単位で設定
			D3DXMATRIX iMatrix;
			if (keyNum > 0) {
				iMatrix = m_Bones[tboneNo].m_mTransform;
				iMatrix *= *(m_MotionArray[tboneNo].GetAnimationMatrix(k, &m_Bones[tboneNo].m_mTransform));
			} else {
				iMatrix = m_Bones[i].m_mTransform;
			}
			t = D3DXMat2Trans(iMatrix); s = D3DXMat2Scale(iMatrix);
			D3DXQuaternionRotationMatrix(&q, &iMatrix); qq.Set(q.x, q.y, q.z, q.w);
			fMat.SetTQS(FbxVector4(t.x, t.y, t.z), qq, FbxVector4(s.x, s.y, s.z));
			// 位置キーフレーム設定
			FbxVector4 val = fMat.GetT();
			curveTX->KeyModifyBegin();
			curveTX->KeyInsert(fbxTime); curveTX->KeySetValue(k, (float)val[0]);
			curveTX->KeyModifyEnd();
			curveTY->KeyModifyBegin();
			curveTY->KeyInsert(fbxTime); curveTY->KeySetValue(k, (float)val[1]);
			curveTY->KeyModifyEnd();
			curveTZ->KeyModifyBegin();
			curveTZ->KeyInsert(fbxTime); curveTZ->KeySetValue(k, (float)val[2]);
			curveTZ->KeyModifyEnd();
			curveTX->KeySetInterpolation(k, FbxAnimCurveDef::eInterpolationLinear);
			curveTY->KeySetInterpolation(k, FbxAnimCurveDef::eInterpolationLinear);
			curveTZ->KeySetInterpolation(k, FbxAnimCurveDef::eInterpolationLinear);
			// 回転キーフレーム設定 (Quaternion -> Euler 変換が必要な場合あり)
			val = fMat.GetR();
			curveRX->KeyModifyBegin();
			curveRX->KeyInsert(fbxTime); curveRX->KeySetValue(k, (float)val[0]);
			curveRX->KeyModifyEnd();
			curveRY->KeyModifyBegin();
			curveRY->KeyInsert(fbxTime); curveRY->KeySetValue(k, (float)val[1]);
			curveRY->KeyModifyEnd();
			curveRZ->KeyModifyBegin();
			curveRZ->KeyInsert(fbxTime); curveRZ->KeySetValue(k, (float)val[2]);
			curveRZ->KeyModifyEnd();
			curveRX->KeySetInterpolation(k, FbxAnimCurveDef::eInterpolationLinear);
			curveRY->KeySetInterpolation(k, FbxAnimCurveDef::eInterpolationLinear);
			curveRZ->KeySetInterpolation(k, FbxAnimCurveDef::eInterpolationLinear);
			// スケールキーフレーム設定
			val = fMat.GetS();
			curveSX->KeyModifyBegin();
			curveSX->KeyInsert(fbxTime); curveSX->KeySetValue(k, (float)val[0]);
			curveSX->KeyModifyEnd();
			curveSY->KeyModifyBegin();
			curveSY->KeyInsert(fbxTime); curveSY->KeySetValue(k, (float)val[1]);
			curveSY->KeyModifyEnd();
			curveSZ->KeyModifyBegin();
			curveSZ->KeyInsert(fbxTime); curveSZ->KeySetValue(k, (float)val[2]);
			curveSZ->KeyModifyEnd();
			//curveSX->KeySetInterpolation(k, FbxAnimCurveDef::eInterpolationLinear);
			//curveSY->KeySetInterpolation(k, FbxAnimCurveDef::eInterpolationLinear);
			//curveSZ->KeySetInterpolation(k, FbxAnimCurveDef::eInterpolationLinear);
			// 補間法設定
		}
	}
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
	//　アニメーション出力
	outputFBXAnimation(fbxScene);
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
	fprintf(fd, " FrameTransformMatrix {\n");
	outputMatrixSub(fd, iMatrix);
	fprintf(fd, " }\n");
	return true;
}

bool CModel::outputMatrixSub(FILE* fd, D3DXMATRIX* iMatrix) {

	if (fd == NULL || iMatrix == NULL) return false;
	fprintf(fd, "  %6.6f,%6.6f,%6.6f,%6.6f,%6.6f,%6.6f,%6.6f,%6.6f,%6.6f,%6.6f,%6.6f,%6.6f,%6.6f,%6.6f,%6.6f,%6.6f;;\n"
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
	fprintf(fd, "   XSkinMeshHeader {\n");
	fprintf(fd, "         2;\n");
	fprintf(fd, "         2;\n");
	int cntSkinWeights = 0;
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

bool CModel::outputSkinWeights(FILE* fd, int boneNo) {
	outputBone2VerNo(fd, boneNo);
	outputBone2VerWeight(fd, boneNo);
	return true;
}


bool CModel::outputFrameX(FILE* fd) {
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

bool CModel::outputAnimationSet(FILE* fd, char* nameMotion) {
	if (fd == NULL) return false;
	fprintf(fd, "  AnimationSet %s {\n", nameMotion);
	for (int i = 0; i < m_nBone; i++) {
		outputAnimationX(fd, i);
	}
	fprintf(fd, "}\n");
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
			(int)(fTime * 0.01), iMatrix._11, iMatrix._12, iMatrix._13, iMatrix._14, iMatrix._21, iMatrix._22, iMatrix._23, iMatrix._24,
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
	int numVer = totalVertex();
	int numFace = totalFace();
	//頂点数出力
	fprintf(fd, " %d;\n", numVer);
	// 頂点座標出力
	bool notfirst = false;
	CMesh* pMesh = (CMesh*)m_Meshs.Top();
	while (pMesh != NULL) {
		CUSTOMVERTEX* pV1;
		(pMesh->m_lpVB1)->Lock(0, pMesh->m_VBSize, (void**)&pV1, D3DLOCK_DISCARD);
		for (unsigned int i = 0; i < pMesh->m_NumVertices; i++, pV1++) {
			if (notfirst)
				fprintf(fd, ",\n");
			else
				notfirst = true;
			if (fabs(pV1->p.x) > 10000.0 ||
				fabs(pV1->p.y) > 10000.0 ||
				fabs(pV1->p.z) > 10000.0) {
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


bool CModel::outputFace(FILE* fd) {
	int numVer = totalVertex();
	int numFace = totalFace();
	// 面数出力
	fprintf(fd, " %d;\n", numFace);
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

bool CModel::outputNormal(FILE* fd) {
	int numVer = totalVertex();
	int numFace = totalFace();

	// 法線数出力
	fprintf(fd, " %d;\n", numVer);
	// 法線ベクトル出力
	bool notfirst = false;
	CMesh* pMesh = (CMesh*)m_Meshs.Top();
	while (pMesh != NULL) {
		CUSTOMVERTEX* pV1;
		(pMesh->m_lpVB1)->Lock(0, pMesh->m_VBSize, (void**)&pV1, D3DLOCK_DISCARD);
		for (unsigned int i = 0; i < pMesh->m_NumVertices; i++, pV1++) {
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

bool CModel::outputNormalFace(FILE* fd) {
	int numVer = totalVertex();
	int numFace = totalFace();
	// 面数出力
	fprintf(fd, " %d;\n", numFace);
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

bool CModel::outputTexCoord(FILE* fd) {
	int numVer = totalVertex();
	int numFace = totalFace();

	// テクスチャu,v出力
	fprintf(fd, "   MeshTextureCoords {\n");
	// uv数出力
	fprintf(fd, " %d;\n", numVer);
	// テクスチャU,V出力
	bool notfirst = false;
	CMesh* pMesh = (CMesh*)m_Meshs.Top();
	while (pMesh != NULL) {
		CUSTOMVERTEX* pV1;
		(pMesh->m_lpVB1)->Lock(0, pMesh->m_VBSize, (void**)&pV1, D3DLOCK_DISCARD);
		for (unsigned int i = 0; i < pMesh->m_NumVertices; i++, pV1++) {
			if (notfirst)
				fprintf(fd, ",\n");
			else
				notfirst = true;
			//if (i == 15) {
			//	int hh = 0;
			//}
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

bool CModel::outputVerDup(FILE* fd) {
	int numVer = totalVertex();
	int numFace = totalFace();
	// 頂点重複リスト
	fprintf(fd, " VertexDuplicationIndices {\n");
	fprintf(fd, "  %d;\n", numVer);
	fprintf(fd, "  %d;\n", numVer);
	for (int i = 0; i < numVer; i++) {
		if (i > 0) fprintf(fd, ",\n");
		fprintf(fd, " %d", i);
	}
	fprintf(fd, ";\n");
	fprintf(fd, "}\n");
	return true;
}

bool CModel::outputMaterialList(char* FPath, char* FName, FILE* fd) {
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

bool CModel::outputMaterial(char* FPath, char* FName, FILE* fd) {
	// material出力
	int count = 0;
	CMaterial* pMaterial = (CMaterial*)m_Materials.Top();
	while (pMaterial != NULL)
	{
		fprintf(fd, " Material {\n");
		fprintf(fd, " 1.000;1.000;1.000;1.000;;\n");
		fprintf(fd, " 0.000;\n");
		fprintf(fd, " 1.000;1.000;1.000;;\n");
		fprintf(fd, " 0.000;0.000;0.000;;\n");
		fprintf(fd, " TextureFilename {\n");
		//		fprintf(fd, " \"%s%02d.bmp\";\n", FName, count);
		char texName[256]; strcpynosp(texName, pMaterial->m_Name); Trim(texName);
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

	CMesh* pMesh = (CMesh*)m_Meshs.Top();
	while (pMesh != NULL) {
		CUSTOMVERTEX* pV1, * pV2;
		(pMesh->m_lpVB1)->Lock(0, pMesh->m_VBSize, (void**)&pV1, D3DLOCK_DISCARD);
		(pMesh->m_lpVB2)->Lock(0, pMesh->m_VBSize, (void**)&pV2, D3DLOCK_DISCARD);
		for (unsigned int i = 0; i < pMesh->m_NumVertices; i++, pV1++, pV2++) {
			if (pV1->b1 > 0.f && pMesh->m_pBoneTbl[pV1->indx] == boneNo) {
				boneCnt++;
			}
			else if (pV2->b1 > 0.f && pMesh->m_pBoneTbl[pV2->indx] == boneNo) {
				boneCnt++;
			}
		}
		(pMesh->m_lpVB1)->Unlock();
		(pMesh->m_lpVB2)->Unlock();
		pMesh = (CMesh*)pMesh->Next;
	}
	return boneCnt;
}

bool CModel::outputBone2VerNo(FILE* fd, int boneNo) {
	int vCnt = 0, fCnt = 0;
	bool notfirst = false;
	CMesh* pMesh = (CMesh*)m_Meshs.Top();
	while (pMesh != NULL) {
		CUSTOMVERTEX* pV1, * pV2;
		(pMesh->m_lpVB1)->Lock(0, pMesh->m_VBSize, (void**)&pV1, D3DLOCK_DISCARD);
		(pMesh->m_lpVB2)->Lock(0, pMesh->m_VBSize, (void**)&pV2, D3DLOCK_DISCARD);
		for (unsigned int i = 0; i < pMesh->m_NumVertices; i++, pV1++, pV2++) {
			if (pV1->b1 > 0.f && pMesh->m_pBoneTbl[pV1->indx] == boneNo) {
				if (notfirst)
					fprintf(fd, ",\n");
				else
					notfirst = true;
				fprintf(fd, "  %5d", i + vCnt);
				//fprintf(fd, "  %5d(X %4.4f Y %4.4f Z %4.4f W %1.6f idx %4d idxG %4d local %4d global %4d)", i + vCnt,
				//pV1->p.x, pV1->p.y, pV1->p.z, pV1->b1, pV1->indx, pMesh->m_pBoneTbl[pV1->indx], i, i + vCnt);
			}
			else if (pV2->b1 > 0.f && pMesh->m_pBoneTbl[pV2->indx] == boneNo) {
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

bool CModel::outputBone2VerWeight(FILE* fd, int boneNo) {
	int vCnt = 0, fCnt = 0;
	bool notfirst = false;
	CMesh* pMesh = (CMesh*)m_Meshs.Top();
	while (pMesh != NULL) {
		CUSTOMVERTEX* pV1, * pV2;
		(pMesh->m_lpVB1)->Lock(0, pMesh->m_VBSize, (void**)&pV1, D3DLOCK_DISCARD);
		(pMesh->m_lpVB2)->Lock(0, pMesh->m_VBSize, (void**)&pV2, D3DLOCK_DISCARD);
		for (unsigned int i = 0; i < pMesh->m_NumVertices; i++, pV1++, pV2++) {
			if (pV1->b1 > 0.f && pMesh->m_pBoneTbl[pV1->indx] == boneNo) {
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
			else if (pV2->b1 > 0.f && pMesh->m_pBoneTbl[pV2->indx] == boneNo) {
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