
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
static ID3D11PixelShader*       g_pPixelShaderToon    = nullptr;
static ID3D11ShaderResourceView*g_pToonRampSRV        = nullptr;
static ID3D11SamplerState*      g_pClampSampler       = nullptr;
static ID3D11VertexShader*      g_pVertexShaderOutline = nullptr;
static ID3D11PixelShader*       g_pPixelShaderOutline  = nullptr;
static ID3D11Buffer*            g_pCBPerFrame         = nullptr;
static ID3D11Buffer*            g_pCBPerObject        = nullptr;
static ID3D11SamplerState*      g_pLinearSampler      = nullptr;
static ID3D11RasterizerState*   g_pRasterizerNormal   = nullptr;
static ID3D11RasterizerState*   g_pRasterizerFlipped  = nullptr;
static ID3D11RasterizerState*   g_pRasterizerFrontCull        = nullptr;
static ID3D11RasterizerState*   g_pRasterizerFrontCullFlipped = nullptr;
static ID3D11RasterizerState*   g_pRasterizerFloor            = nullptr;
static ID3D11DepthStencilView*  g_pShadowDSV          = nullptr;
static ID3D11ShaderResourceView*g_pShadowSRV          = nullptr;
static ID3D11VertexShader*      g_pShadowVS           = nullptr;
static ID3D11SamplerState*      g_pShadowSampler      = nullptr;
static ID3D11VertexShader*      g_pFloorVS            = nullptr;
static ID3D11PixelShader*       g_pFloorPS            = nullptr;
static ID3D11InputLayout*       g_pFloorInputLayout   = nullptr;
static ID3D11Buffer*            g_pFloorVB            = nullptr;
static ID3D11BlendState*        g_pBlendAlpha         = nullptr;


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
ID3D11PixelShader*      GetPixelShaderToon( void )  { return g_pPixelShaderToon; }
ID3D11ShaderResourceView* GetToonRampSRV( void )    { return g_pToonRampSRV; }
ID3D11SamplerState*       GetClampSampler( void )   { return g_pClampSampler; }
ID3D11VertexShader*     GetVertexShaderOutline( void ) { return g_pVertexShaderOutline; }
ID3D11PixelShader*      GetPixelShaderOutline( void )  { return g_pPixelShaderOutline; }
ID3D11Buffer*           GetCBPerFrame( void )       { return g_pCBPerFrame; }
ID3D11Buffer*           GetCBPerObject( void )      { return g_pCBPerObject; }
ID3D11SamplerState*     GetLinearSampler( void )    { return g_pLinearSampler; }
ID3D11RasterizerState*  GetRasterizerNormal( void ) { return g_pRasterizerNormal; }
ID3D11RasterizerState*  GetRasterizerFlipped( void ){ return g_pRasterizerFlipped; }
ID3D11RasterizerState*  GetRasterizerFrontCull( void )       { return g_pRasterizerFrontCull; }
ID3D11RasterizerState*  GetRasterizerFrontCullFlipped( void ){ return g_pRasterizerFrontCullFlipped; }
ID3D11DepthStencilView*   GetShadowDSV( void )     { return g_pShadowDSV; }
ID3D11ShaderResourceView* GetShadowSRV( void )     { return g_pShadowSRV; }
ID3D11VertexShader*       GetShadowVS( void )      { return g_pShadowVS; }
ID3D11SamplerState*       GetShadowSampler( void ) { return g_pShadowSampler; }


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
//		レンダーターゲット・深度バッファのリサイズ（ウィンドウリサイズ時）
//
//======================================================================
bool ResizeRenderTarget( int w, int h )
{
	// 既存の RTV/DSV を解放してからバッファをリサイズする
	g_pD3DContext->OMSetRenderTargets( 0, nullptr, nullptr );
	if ( g_pRenderTargetView ) { g_pRenderTargetView->Release(); g_pRenderTargetView = nullptr; }
	if ( g_pDepthStencilView ) { g_pDepthStencilView->Release(); g_pDepthStencilView = nullptr; }

	HRESULT hr = g_pSwapChain->ResizeBuffers( 0, (UINT)w, (UINT)h, DXGI_FORMAT_UNKNOWN, 0 );
	if ( FAILED(hr) ) return false;

	return InitRenderTarget();
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
	// トゥーン版ピクセルシェーダーのコンパイル（TOON マクロ有効）
	//==============================================================================
	static const D3D_SHADER_MACRO toonDefines[] = { { "TOON", "1" }, { nullptr, nullptr } };
	hr = D3DCompileFromFile( L"basic_ps.hlsl", toonDefines, nullptr,
	                         "main", "ps_4_0", compileFlags, 0, &pPSBlob, &pErrBlob );
	if ( FAILED(hr) ) {
		if ( pErrBlob ) {
			MessageBoxA( nullptr, (char*)pErrBlob->GetBufferPointer(), "ToonPS Compile Error", MB_OK );
			pErrBlob->Release();
		}
		return false;
	}
	hr = g_pD3DDevice->CreatePixelShader(
		pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(),
		nullptr, &g_pPixelShaderToon );
	pPSBlob->Release();
	if ( FAILED(hr) ) return false;

	//==============================================================================
	// 輪郭線用頂点シェーダーのコンパイル（OUTLINE マクロ有効、レイアウトは既存を共用）
	//==============================================================================
	static const D3D_SHADER_MACRO outlineDefines[] = { { "OUTLINE", "1" }, { nullptr, nullptr } };
	ID3DBlob *pOutlineVSBlob = nullptr;
	hr = D3DCompileFromFile( L"skinning_vs.hlsl", outlineDefines, nullptr,
	                         "main", "vs_4_0", compileFlags, 0, &pOutlineVSBlob, &pErrBlob );
	if ( FAILED(hr) ) {
		if ( pErrBlob ) {
			MessageBoxA( nullptr, (char*)pErrBlob->GetBufferPointer(), "OutlineVS Compile Error", MB_OK );
			pErrBlob->Release();
		}
		return false;
	}
	hr = g_pD3DDevice->CreateVertexShader(
		pOutlineVSBlob->GetBufferPointer(), pOutlineVSBlob->GetBufferSize(),
		nullptr, &g_pVertexShaderOutline );
	pOutlineVSBlob->Release();
	if ( FAILED(hr) ) return false;

	//==============================================================================
	// 輪郭線用ピクセルシェーダーのコンパイル
	//==============================================================================
	hr = D3DCompileFromFile( L"outline_ps.hlsl", nullptr, nullptr,
	                         "main", "ps_4_0", compileFlags, 0, &pPSBlob, &pErrBlob );
	if ( FAILED(hr) ) {
		if ( pErrBlob ) {
			MessageBoxA( nullptr, (char*)pErrBlob->GetBufferPointer(), "OutlinePS Compile Error", MB_OK );
			pErrBlob->Release();
		}
		return false;
	}
	hr = g_pD3DDevice->CreatePixelShader(
		pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(),
		nullptr, &g_pPixelShaderOutline );
	pPSBlob->Release();
	if ( FAILED(hr) ) return false;

	//==============================================================================
	// トゥーン用ランプテクスチャ読み込み（256x1 BMP、横軸=N・L、暗部→明部）
	//==============================================================================
	{
		HBITMAP hBmp = (HBITMAP)LoadImageA( nullptr, "toon_ramp.bmp", IMAGE_BITMAP,
		                                    0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION );
		if ( hBmp == nullptr ) {
			MessageBoxA( nullptr, "toon_ramp.bmp が読み込めません", "ToonRamp Load Error", MB_OK );
			return false;
		}
		DIBSECTION dib = {};
		GetObject( hBmp, sizeof(dib), &dib );
		int  w   = dib.dsBm.bmWidth;
		int  h   = dib.dsBm.bmHeight;
		int  bpp = dib.dsBm.bmBitsPixel;
		if ( h != 1 || ( bpp != 24 && bpp != 32 ) || dib.dsBm.bmBits == nullptr ) {
			DeleteObject( hBmp );
			MessageBoxA( nullptr, "toon_ramp.bmp は 高さ1・24/32bit である必要があります", "ToonRamp Load Error", MB_OK );
			return false;
		}
		// BGR(A) → RGBA 変換
		unsigned char *pSrc = (unsigned char*)dib.dsBm.bmBits;
		unsigned char *pRGBA = new unsigned char[ w * 4 ];
		for ( int x = 0; x < w; x++ ) {
			unsigned char *s = pSrc + x * ( bpp / 8 );
			pRGBA[x*4+0] = s[2];
			pRGBA[x*4+1] = s[1];
			pRGBA[x*4+2] = s[0];
			pRGBA[x*4+3] = 255;
		}
		D3D11_TEXTURE2D_DESC td = {};
		td.Width            = w;
		td.Height           = 1;
		td.MipLevels        = 1;
		td.ArraySize        = 1;
		td.Format           = DXGI_FORMAT_R8G8B8A8_UNORM;
		td.SampleDesc.Count = 1;
		td.Usage            = D3D11_USAGE_IMMUTABLE;
		td.BindFlags        = D3D11_BIND_SHADER_RESOURCE;
		D3D11_SUBRESOURCE_DATA initData = {};
		initData.pSysMem     = pRGBA;
		initData.SysMemPitch = w * 4;
		ID3D11Texture2D *pTex = nullptr;
		hr = g_pD3DDevice->CreateTexture2D( &td, &initData, &pTex );
		delete[] pRGBA;
		DeleteObject( hBmp );
		if ( FAILED(hr) ) return false;
		hr = g_pD3DDevice->CreateShaderResourceView( pTex, nullptr, &g_pToonRampSRV );
		pTex->Release();
		if ( FAILED(hr) ) return false;
	}

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
	// クランプサンプラー生成（ランプテクスチャ用）
	//==============================================================================
	D3D11_SAMPLER_DESC sd3 = {};
	sd3.Filter         = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sd3.AddressU       = D3D11_TEXTURE_ADDRESS_CLAMP;
	sd3.AddressV       = D3D11_TEXTURE_ADDRESS_CLAMP;
	sd3.AddressW       = D3D11_TEXTURE_ADDRESS_CLAMP;
	sd3.MaxAnisotropy  = 1;
	sd3.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	sd3.MaxLOD         = D3D11_FLOAT32_MAX;
	hr = g_pD3DDevice->CreateSamplerState( &sd3, &g_pClampSampler );
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

	rd.FrontCounterClockwise = FALSE;
	rd.CullMode              = D3D11_CULL_FRONT;
	g_pD3DDevice->CreateRasterizerState( &rd, &g_pRasterizerFrontCull );

	rd.FrontCounterClockwise = TRUE;
	g_pD3DDevice->CreateRasterizerState( &rd, &g_pRasterizerFrontCullFlipped );

	//==============================================================================
	// 床専用ラスタライザーステート（CULL_NONE）
	// 床クアッドのTRIANGLESTRIP巻き順が逆でも無音で消えないよう、
	// 両面描画にして表裏カリングの影響を受けないようにする。
	//==============================================================================
	rd.FrontCounterClockwise = FALSE;
	rd.CullMode              = D3D11_CULL_NONE;
	g_pD3DDevice->CreateRasterizerState( &rd, &g_pRasterizerFloor );

	//==============================================================================
	// シャドウパス用頂点シェーダーのコンパイル
	//==============================================================================
	ID3DBlob *pShadowVSBlob = nullptr;
	hr = D3DCompileFromFile( L"shadow_vs.hlsl", nullptr, nullptr,
	                         "main", "vs_4_0", compileFlags, 0, &pShadowVSBlob, &pErrBlob );
	if ( FAILED(hr) ) {
		if ( pErrBlob ) {
			MessageBoxA( nullptr, (char*)pErrBlob->GetBufferPointer(), "ShadowVS Compile Error", MB_OK );
			pErrBlob->Release();
		}
		return false;
	}
	hr = g_pD3DDevice->CreateVertexShader(
		pShadowVSBlob->GetBufferPointer(), pShadowVSBlob->GetBufferSize(),
		nullptr, &g_pShadowVS );
	pShadowVSBlob->Release();
	if ( FAILED(hr) ) return false;

	//==============================================================================
	// シャドウマップ用テクスチャ / DSV / SRV 生成（2048x2048）
	//==============================================================================
	D3D11_TEXTURE2D_DESC shadowTexDesc = {};
	shadowTexDesc.Width            = 2048;
	shadowTexDesc.Height           = 2048;
	shadowTexDesc.MipLevels        = 1;
	shadowTexDesc.ArraySize        = 1;
	shadowTexDesc.Format           = DXGI_FORMAT_R32_TYPELESS;
	shadowTexDesc.SampleDesc.Count = 1;
	shadowTexDesc.Usage            = D3D11_USAGE_DEFAULT;
	shadowTexDesc.BindFlags        = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;

	ID3D11Texture2D *pShadowTex = nullptr;
	hr = g_pD3DDevice->CreateTexture2D( &shadowTexDesc, nullptr, &pShadowTex );
	if ( FAILED(hr) ) return false;

	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format             = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.ViewDimension      = D3D11_DSV_DIMENSION_TEXTURE2D;
	hr = g_pD3DDevice->CreateDepthStencilView( pShadowTex, &dsvDesc, &g_pShadowDSV );
	if ( FAILED(hr) ) { pShadowTex->Release(); return false; }

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format                    = DXGI_FORMAT_R32_FLOAT;
	srvDesc.ViewDimension             = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels       = 1;
	hr = g_pD3DDevice->CreateShaderResourceView( pShadowTex, &srvDesc, &g_pShadowSRV );
	pShadowTex->Release();
	if ( FAILED(hr) ) return false;

	//==============================================================================
	// 比較サンプラー生成（PCF用）
	//==============================================================================
	D3D11_SAMPLER_DESC sdShadow = {};
	sdShadow.Filter         = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
	sdShadow.AddressU       = D3D11_TEXTURE_ADDRESS_BORDER;
	sdShadow.AddressV       = D3D11_TEXTURE_ADDRESS_BORDER;
	sdShadow.AddressW       = D3D11_TEXTURE_ADDRESS_BORDER;
	sdShadow.BorderColor[0] = sdShadow.BorderColor[1] = sdShadow.BorderColor[2] = sdShadow.BorderColor[3] = 1.0f;
	sdShadow.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
	sdShadow.MaxLOD         = D3D11_FLOAT32_MAX;
	hr = g_pD3DDevice->CreateSamplerState( &sdShadow, &g_pShadowSampler );
	if ( FAILED(hr) ) return false;

	//==============================================================================
	// シャドウキャッチャー床用 頂点シェーダーのコンパイル
	//==============================================================================
	ID3DBlob *pFloorVSBlob = nullptr;
	hr = D3DCompileFromFile( L"floor_vs.hlsl", nullptr, nullptr,
	                         "main", "vs_4_0", compileFlags, 0, &pFloorVSBlob, &pErrBlob );
	if ( FAILED(hr) ) {
		if ( pErrBlob ) {
			MessageBoxA( nullptr, (char*)pErrBlob->GetBufferPointer(), "FloorVS Compile Error", MB_OK );
			pErrBlob->Release();
		}
		return false;
	}

	//==============================================================================
	// 床用インプットレイアウト（POSITION float3 のみ）
	//==============================================================================
	D3D11_INPUT_ELEMENT_DESC floorLayout[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	hr = g_pD3DDevice->CreateInputLayout(
		floorLayout, ARRAYSIZE(floorLayout),
		pFloorVSBlob->GetBufferPointer(), pFloorVSBlob->GetBufferSize(),
		&g_pFloorInputLayout );
	if ( FAILED(hr) ) { pFloorVSBlob->Release(); return false; }

	hr = g_pD3DDevice->CreateVertexShader(
		pFloorVSBlob->GetBufferPointer(), pFloorVSBlob->GetBufferSize(),
		nullptr, &g_pFloorVS );
	pFloorVSBlob->Release();
	if ( FAILED(hr) ) return false;

	//==============================================================================
	// シャドウキャッチャー床用 ピクセルシェーダーのコンパイル
	//==============================================================================
	hr = D3DCompileFromFile( L"floor_ps.hlsl", nullptr, nullptr,
	                         "main", "ps_4_0", compileFlags, 0, &pPSBlob, &pErrBlob );
	if ( FAILED(hr) ) {
		if ( pErrBlob ) {
			MessageBoxA( nullptr, (char*)pErrBlob->GetBufferPointer(), "FloorPS Compile Error", MB_OK );
			pErrBlob->Release();
		}
		return false;
	}
	hr = g_pD3DDevice->CreatePixelShader(
		pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(),
		nullptr, &g_pFloorPS );
	pPSBlob->Release();
	if ( FAILED(hr) ) return false;

	//==============================================================================
	// 床用頂点バッファ生成（y≒0 のクアッド、TRIANGLESTRIP用、POSITIONのみ）
	// y = -0.001f: キャラクターの足裏ジオメトリ(y=0)と完全に同一深度になると
	// Zファイティングでちらつくため、わずかに沈めて回避する。
	//==============================================================================
	{
		const float floorY = -0.001f;
		// 大型モデル（翼を広げた大鳥など）の影が床からはみ出さないよう±10単位
		float floorVerts[4][3] = {
			{ -10.0f, floorY, -10.0f },
			{ -10.0f, floorY,  10.0f },
			{  10.0f, floorY, -10.0f },
			{  10.0f, floorY,  10.0f },
		};
		D3D11_BUFFER_DESC vbd = {};
		vbd.ByteWidth = sizeof(floorVerts);
		vbd.Usage     = D3D11_USAGE_IMMUTABLE;
		vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		D3D11_SUBRESOURCE_DATA vbInit = {};
		vbInit.pSysMem = floorVerts;
		hr = g_pD3DDevice->CreateBuffer( &vbd, &vbInit, &g_pFloorVB );
		if ( FAILED(hr) ) return false;
	}

	//==============================================================================
	// アルファブレンドステート生成（床の半透明シャドウ描画用）
	//==============================================================================
	D3D11_BLEND_DESC blendDesc = {};
	blendDesc.RenderTarget[0].BlendEnable           = TRUE;
	blendDesc.RenderTarget[0].SrcBlend              = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend             = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp               = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha         = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha        = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha          = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	hr = g_pD3DDevice->CreateBlendState( &blendDesc, &g_pBlendAlpha );
	if ( FAILED(hr) ) return false;

	return true;
}


//======================================================================
//
//		シャドウパス開始（ライト視点DSVにのみ描画）
//
//======================================================================
void BeginShadowPass( void )
{
	// シャドウSRVをPS slot1から外す（同時バインド防止）
	ID3D11ShaderResourceView *pNullSRV = nullptr;
	g_pD3DContext->PSSetShaderResources( 1, 1, &pNullSRV );

	// カラーターゲットなしでシャドウDSVのみセット
	ID3D11RenderTargetView *pNullRTV = nullptr;
	g_pD3DContext->OMSetRenderTargets( 1, &pNullRTV, g_pShadowDSV );
	g_pD3DContext->ClearDepthStencilView( g_pShadowDSV, D3D11_CLEAR_DEPTH, 1.0f, 0 );

	D3D11_VIEWPORT vp = {};
	vp.Width    = vp.Height = 2048.0f;
	vp.MaxDepth = 1.0f;
	g_pD3DContext->RSSetViewports( 1, &vp );
}


//======================================================================
//
//		シャドウパス終了（メインRTV/DSVを復元しシャドウSRVをバインド）
//
//======================================================================
void EndShadowPass( void )
{
	g_pD3DContext->OMSetRenderTargets( 1, &g_pRenderTargetView, g_pDepthStencilView );

	D3D11_VIEWPORT vp = {};
	vp.Width    = (FLOAT)GetScreenWidth();
	vp.Height   = (FLOAT)GetScreenHeight();
	vp.MaxDepth = 1.0f;
	g_pD3DContext->RSSetViewports( 1, &vp );

	g_pD3DContext->PSSetShaderResources( 1, 1, &g_pShadowSRV );
	g_pD3DContext->PSSetSamplers( 1, 1, &g_pShadowSampler );
}


//======================================================================
//
//		シャドウキャッチャー床 描画
//
//		影の落ちる部分だけ半透明の黒を重ね描きする（y=0 固定ジオメトリ）。
//		CBPerFrame は直前のモデル描画（Rendering/ShadowRendering）で
//		当該フレームの view/proj/light 行列に更新済みのため、ここでの
//		再アップロードは不要。深度テストによりモデルより手前には描かれないため
//		モデル描画後に描いてよい。
//
//======================================================================
void RenderFloorShadow( void )
{
	// ---- 入力レイアウト・頂点バッファ・プリミティブトポロジ ----
	g_pD3DContext->IASetInputLayout( g_pFloorInputLayout );
	UINT stride = sizeof(float) * 3;
	UINT offset = 0;
	g_pD3DContext->IASetVertexBuffers( 0, 1, &g_pFloorVB, &stride, &offset );
	g_pD3DContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP );

	// ---- 床用VS/PS設定 ----
	g_pD3DContext->VSSetShader( g_pFloorVS, nullptr, 0 );
	g_pD3DContext->PSSetShader( g_pFloorPS, nullptr, 0 );

	// ---- CBPerFrame（VS/PS 両方にバインド） ----
	g_pD3DContext->VSSetConstantBuffers( 0, 1, &g_pCBPerFrame );
	g_pD3DContext->PSSetConstantBuffers( 0, 1, &g_pCBPerFrame );

	// ---- シャドウマップ（t1/s1）を明示バインド ----
	g_pD3DContext->PSSetShaderResources( 1, 1, &g_pShadowSRV );
	g_pD3DContext->PSSetSamplers( 1, 1, &g_pShadowSampler );

	// ---- ラスタライザ（巻き順未検証のため両面描画のCULL_NONEを使用） ----
	g_pD3DContext->RSSetState( g_pRasterizerFloor );

	// ---- アルファブレンド有効化して描画、終了後は不透明描画に復帰 ----
	g_pD3DContext->OMSetBlendState( g_pBlendAlpha, nullptr, 0xffffffff );
	g_pD3DContext->Draw( 4, 0 );
	g_pD3DContext->OMSetBlendState( nullptr, nullptr, 0xffffffff );

	// ---- ラスタライザを通常状態に復帰（次のモデル描画に影響を残さない） ----
	g_pD3DContext->RSSetState( g_pRasterizerNormal );
}


