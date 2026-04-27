
//======================================================================
// INCLUDE
//======================================================================
#include "WinMain.h"
#include "Dx.h"


//======================================================================
// GLOBAL
//======================================================================
static ID3D11Device*            g_pD3DDevice         = nullptr;
static ID3D11DeviceContext*     g_pD3DContext         = nullptr;
static IDXGISwapChain*          g_pSwapChain          = nullptr;
static ID3D11RenderTargetView*  g_pRenderTargetView   = nullptr;
static ID3D11DepthStencilView*  g_pDepthStencilView   = nullptr;
static ID3D11InputLayout*       g_pInputLayout        = nullptr;
static ID3D11VertexShader*      g_pVertexShader       = nullptr;
static ID3D11PixelShader*       g_pPixelShader        = nullptr;
static ID3D11Buffer*            g_pCBPerFrame         = nullptr;
static ID3D11Buffer*            g_pCBPerObject        = nullptr;
static ID3D11SamplerState*      g_pLinearSampler      = nullptr;
static ID3D11RasterizerState*   g_pRasterizerNormal   = nullptr;
static ID3D11RasterizerState*   g_pRasterizerFlipped  = nullptr;


//======================================================================
// アクセサー
//======================================================================
ID3D11Device*           GetDevice( void )          { return g_pD3DDevice; }
ID3D11DeviceContext*    GetContext( void )          { return g_pD3DContext; }
IDXGISwapChain*         GetSwapChain( void )        { return g_pSwapChain; }
ID3D11RenderTargetView* GetRenderTargetView( void ) { return g_pRenderTargetView; }
ID3D11DepthStencilView* GetDepthStencilView( void ) { return g_pDepthStencilView; }
ID3D11InputLayout*      GetInputLayout( void )      { return g_pInputLayout; }
ID3D11VertexShader*     GetVertexShader( void )     { return g_pVertexShader; }
ID3D11PixelShader*      GetPixelShader( void )      { return g_pPixelShader; }
ID3D11Buffer*           GetCBPerFrame( void )       { return g_pCBPerFrame; }
ID3D11Buffer*           GetCBPerObject( void )      { return g_pCBPerObject; }
ID3D11SamplerState*     GetLinearSampler( void )    { return g_pLinearSampler; }
ID3D11RasterizerState*  GetRasterizerNormal( void ) { return g_pRasterizerNormal; }
ID3D11RasterizerState*  GetRasterizerFlipped( void ){ return g_pRasterizerFlipped; }


//======================================================================
//
//		DirectX 11 初期化
//
//======================================================================
bool InitD3D( void )
{
	//==============================================================================
	// スワップチェーン設定
	//==============================================================================
	DXGI_SWAP_CHAIN_DESC sd                       = {};
	sd.BufferCount                                = 1;
	sd.BufferDesc.Width                           = (UINT)GetScreenWidth();
	sd.BufferDesc.Height                          = (UINT)GetScreenHeight();
	sd.BufferDesc.Format                          = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator           = 60;
	sd.BufferDesc.RefreshRate.Denominator         = 1;
	sd.BufferUsage                                = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow                               = GetWindow();
	sd.SampleDesc.Count                           = 1;
	sd.SampleDesc.Quality                         = 0;
	sd.Windowed                                   = TRUE;
	sd.SwapEffect                                 = DXGI_SWAP_EFFECT_DISCARD;

	//==============================================================================
	// 対応フィーチャーレベル（DX11 → DX10.1 → DX10 の順）
	//==============================================================================
	D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};
	D3D_FEATURE_LEVEL featureLevel;

	UINT createFlags = 0;
