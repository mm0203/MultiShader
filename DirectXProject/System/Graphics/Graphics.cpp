#include <System/Graphics/Graphics.h>
#include <System/Macro.h>
#include <System/Texture/TextureFactory.h>
#include <System/Texture/DepthStencil.h>
#include <System/Texture/RenderTarget.h>

//--- �O���[�o���ϐ�
ID3D11Device *g_pDevice;
ID3D11DeviceContext *g_pContext;
IDXGISwapChain *g_pSwapChain;

ID3D11RasterizerState* g_pRasterizer[CULL_MAX];


HRESULT InitDX(HWND hWnd, UINT width, UINT height, bool fullscreen)
{
	HRESULT	hr = E_FAIL;

	// �X���b�v�`�F�C���̐ݒ�
	// �X���b�v�`�F�C���Ƃ́A�E�C���h�E�ւ̕\���_�u���o�b�t�@���Ǘ�����
	// �}���`�T���v�����O�A���t���b�V�����[�g���ݒ�ł���
	// �����̃o�b�N�o�b�t�@���쐬�ł���
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));					// �[���N���A
	sd.BufferDesc.Width = width;						// �o�b�N�o�b�t�@�̕�
	sd.BufferDesc.Height = height;						// �o�b�N�o�b�t�@�̍���
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;	// �o�b�N�o�b�t�@�t�H�[�}�b�g(R,G,B,A)
	sd.SampleDesc.Count = 1;		// �}���`�T���v���̐�
	sd.BufferDesc.RefreshRate.Numerator = 1000;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;	// �o�b�N�o�b�t�@�̎g�p���@
	sd.BufferCount = 1;									// �o�b�N�o�b�t�@�̐�
	sd.OutputWindow = hWnd;			// �֘A�t����E�C���h�E
	sd.Windowed = fullscreen ? FALSE : TRUE;
	//sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	
	// �h���C�o�̎��
	D3D_DRIVER_TYPE driverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE,	// GPU�ŕ`��
		D3D_DRIVER_TYPE_WARP,		// �����x(�ᑬ
		D3D_DRIVER_TYPE_REFERENCE,	// CPU�ŕ`��
	};
	UINT numDriverTypes = ARRAYSIZE(driverTypes);

	UINT createDeviceFlags = 0;
	// createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
	// �@�\���x��
	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_1,		// DirectX11.1�Ή�GPU���x��
		D3D_FEATURE_LEVEL_11_0,		// DirectX11�Ή�GPU���x��
		D3D_FEATURE_LEVEL_10_1,		// DirectX10.1�Ή�GPU���x��
		D3D_FEATURE_LEVEL_10_0,		// DirectX10�Ή�GPU���x��
		D3D_FEATURE_LEVEL_9_3,		// DirectX9.3�Ή�GPU���x��
		D3D_FEATURE_LEVEL_9_2,		// DirectX9.2�Ή�GPU���x��
		D3D_FEATURE_LEVEL_9_1		// Direct9.1�Ή�GPU���x��
	};
	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

	D3D_DRIVER_TYPE driverType;
	D3D_FEATURE_LEVEL featureLevel;

	for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; ++driverTypeIndex)
	{
		driverType = driverTypes[driverTypeIndex];
		hr = D3D11CreateDeviceAndSwapChain(
			NULL,					// �f�B�X�v���C�f�o�C�X�̃A�_�v�^�iNULL�̏ꍇ�ŏ��Ɍ��������A�_�v�^�j
			driverType,				// �f�o�C�X�h���C�o�̃^�C�v
			NULL,					// �\�t�g�E�F�A���X�^���C�U���g�p����ꍇ�Ɏw�肷��
			createDeviceFlags,		// �f�o�C�X�t���O
			featureLevels,			// �@�\���x��
			numFeatureLevels,		// �@�\���x����
			D3D11_SDK_VERSION,		// 
			&sd,					// �X���b�v�`�F�C���̐ݒ�
			&g_pSwapChain,			// IDXGIDwapChain�C���^�t�F�[�X	
			&g_pDevice,				// ID3D11Device�C���^�t�F�[�X
			&featureLevel,		// �T�|�[�g����Ă���@�\���x��
			&g_pContext);		// �f�o�C�X�R���e�L�X�g
		if (SUCCEEDED(hr)) {
			break;
		}
	}
	if (FAILED(hr)) {
		return hr;
	}


	//--- ���X�^���C�Y
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
	// �����_�[�^�[�Q�b�g
	m_pDefRenderTarget = TextureFactory::CreateRenderTargetFromScreen();
	UINT width = m_pDefRenderTarget->GetWidth();
	UINT height = m_pDefRenderTarget->GetHeight();
	m_pDefDepthStencil = TextureFactory::CreateDepthStencil(width, height, false);
	// �u�����h�X�e�[�g������
	D3D11_RENDER_TARGET_BLEND_DESC blendDesc = {};
	blendDesc.BlendEnable = TRUE;
	blendDesc.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	blendDesc.SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.BlendOpAlpha = D3D11_BLEND_OP_ADD;
	// �ʏ�
	m_pDefBlendState[BLEND_ALPHA] = new BlendState;
	m_pDefBlendState[BLEND_ALPHA]->Create(blendDesc);
	// ���Z
	blendDesc.DestBlend = D3D11_BLEND_ONE;
	m_pDefBlendState[BLEND_ADD] = new BlendState;
	m_pDefBlendState[BLEND_ADD]->Create(blendDesc);

	// �T���v���[������
	m_pDefSamplerState[SAMPLER_POINT] = new SamplerState;
	m_pDefSamplerState[SAMPLER_POINT]->Create(D3D11_FILTER_MIN_MAG_MIP_POINT, D3D11_TEXTURE_ADDRESS_WRAP);
	m_pDefSamplerState[SAMPLER_LINEAR] = new SamplerState;
	m_pDefSamplerState[SAMPLER_LINEAR]->Create(D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_WRAP);
	// �f�v�X�X�e���V��������
	m_pDefDepthStencilState[DEPTHSTENCIL_OFF] = new DepthStencilState;
	m_pDefDepthStencilState[DEPTHSTENCIL_OFF]->Create(false, false);
	m_pDefDepthStencilState[DEPTHSTENCIL_ON] = new DepthStencilState;
	m_pDefDepthStencilState[DEPTHSTENCIL_ON]->Create(true, false);

	// �����l�ݒ�
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
	// �X�V�`�F�b�N
	if (!ppRenderTarget || !ppRenderTarget[0])
	{
		return;
	}

	// �����_�[�^�[�Q�b�g�X�V
	m_renderTargetNum = min(numView, 4);
	for (UINT i = 0; i < m_renderTargetNum; ++i)
	{
		m_pRenderTarget[i] = ppRenderTarget[i];
	}
	UpdateTargetView();

	// �r���[�|�[�g�ݒ�
	D3D11_VIEWPORT vp;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	vp.Width = static_cast<FLOAT>(m_pRenderTarget[0]->GetWidth());
	vp.Height = static_cast<FLOAT>(m_pRenderTarget[0]->GetHeight());
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	g_pContext->RSSetViewports(1, &vp);

	// �N���A
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
	// �X�V�`�F�b�N
	if (!pDepthStencilView || m_pDepthStencilView == pDepthStencilView)
	{
		return;
	}

	// �[�x�o�b�t�@�ݒ�
	m_pDepthStencilView = pDepthStencilView;
	UpdateTargetView();

	// �o�b�t�@�N���A
	if (isClear)
	{
		DepthStencil* pDSV = reinterpret_cast<DepthStencil*>(m_pDepthStencilView);
		g_pContext->ClearDepthStencilView(
			pDSV->GetView(),
			D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
			// �[�x�l���������������̒l
			// �J�������f����͈͍͂ŏI�I��0~1�͈̔͂Ɋۂ߂��邽�߁A1����ԉ��ɂȂ�B
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
	// �����_�[�^�[�Q�b�g�擾
	ID3D11RenderTargetView* pRTV[4] = {};
	for (UINT i = 0; i < m_renderTargetNum && i < 4; ++i)
	{
		pRTV[i] = reinterpret_cast<RenderTarget*>(m_pRenderTarget[i])->GetView();
	}
	// �[�x�X�e���V���擾
	DepthStencil* pDSV = reinterpret_cast<DepthStencil*>(m_pDepthStencilView);
	// �ݒ�
	g_pContext->OMSetRenderTargets(m_renderTargetNum, pRTV, pDSV->GetView());
}