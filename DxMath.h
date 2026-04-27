#pragma once

//======================================================================
// D3DX math を DirectXMath で置き換える互換ヘッダー
// DX9互換定数
#ifndef D3D_OK
#define D3D_OK S_OK
#endif
// Phase 2: d3dx9.h の数学型・関数をすべてここで提供する
//======================================================================
#include <DirectXMath.h>
using namespace DirectX;


//======================================================================
// D3DXVECTOR3 互換型
//======================================================================
struct D3DXVECTOR3 : XMFLOAT3
{
	D3DXVECTOR3()                              : XMFLOAT3(0.f, 0.f, 0.f) {}
	D3DXVECTOR3(float x, float y, float z)     : XMFLOAT3(x, y, z) {}
	explicit D3DXVECTOR3(const XMFLOAT3& v)    : XMFLOAT3(v) {}

	D3DXVECTOR3  operator+ (const D3DXVECTOR3& v) const { return { x+v.x, y+v.y, z+v.z }; }
	D3DXVECTOR3  operator- (const D3DXVECTOR3& v) const { return { x-v.x, y-v.y, z-v.z }; }
	D3DXVECTOR3  operator* (float f)              const { return { x*f,   y*f,   z*f   }; }
	D3DXVECTOR3  operator/ (float f)              const { float inv=1.f/f; return { x*inv, y*inv, z*inv }; }
	D3DXVECTOR3  operator- ()                     const { return { -x, -y, -z }; }
	D3DXVECTOR3& operator+=(const D3DXVECTOR3& v) { x+=v.x; y+=v.y; z+=v.z; return *this; }
	D3DXVECTOR3& operator-=(const D3DXVECTOR3& v) { x-=v.x; y-=v.y; z-=v.z; return *this; }
	D3DXVECTOR3& operator*=(float f)              { x*=f; y*=f; z*=f; return *this; }
};
inline D3DXVECTOR3 operator*(float f, const D3DXVECTOR3& v) { return { v.x*f, v.y*f, v.z*f }; }


//======================================================================
// D3DXVECTOR4 互換型
//======================================================================
struct D3DXVECTOR4 : XMFLOAT4
{
	D3DXVECTOR4()                                         : XMFLOAT4(0.f,0.f,0.f,0.f) {}
	D3DXVECTOR4(float x, float y, float z, float w)       : XMFLOAT4(x,y,z,w) {}
	explicit D3DXVECTOR4(const XMFLOAT4& v)               : XMFLOAT4(v) {}

	D3DXVECTOR4 operator+(const D3DXVECTOR4& v) const { return { x+v.x, y+v.y, z+v.z, w+v.w }; }
	D3DXVECTOR4 operator-(const D3DXVECTOR4& v) const { return { x-v.x, y-v.y, z-v.z, w-v.w }; }
	D3DXVECTOR4 operator*(float f)              const { return { x*f,   y*f,   z*f,   w*f   }; }
};


//======================================================================
// D3DXQUATERNION 互換型
//======================================================================
struct D3DXQUATERNION : XMFLOAT4
{
	D3DXQUATERNION()                                          : XMFLOAT4(0.f,0.f,0.f,1.f) {}
	D3DXQUATERNION(float x, float y, float z, float w)        : XMFLOAT4(x,y,z,w) {}
	explicit D3DXQUATERNION(const XMFLOAT4& v)                : XMFLOAT4(v) {}
};


//======================================================================
// D3DXMATRIX 互換型
//======================================================================
struct D3DXMATRIX : XMFLOAT4X4
{
	D3DXMATRIX() : XMFLOAT4X4() {}
	D3DXMATRIX(
		float m11, float m12, float m13, float m14,
		float m21, float m22, float m23, float m24,
		float m31, float m32, float m33, float m34,
		float m41, float m42, float m43, float m44)
		: XMFLOAT4X4(m11,m12,m13,m14, m21,m22,m23,m24, m31,m32,m33,m34, m41,m42,m43,m44) {}
	explicit D3DXMATRIX(const XMFLOAT4X4& m) : XMFLOAT4X4(m) {}