//======================================================================
//
//		DirectX 11 解放
//
//======================================================================
void ReleaseD3D( void )
{
	if ( g_pBlendAlpha )        { g_pBlendAlpha->Release();        g_pBlendAlpha        = nullptr; }
	if ( g_pFloorVB )           { g_pFloorVB->Release();           g_pFloorVB           = nullptr; }
	if ( g_pFloorPS )           { g_pFloorPS->Release();           g_pFloorPS           = nullptr; }
	if ( g_pFloorVS )           { g_pFloorVS->Release();           g_pFloorVS           = nullptr; }
	if ( g_pFloorInputLayout )  { g_pFloorInputLayout->Release();  g_pFloorInputLayout  = nullptr; }
	if ( g_pRasterizerFloor )   { g_pRasterizerFloor->Release();   g_pRasterizerFloor   = nullptr; }
	if ( g_pShadowSampler )     { g_pShadowSampler->Release();     g_pShadowSampler     = nullptr; }
	if ( g_pShadowSRV )        { g_pShadowSRV->Release();        g_pShadowSRV         = nullptr; }
	if ( g_pShadowDSV )        { g_pShadowDSV->Release();        g_pShadowDSV         = nullptr; }
	if ( g_pShadowVS )         { g_pShadowVS->Release();         g_pShadowVS          = nullptr; }
	if ( g_pRasterizerFrontCullFlipped ) { g_pRasterizerFrontCullFlipped->Release(); g_pRasterizerFrontCullFlipped = nullptr; }
	if ( g_pRasterizerFrontCull )        { g_pRasterizerFrontCull->Release();        g_pRasterizerFrontCull        = nullptr; }
	if ( g_pRasterizerFlipped ) { g_pRasterizerFlipped->Release(); g_pRasterizerFlipped = nullptr; }
	if ( g_pRasterizerNormal )  { g_pRasterizerNormal->Release();  g_pRasterizerNormal  = nullptr; }
	if ( g_pLinearSampler )     { g_pLinearSampler->Release();     g_pLinearSampler     = nullptr; }
	if ( g_pCBPerObject )       { g_pCBPerObject->Release();       g_pCBPerObject       = nullptr; }
	if ( g_pCBPerFrame )        { g_pCBPerFrame->Release();        g_pCBPerFrame        = nullptr; }
	if ( g_pToonRampSRV )       { g_pToonRampSRV->Release();       g_pToonRampSRV       = nullptr; }
	if ( g_pClampSampler )      { g_pClampSampler->Release();      g_pClampSampler      = nullptr; }
	if ( g_pVertexShaderOutline ) { g_pVertexShaderOutline->Release(); g_pVertexShaderOutline = nullptr; }
	if ( g_pPixelShaderOutline )  { g_pPixelShaderOutline->Release();  g_pPixelShaderOutline  = nullptr; }
	if ( g_pPixelShaderToon )   { g_pPixelShaderToon->Release();   g_pPixelShaderToon   = nullptr; }
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
