#include <System/Graphics/Graphics.h>
#include <System/Macro.h>
#include <System/Texture/TextureFactory.h>
#include <System/Texture/DepthStencil.h>
#include <System/Texture/RenderTarget.h>

//--- グローバル変数
ID3D11Device *g_pDevice;
ID3D11DeviceContext *g_pContext;
IDXGISwapChain *g_pSwapChain;

ID3D11RasterizerState* g_pRasterizer[CULL_MAX];


HRESULT InitDX(HWND hWnd, UINT width, UINT height, bool fullscreen)
{
	HRESULT	hr = E_FAIL;

	// スワップチェインの設定
	// スワップチェインとは、ウインドウへの表示ダブルバッファを管理する
	// マルチサンプリング、リフレッシュレートが設定できる
	// 複数のバックバッファが作成できる
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));					// ゼロクリア
	sd.BufferDesc.Width = width;						// バックバッファの幅
	sd.BufferDesc.Height = height;						// バックバッファの高さ
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;	// バックバッファフォーマット(R,G,B,A)
	sd.SampleDesc.Count = 1;		// マルチサンプルの数
	sd.BufferDesc.RefreshRate.Numerator = 1000;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;	// バックバッファの使用方法
	sd.BufferCount = 1;									// バックバッファの数
	sd.OutputWindow = hWnd;			// 関連付けるウインドウ
	sd.Windowed = fullscreen ? FALSE : TRUE;
	//sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	
	// ドライバの種類
	D3D_DRIVER_TYPE driverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE,	// GPUで描画
		D3D_DRIVER_TYPE_WARP,		// 高精度(低速
		D3D_DRIVER_TYPE_REFERENCE,	// CPUで描画
	};
	UINT numDriverTypes = ARRAYSIZE(driverTypes);

	UINT createDeviceFlags = 0;
	// createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
	// 機能レベル
	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_1,		// DirectX11.1対応GPUレベル
		D3D_FEATURE_LEVEL_11_0,		// DirectX11対応GPUレベル
		D3D_FEATURE_LEVEL_10_1,		// DirectX10.1対応GPUレベル
		D3D_FEATURE_LEVEL_10_0,		// DirectX10対応GPUレベル
		D3D_FEATURE_LEVEL_9_3,		// DirectX9.3対応GPUレベル
		D3D_FEATURE_LEVEL_9_2,		// DirectX9.2対応GPUレベル
		D3D_FEATURE_LEVEL_9_1		// Direct9.1対応GPUレベル
	};
	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

	D3D_DRIVER_TYPE driverType;
	D3D_FEATURE_LEVEL featureLevel;

	for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; ++driverTypeIndex)
	{
		driverType = driverTypes[driverTypeIndex];
		hr = D3D11CreateDeviceAndSwapChain(
			NULL,					// ディスプレイデバイスのアダプタ（NULLの場合最初に見つかったアダプタ）
			driverType,				// デバイスドライバのタイプ
			NULL,					// ソフトウェアラスタライザを使用する場合に指定する
			createDeviceFlags,		// デバイスフラグ
			featureLevels,			// 機能レベル
			numFeatureLevels,		// 機能レベル数
			D3D11_SDK_VERSION,		// 
			&sd,					// スワップチェインの設定
			&g_pSwapChain,			// IDXGIDwapChainインタフェース	
			&g_pDevice,				// ID3D11Deviceインタフェース
			&featureLevel,		// サポートされている機能レベル
			&g_pContext);		// デバイスコンテキスト
		if (SUCCEEDED(hr)) {
			break;
		}
	}
	if (FAILED(hr)) {
		return hr;
	}


	//--- ラスタライズ
	D3D11_RASTERIZER_DESC rasterDesc = {};
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.CullMode = D3D11_CULL_NONE;
	hr = g_pDevice->CreateRasterizerState(&rasterDesc, &g_pRasterizer[CULL_NONE]);
	if (FAILED(hr)) { return hr; }
	rasterDesc.CullMode = D3D11_CULL_FRONT;
	hr = g_pDevice->CreateRasterizerState(&rasterDesc, &g_pRasterizer[CULL_FRONT]);
	if (FAILED(hr)) { return hr; }
	rasterDesc.CullMode = D3D11_CULL_BACK;
	hr = g_pDevice->CreateRasterizerState(&rasterDesc, &g_pRasterizer[CULL_BACK]);
	if (FAILED(hr)) { return hr; }
	SetCulling(CULL_NONE);

	return S_OK;
}