	D3DXMATRIX operator*(const D3DXMATRIX& m) const {
		D3DXMATRIX r;
		XMStoreFloat4x4(&r, XMMatrixMultiply(XMLoadFloat4x4(this), XMLoadFloat4x4(&m)));
		return r;
	}
	D3DXMATRIX& operator*=(const D3DXMATRIX& m) {
		XMStoreFloat4x4(this, XMMatrixMultiply(XMLoadFloat4x4(this), XMLoadFloat4x4(&m)));
		return *this;
	}
	D3DXMATRIX operator+(const D3DXMATRIX& m) const {
		D3DXMATRIX r;
		r._11=_11+m._11; r._12=_12+m._12; r._13=_13+m._13; r._14=_14+m._14;
		r._21=_21+m._21; r._22=_22+m._22; r._23=_23+m._23; r._24=_24+m._24;
		r._31=_31+m._31; r._32=_32+m._32; r._33=_33+m._33; r._34=_34+m._34;
		r._41=_41+m._41; r._42=_42+m._42; r._43=_43+m._43; r._44=_44+m._44;
		return r;
	}
	D3DXMATRIX operator*(float f) const {
		D3DXMATRIX r;
		r._11=_11*f; r._12=_12*f; r._13=_13*f; r._14=_14*f;
		r._21=_21*f; r._22=_22*f; r._23=_23*f; r._24=_24*f;
		r._31=_31*f; r._32=_32*f; r._33=_33*f; r._34=_34*f;
		r._41=_41*f; r._42=_42*f; r._43=_43*f; r._44=_44*f;
		return r;
	}
	D3DXMATRIX& operator*=(float f) {
		_11*=f; _12*=f; _13*=f; _14*=f;
		_21*=f; _22*=f; _23*=f; _24*=f;
		_31*=f; _32*=f; _33*=f; _34*=f;
		_41*=f; _42*=f; _43*=f; _44*=f;
		return *this;
	}
};


//======================================================================
// 行列関数
//======================================================================

inline D3DXMATRIX* D3DXMatrixIdentity(D3DXMATRIX* pOut)
{
	XMStoreFloat4x4(pOut, XMMatrixIdentity());
	return pOut;
}

inline D3DXMATRIX* D3DXMatrixMultiply(D3DXMATRIX* pOut, const D3DXMATRIX* pM1, const D3DXMATRIX* pM2)
{
	XMStoreFloat4x4(pOut, XMMatrixMultiply(XMLoadFloat4x4(pM1), XMLoadFloat4x4(pM2)));
	return pOut;
}

inline D3DXMATRIX* D3DXMatrixTranslation(D3DXMATRIX* pOut, float x, float y, float z)
{
	XMStoreFloat4x4(pOut, XMMatrixTranslation(x, y, z));
	return pOut;
}

inline D3DXMATRIX* D3DXMatrixScaling(D3DXMATRIX* pOut, float sx, float sy, float sz)
{
	XMStoreFloat4x4(pOut, XMMatrixScaling(sx, sy, sz));
	return pOut;
}

inline D3DXMATRIX* D3DXMatrixRotationX(D3DXMATRIX* pOut, float angle)
{
	XMStoreFloat4x4(pOut, XMMatrixRotationX(angle));
	return pOut;
}

inline D3DXMATRIX* D3DXMatrixRotationY(D3DXMATRIX* pOut, float angle)
{
	XMStoreFloat4x4(pOut, XMMatrixRotationY(angle));
	return pOut;
}

inline D3DXMATRIX* D3DXMatrixRotationZ(D3DXMATRIX* pOut, float angle)
{
	XMStoreFloat4x4(pOut, XMMatrixRotationZ(angle));
	return pOut;
}

inline D3DXMATRIX* D3DXMatrixRotationQuaternion(D3DXMATRIX* pOut, const D3DXQUATERNION* pQ)
{
	XMStoreFloat4x4(pOut, XMMatrixRotationQuaternion(XMLoadFloat4(pQ)));
	return pOut;
}

inline D3DXMATRIX* D3DXMatrixLookAtLH(D3DXMATRIX* pOut, const D3DXVECTOR3* pEye,
                                       const D3DXVECTOR3* pAt, const D3DXVECTOR3* pUp)
{
	XMStoreFloat4x4(pOut, XMMatrixLookAtLH(XMLoadFloat3(pEye), XMLoadFloat3(pAt), XMLoadFloat3(pUp)));
	return pOut;
}

inline D3DXMATRIX* D3DXMatrixPerspectiveFovLH(D3DXMATRIX* pOut, float fovY, float aspect,
                                               float zn, float zf)
{
	XMStoreFloat4x4(pOut, XMMatrixPerspectiveFovLH(fovY, aspect, zn, zf));
	return pOut;
}

inline D3DXMATRIX* D3DXMatrixTranspose(D3DXMATRIX* pOut, const D3DXMATRIX* pM)
{
	XMStoreFloat4x4(pOut, XMMatrixTranspose(XMLoadFloat4x4(pM)));
	return pOut;
}

inline D3DXMATRIX* D3DXMatrixInverse(D3DXMATRIX* pOut, float* pDeterminant, const D3DXMATRIX* pM)
{
	XMVECTOR det;
	XMStoreFloat4x4(pOut, XMMatrixInverse(&det, XMLoadFloat4x4(pM)));
	if (pDeterminant) XMStoreFloat(pDeterminant, det);
	return pOut;
}

