
#pragma once

//======================================================================
// INCLUDE
//======================================================================
#include <commctrl.h>
#include "DxMath.h"

//======================================================================
// ライトデータ（D3DLIGHT9 の代替）
//======================================================================
struct LIGHTDATA {
    struct COLOR { float r, g, b, a; } Diffuse, Ambient, Specular;
    D3DXVECTOR3 Direction;
};

//======================================================================
// グローバル（Render.cpp で定義、他のファイルから参照）
//======================================================================
extern D3DXMATRIX  g_mView;
extern D3DXMATRIX  g_mProjection;
extern LIGHTDATA   g_mLight;
extern D3DXVECTOR3 g_mAt;
extern D3DXVECTOR3 g_mUp;
extern D3DXVECTOR3 g_mEye;
extern D3DXVECTOR3 g_mEyebase;
extern D3DXMATRIX  g_mViewLight;
extern float       g_mLightDist;
extern D3DXVECTOR3 g_mLightPosition;

//======================================================================
// PROTOTYPE
//======================================================================
bool InitRender( void );
void UnInitRender( void );
void Rendering( void );
bool Create3DSpace( void );

struct ID3D11RasterizerState;
ID3D11RasterizerState* GetRasterizerNormal( void );
ID3D11RasterizerState* GetRasterizerFlipped( void );