#ifdef _DEBUG
	createFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	//==============================================================================
	// デバイス・コンテキスト・スワップチェーン生成
	//==============================================================================
	HRESULT hr = D3D11CreateDeviceAndSwapChain(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		createFlags,
		featureLevels,
		ARRAYSIZE(featureLevels),
		D3D11_SDK_VERSION,
		&sd,
		&g_pSwapChain,
		&g_pD3DDevice,
		&featureLevel,
		&g_pD3DContext
	);

	if ( FAILED(hr) ) {
		MessageBox( nullptr, "Direct3D 11デバイスの生成に失敗しました", "Error", MB_OK | MB_ICONSTOP );
		return false;
	}

	if ( !InitRenderTarget() ) {
		MessageBox( nullptr, "レンダーターゲットの初期化に失敗しました", "Error", MB_OK | MB_ICONSTOP );
		return false;
	}

	if ( !InitShaders() ) {
		MessageBox( nullptr, "シェーダーの初期化に失敗しました", "Error", MB_OK | MB_ICONSTOP );
		return false;
	}

	return true;
}


//======================================================================
//
//		レンダーターゲット・深度バッファ初期化
//
//======================================================================
bool InitRenderTarget( void )
{
	HRESULT hr;

	//==============================================================================
	// バックバッファから RenderTargetView を作成
	//==============================================================================
	ID3D11Texture2D *pBackBuffer = nullptr;
	hr = g_pSwapChain->GetBuffer( 0, __uuidof(ID3D11Texture2D), (void**)&pBackBuffer );
	if ( FAILED(hr) ) return false;

	hr = g_pD3DDevice->CreateRenderTargetView( pBackBuffer, nullptr, &g_pRenderTargetView );
	pBackBuffer->Release();
	if ( FAILED(hr) ) return false;

	//==============================================================================
	// 深度ステンシルテクスチャ・ビューを作成
	//==============================================================================
	D3D11_TEXTURE2D_DESC depthDesc = {};
	depthDesc.Width              = (UINT)GetScreenWidth();
	depthDesc.Height             = (UINT)GetScreenHeight();
	depthDesc.MipLevels          = 1;
	depthDesc.ArraySize          = 1;
	depthDesc.Format             = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthDesc.SampleDesc.Count   = 1;
	depthDesc.SampleDesc.Quality = 0;
	depthDesc.Usage              = D3D11_USAGE_DEFAULT;
	depthDesc.BindFlags          = D3D11_BIND_DEPTH_STENCIL;

	ID3D11Texture2D *pDepthTex = nullptr;
	hr = g_pD3DDevice->CreateTexture2D( &depthDesc, nullptr, &pDepthTex );
	if ( FAILED(hr) ) return false;

	hr = g_pD3DDevice->CreateDepthStencilView( pDepthTex, nullptr, &g_pDepthStencilView );
	pDepthTex->Release();
	if ( FAILED(hr) ) return false;

	//==============================================================================
	// RTV・DSV をパイプラインに設定
	//==============================================================================
	g_pD3DContext->OMSetRenderTargets( 1, &g_pRenderTargetView, g_pDepthStencilView );

	//==============================================================================
	// ビューポートの設定
	//==============================================================================
	D3D11_VIEWPORT vp = {};
	vp.Width    = (FLOAT)GetScreenWidth();
	vp.Height   = (FLOAT)GetScreenHeight();
	vp.MinDepth = 0.f;
	vp.MaxDepth = 1.f;
	vp.TopLeftX = 0.f;
	vp.TopLeftY = 0.f;
	g_pD3DContext->RSSetViewports( 1, &vp );

	return true;
}