inline D3DXMATRIX* D3DXMatrixTransformation(
	D3DXMATRIX*          pOut,
	const D3DXVECTOR3*   pScalingCenter,
	const D3DXQUATERNION* pScalingRotation,
	const D3DXVECTOR3*   pScaling,
	const D3DXVECTOR3*   pRotationCenter,
	const D3DXQUATERNION* pRotation,
	const D3DXVECTOR3*   pTranslation)
{
	XMVECTOR vSC = pScalingCenter   ? XMLoadFloat3(pScalingCenter)   : XMVectorZero();
	XMVECTOR vSR = pScalingRotation ? XMLoadFloat4(pScalingRotation) : XMQuaternionIdentity();
	XMVECTOR vS  = pScaling         ? XMLoadFloat3(pScaling)         : g_XMOne.v;
	XMVECTOR vRC = pRotationCenter  ? XMLoadFloat3(pRotationCenter)  : XMVectorZero();
	XMVECTOR vR  = pRotation        ? XMLoadFloat4(pRotation)        : XMQuaternionIdentity();
	XMVECTOR vT  = pTranslation     ? XMLoadFloat3(pTranslation)     : XMVectorZero();
	XMStoreFloat4x4(pOut, XMMatrixTransformation(vSC, vSR, vS, vRC, vR, vT));
	return pOut;
}


//======================================================================
// ベクトル3関数
//======================================================================

inline D3DXVECTOR3* D3DXVec3Normalize(D3DXVECTOR3* pOut, const D3DXVECTOR3* pV)
{
	XMStoreFloat3(pOut, XMVector3Normalize(XMLoadFloat3(pV)));
	return pOut;
}

inline D3DXVECTOR3* D3DXVec3Lerp(D3DXVECTOR3* pOut,
                                  const D3DXVECTOR3* pV1, const D3DXVECTOR3* pV2, float t)
{
	XMStoreFloat3(pOut, XMVectorLerp(XMLoadFloat3(pV1), XMLoadFloat3(pV2), t));
	return pOut;
}

inline D3DXVECTOR3* D3DXVec3Cross(D3DXVECTOR3* pOut,
                                   const D3DXVECTOR3* pV1, const D3DXVECTOR3* pV2)
{
	XMStoreFloat3(pOut, XMVector3Cross(XMLoadFloat3(pV1), XMLoadFloat3(pV2)));
	return pOut;
}

inline float D3DXVec3Dot(const D3DXVECTOR3* pV1, const D3DXVECTOR3* pV2)
{
	float result;
	XMStoreFloat(&result, XMVector3Dot(XMLoadFloat3(pV1), XMLoadFloat3(pV2)));
	return result;
}

inline float D3DXVec3Length(const D3DXVECTOR3* pV)
{
	float result;
	XMStoreFloat(&result, XMVector3Length(XMLoadFloat3(pV)));
	return result;
}

inline float D3DXVec3LengthSq(const D3DXVECTOR3* pV)
{
	float result;
	XMStoreFloat(&result, XMVector3LengthSq(XMLoadFloat3(pV)));
	return result;
}

inline D3DXVECTOR3* D3DXVec3TransformCoord(D3DXVECTOR3* pOut,
                                            const D3DXVECTOR3* pV, const D3DXMATRIX* pM)
{
	XMStoreFloat3(pOut, XMVector3TransformCoord(XMLoadFloat3(pV), XMLoadFloat4x4(pM)));
	return pOut;
}

inline D3DXVECTOR3* D3DXVec3TransformNormal(D3DXVECTOR3* pOut,
                                             const D3DXVECTOR3* pV, const D3DXMATRIX* pM)
{
	XMStoreFloat3(pOut, XMVector3TransformNormal(XMLoadFloat3(pV), XMLoadFloat4x4(pM)));
	return pOut;
}


//======================================================================
// ベクトル4関数
//======================================================================

inline D3DXVECTOR4* D3DXVec4Transform(D3DXVECTOR4* pOut,
                                       const D3DXVECTOR4* pV, const D3DXMATRIX* pM)
{
	XMStoreFloat4(pOut, XMVector4Transform(XMLoadFloat4(pV), XMLoadFloat4x4(pM)));
	return pOut;
}


//======================================================================
// クォータニオン関数
//======================================================================

inline D3DXQUATERNION* D3DXQuaternionRotationMatrix(D3DXQUATERNION* pOut, const D3DXMATRIX* pM)
{
	XMStoreFloat4(pOut, XMQuaternionRotationMatrix(XMLoadFloat4x4(pM)));
	return pOut;
}

inline D3DXQUATERNION* D3DXQuaternionSlerp(D3DXQUATERNION* pOut,
                                            const D3DXQUATERNION* pQ1,
                                            const D3DXQUATERNION* pQ2, float t)
{
	XMStoreFloat4(pOut, XMQuaternionSlerp(XMLoadFloat4(pQ1), XMLoadFloat4(pQ2), t));
	return pOut;
}

inline D3DXQUATERNION* D3DXQuaternionNormalize(D3DXQUATERNION* pOut, const D3DXQUATERNION* pQ)
{
	XMStoreFloat4(pOut, XMQuaternionNormalize(XMLoadFloat4(pQ)));
	return pOut;
}
