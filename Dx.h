
#pragma once

//======================================================================
// INCLUDE
//======================================================================
#include <d3d9.h>
#include <d3dx9.h>


//======================================================================
// PROTOTYPE
//======================================================================
bool InitD3D( void );
void ReleaseD3D( void );

LPDIRECT3DDEVICE9 GetDevice( void );
D3DPRESENT_PARAMETERS *GetAdapter( void );
unsigned long GetVertexShaderVersion( void );

HRESULT CreateVB( LPDIRECT3DVERTEXBUFFER9 *lpVB, DWORD size, DWORD Usage, DWORD fvf );
HRESULT CreateIB( LPDIRECT3DINDEXBUFFER9 *lpIB, DWORD size, DWORD Usage );