//======================================================================
//
//		シェーダー・インプットレイアウト・定数バッファ初期化
//
//======================================================================
bool InitShaders( void )
{
	HRESULT hr;
	UINT compileFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
	compileFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	//==============================================================================
	// 頂点シェーダーのコンパイル
	//==============================================================================
	ID3DBlob *pVSBlob  = nullptr;
	ID3DBlob *pErrBlob = nullptr;
	hr = D3DCompileFromFile( L"skinning_vs.hlsl", nullptr, nullptr,
	                         "main", "vs_4_0", compileFlags, 0, &pVSBlob, &pErrBlob );
	if ( FAILED(hr) ) {
		if ( pErrBlob ) {
			MessageBoxA( nullptr, (char*)pErrBlob->GetBufferPointer(), "VS Compile Error", MB_OK );
			pErrBlob->Release();
		}
		return false;
	}

	//==============================================================================
	// インプットレイアウト（CUSTOMVERTEX に対応）
	//==============================================================================
	D3D11_INPUT_ELEMENT_DESC layout[] = {
		// Slot 0: VB1 (bone1)
		{ "POSITION",     0, DXGI_FORMAT_R32G32B32_FLOAT,   0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BLENDWEIGHT",  0, DXGI_FORMAT_R32_FLOAT,          0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BLENDINDICES", 0, DXGI_FORMAT_R8G8B8A8_UINT,     0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",       0, DXGI_FORMAT_R32G32B32_FLOAT,   0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",     0, DXGI_FORMAT_R32G32_FLOAT,      0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		// Slot 1: VB2 (bone2)
		{ "POSITION",     1, DXGI_FORMAT_R32G32B32_FLOAT,   1,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BLENDWEIGHT",  1, DXGI_FORMAT_R32_FLOAT,          1, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BLENDINDICES", 1, DXGI_FORMAT_R8G8B8A8_UINT,     1, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",       1, DXGI_FORMAT_R32G32B32_FLOAT,   1, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	hr = g_pD3DDevice->CreateInputLayout(
		layout, ARRAYSIZE(layout),
		pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(),
		&g_pInputLayout );
	if ( FAILED(hr) ) { pVSBlob->Release(); return false; }

	hr = g_pD3DDevice->CreateVertexShader(
		pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(),
		nullptr, &g_pVertexShader );
	pVSBlob->Release();
	if ( FAILED(hr) ) return false;

	//==============================================================================
	// ピクセルシェーダーのコンパイル
	//==============================================================================
	ID3DBlob *pPSBlob = nullptr;
	hr = D3DCompileFromFile( L"basic_ps.hlsl", nullptr, nullptr,
	                         "main", "ps_4_0", compileFlags, 0, &pPSBlob, &pErrBlob );
	if ( FAILED(hr) ) {
		if ( pErrBlob ) {
			MessageBoxA( nullptr, (char*)pErrBlob->GetBufferPointer(), "PS Compile Error", MB_OK );
			pErrBlob->Release();
		}
		return false;
	}
	hr = g_pD3DDevice->CreatePixelShader(
		pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(),
		nullptr, &g_pPixelShader );
	pPSBlob->Release();
	if ( FAILED(hr) ) return false;

	//==============================================================================
	// 定数バッファ生成
	//==============================================================================
	D3D11_BUFFER_DESC cbd = {};
	cbd.BindFlags      = D3D11_BIND_CONSTANT_BUFFER;
	cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbd.Usage          = D3D11_USAGE_DYNAMIC;

	cbd.ByteWidth = sizeof(CBPerFrame);
	hr = g_pD3DDevice->CreateBuffer( &cbd, nullptr, &g_pCBPerFrame );
	if ( FAILED(hr) ) return false;

	cbd.ByteWidth = sizeof(CBPerObject);
	hr = g_pD3DDevice->CreateBuffer( &cbd, nullptr, &g_pCBPerObject );
	if ( FAILED(hr) ) return false;

	//==============================================================================
	// リニアサンプラー生成
	//==============================================================================
	D3D11_SAMPLER_DESC sd2 = {};
	sd2.Filter         = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sd2.AddressU       = D3D11_TEXTURE_ADDRESS_WRAP;
	sd2.AddressV       = D3D11_TEXTURE_ADDRESS_WRAP;
	sd2.AddressW       = D3D11_TEXTURE_ADDRESS_WRAP;
	sd2.MaxAnisotropy  = 1;
	sd2.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	sd2.MaxLOD         = D3D11_FLOAT32_MAX;
	hr = g_pD3DDevice->CreateSamplerState( &sd2, &g_pLinearSampler );
	if ( FAILED(hr) ) return false;

	//==============================================================================
	// ラスタライザーステート生成（通常 / ミラーメッシュ用）
	//==============================================================================
	D3D11_RASTERIZER_DESC rd = {};
	rd.FillMode        = D3D11_FILL_SOLID;
	rd.CullMode        = D3D11_CULL_BACK;
	rd.DepthClipEnable = TRUE;
	rd.FrontCounterClockwise = FALSE;
	g_pD3DDevice->CreateRasterizerState( &rd, &g_pRasterizerNormal );
	g_pD3DContext->RSSetState( g_pRasterizerNormal );

	rd.FrontCounterClockwise = TRUE;
	g_pD3DDevice->CreateRasterizerState( &rd, &g_pRasterizerFlipped );

	return true;
}


//======================================================================
//
//		DirectX 11 解放
//
//======================================================================
void ReleaseD3D( void )
{
	if ( g_pRasterizerFlipped ) { g_pRasterizerFlipped->Release(); g_pRasterizerFlipped = nullptr; }
	if ( g_pRasterizerNormal )  { g_pRasterizerNormal->Release();  g_pRasterizerNormal  = nullptr; }
	if ( g_pLinearSampler )     { g_pLinearSampler->Release();     g_pLinearSampler     = nullptr; }
	if ( g_pCBPerObject )       { g_pCBPerObject->Release();       g_pCBPerObject       = nullptr; }
	if ( g_pCBPerFrame )        { g_pCBPerFrame->Release();        g_pCBPerFrame        = nullptr; }
	if ( g_pPixelShader )       { g_pPixelShader->Release();       g_pPixelShader       = nullptr; }
	if ( g_pVertexShader )      { g_pVertexShader->Release();      g_pVertexShader      = nullptr; }
	if ( g_pInputLayout )       { g_pInputLayout->Release();       g_pInputLayout       = nullptr; }
	if ( g_pDepthStencilView )  { g_pDepthStencilView->Release();  g_pDepthStencilView  = nullptr; }
	if ( g_pRenderTargetView )  { g_pRenderTargetView->Release();  g_pRenderTargetView  = nullptr; }
	if ( g_pD3DContext )        { g_pD3DContext->Release();        g_pD3DContext        = nullptr; }
	if ( g_pSwapChain )         { g_pSwapChain->Release();         g_pSwapChain         = nullptr; }
	if ( g_pD3DDevice )         { g_pD3DDevice->Release();         g_pD3DDevice         = nullptr; }
}


//======================================================================
//
//		頂点バッファ生成
//
//======================================================================
HRESULT CreateVB( ID3D11Buffer** ppVB, UINT byteWidth, bool dynamic )
{
	D3D11_BUFFER_DESC bd = {};
	bd.ByteWidth      = byteWidth;
	bd.BindFlags      = D3D11_BIND_VERTEX_BUFFER;
	bd.Usage          = dynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
	bd.CPUAccessFlags = dynamic ? D3D11_CPU_ACCESS_WRITE : 0;
	return g_pD3DDevice->CreateBuffer( &bd, nullptr, ppVB );
}


//======================================================================
//
//		インデックスバッファ生成
//
//======================================================================
HRESULT CreateIB( ID3D11Buffer** ppIB, UINT byteWidth, bool dynamic )
{
	D3D11_BUFFER_DESC bd = {};
	bd.ByteWidth      = byteWidth;
	bd.BindFlags      = D3D11_BIND_INDEX_BUFFER;
	bd.Usage          = dynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
	bd.CPUAccessFlags = dynamic ? D3D11_CPU_ACCESS_WRITE : 0;
	return g_pD3DDevice->CreateBuffer( &bd, nullptr, ppIB );
}