void UninitDX()
{
	SAFE_RELEASE(g_pRasterizer[0]);
	SAFE_RELEASE(g_pRasterizer[1]);

	g_pContext->ClearState();
	SAFE_RELEASE(g_pContext);

	g_pSwapChain->SetFullscreenState(false, NULL);
	SAFE_RELEASE(g_pSwapChain);

	SAFE_RELEASE(g_pDevice);
}

ID3D11Device *GetDevice()
{
	return g_pDevice;
}

ID3D11DeviceContext *GetContext()
{
	return g_pContext;
}
IDXGISwapChain* GetSwapChain()
{
	return g_pSwapChain;
}
void SetCulling(CullingMode cull)
{
	g_pContext->RSSetState(g_pRasterizer[cull]);
}


void Graphics::Init()
{
	// レンダーターゲット
	m_pDefRenderTarget = TextureFactory::CreateRenderTargetFromScreen();
	UINT width = m_pDefRenderTarget->GetWidth();
	UINT height = m_pDefRenderTarget->GetHeight();
	m_pDefDepthStencil = TextureFactory::CreateDepthStencil(width, height, false);
	// ブレンドステート初期化
	D3D11_RENDER_TARGET_BLEND_DESC blendDesc = {};
	blendDesc.BlendEnable = TRUE;
	blendDesc.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	blendDesc.SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.BlendOpAlpha = D3D11_BLEND_OP_ADD;
	// 通常
	m_pDefBlendState[BLEND_ALPHA] = new BlendState;
	m_pDefBlendState[BLEND_ALPHA]->Create(blendDesc);
	// 加算
	blendDesc.DestBlend = D3D11_BLEND_ONE;
	m_pDefBlendState[BLEND_ADD] = new BlendState;
	m_pDefBlendState[BLEND_ADD]->Create(blendDesc);

	// サンプラー初期化
	m_pDefSamplerState[SAMPLER_POINT] = new SamplerState;
	m_pDefSamplerState[SAMPLER_POINT]->Create(D3D11_FILTER_MIN_MAG_MIP_POINT, D3D11_TEXTURE_ADDRESS_WRAP);
	m_pDefSamplerState[SAMPLER_LINEAR] = new SamplerState;
	m_pDefSamplerState[SAMPLER_LINEAR]->Create(D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_WRAP);
	// デプスステンシル初期化
	m_pDefDepthStencilState[DEPTHSTENCIL_OFF] = new DepthStencilState;
	m_pDefDepthStencilState[DEPTHSTENCIL_OFF]->Create(false, false);
	m_pDefDepthStencilState[DEPTHSTENCIL_ON] = new DepthStencilState;
	m_pDefDepthStencilState[DEPTHSTENCIL_ON]->Create(true, false);

	// 初期値設定
	m_pDepthStencilView = m_pDefDepthStencil;
	SetRenderTargetDefault();
	SetDepthStencilViewDefault();
	SetBlendState(BLEND_ALPHA);
	SetSamplerState(SAMPLER_LINEAR);
	SetDepthStencilState(DEPTHSTENCIL_ON);
}
void Graphics::Uninit()
{
	SAFE_DELETE(m_pDefDepthStencilState[DEPTHSTENCIL_OFF]);
	SAFE_DELETE(m_pDefDepthStencilState[DEPTHSTENCIL_ON]);
	SAFE_DELETE(m_pDefSamplerState[SAMPLER_LINEAR]);
	SAFE_DELETE(m_pDefSamplerState[SAMPLER_POINT]);
	SAFE_DELETE(m_pDefBlendState[BLEND_ALPHA]);
	SAFE_DELETE(m_pDefBlendState[BLEND_ADD]);
	SAFE_DELETE(m_pDefDepthStencil);
	SAFE_DELETE(m_pDefRenderTarget);
}
void Graphics::BeginDraw()
{
	float color[4] = { 0.8f, 0.8f, 0.9f, 1.0f };
	ID3D11RenderTargetView* pRTV = reinterpret_cast<RenderTarget*>(m_pDefRenderTarget)->GetView();
	ID3D11DepthStencilView* pDSV = reinterpret_cast<DepthStencil*>(m_pDefDepthStencil)->GetView();
	g_pContext->ClearRenderTargetView(pRTV, color);
	g_pContext->ClearDepthStencilView(pDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void Graphics::EndDraw()
{
	g_pSwapChain->Present(0, 0);
}
void Graphics::SetRenderTarget(Texture** ppRenderTarget, UINT numView, float* pClearColor)
{
	// 更新チェック
	if (!ppRenderTarget || !ppRenderTarget[0])
	{
		return;
	}

	// レンダーターゲット更新
	m_renderTargetNum = min(numView, 4);
	for (UINT i = 0; i < m_renderTargetNum; ++i)
	{
		m_pRenderTarget[i] = ppRenderTarget[i];
	}
	UpdateTargetView();

	// ビューポート設定
	D3D11_VIEWPORT vp;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	vp.Width = static_cast<FLOAT>(m_pRenderTarget[0]->GetWidth());
	vp.Height = static_cast<FLOAT>(m_pRenderTarget[0]->GetHeight());
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	g_pContext->RSSetViewports(1, &vp);

	// クリア
	if (pClearColor)
	{
		for (UINT i = 0; i < m_renderTargetNum; ++i)
		{
			ID3D11RenderTargetView* pRTV = reinterpret_cast<RenderTarget*>(m_pRenderTarget[i])->GetView();
			g_pContext->ClearRenderTargetView(pRTV, pClearColor);
		}
	}
}
void Graphics::SetRenderTargetDefault(float* pClearColor)
{
	SetRenderTarget(&m_pDefRenderTarget, 1, pClearColor);
}
void Graphics::SetDepthStencilView(Texture* pDepthStencilView, bool isClear)
{
	// 更新チェック
	if (!pDepthStencilView || m_pDepthStencilView == pDepthStencilView)
	{
		return;
	}

	// 深度バッファ設定
	m_pDepthStencilView = pDepthStencilView;
	UpdateTargetView();

	// バッファクリア
	if (isClear)
	{
		DepthStencil* pDSV = reinterpret_cast<DepthStencil*>(m_pDepthStencilView);
		g_pContext->ClearDepthStencilView(
			pDSV->GetView(),
			D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
			// 深度値を初期化した時の値
			// カメラが映せる範囲は最終的に0~1の範囲に丸められるため、1が一番奥になる。
			1.0f,
			0
		);
	}
}
void Graphics::SetDepthStencilViewDefault(bool isClear)
{
	SetDepthStencilView(m_pDefDepthStencil, isClear);
}
void Graphics::SetBlendState(BlendStateKind kind)
{
	m_pDefBlendState[kind]->Bind();
}
void Graphics::SetBlendState(BlendState* pState)
{
	m_pBlendState = pState;
}
BlendState* Graphics::GetBlendState()
{
	return m_pBlendState;
}
void Graphics::SetSamplerState(SamplerStateKind kind)
{
	m_pDefSamplerState[kind]->Bind();
}
void Graphics::SetSamplerState(SamplerState* pState)
{
	m_pSamplerState = pState;
}
SamplerState* Graphics::GetSamplerState()
{
	return m_pSamplerState;
}
void Graphics::SetDepthStencilState(DepthStencilKind kind)
{
	m_pDefDepthStencilState[kind]->Bind();
}
void Graphics::SetDepthStencilState(DepthStencilState* pState)
{
	m_pDepthStencilState = pState;
}
DepthStencilState* Graphics::GetDepthStencilState()
{
	return m_pDepthStencilState;
}

void Graphics::UpdateTargetView()
{
	// レンダーターゲット取得
	ID3D11RenderTargetView* pRTV[4] = {};
	for (UINT i = 0; i < m_renderTargetNum && i < 4; ++i)
	{
		pRTV[i] = reinterpret_cast<RenderTarget*>(m_pRenderTarget[i])->GetView();
	}
	// 深度ステンシル取得
	DepthStencil* pDSV = reinterpret_cast<DepthStencil*>(m_pDepthStencilView);
	// 設定
	g_pContext->OMSetRenderTargets(m_renderTargetNum, pRTV, pDSV->GetView());
}