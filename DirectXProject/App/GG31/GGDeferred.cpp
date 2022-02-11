#include <App/GG31/GGDeferred.h>
#include <App/GG31/GGLevelMain.h>
#include <System/Scene/SceneManager.hpp>
#include <System/Macro.h>
#include <System/Camera/CameraManager.hpp>
#include <System/Camera/Camera2D.h>
#include <System/Texture/TextureFactory.h>
#include <System/Graphics/Graphics.h>

struct PointLightInfo
{
	DirectX::XMFLOAT3 pos;
	float range;
	DirectX::XMFLOAT4 color;
};

/* GBufferとは？
* 事前に書き出しておくテクスチャ
* 表現次第でほかの値も書き込む
*/
enum GBufferKind
{
	GBUF_ALBEDO,
	GBUF_NORMAL,
	GBUF_DEPTH,
	GBUF_MAX
};

HRESULT GGDeferred::Load()
{
	// 定数バッファ
	m_pMatrix = new ConstantBuffer;
	m_pMatrix->Create(sizeof(DirectX::XMFLOAT4X4) * 3);
	m_pColorBuf = new ConstantBuffer;
	m_pColorBuf->Create(sizeof(DirectX::XMFLOAT4));
	m_pLightBuf = new ConstantBuffer;
	m_pLightBuf->Create(sizeof(PointLightInfo) * LIGHT_NUM);
	m_pInvVPS = new ConstantBuffer;
	m_pInvVPS->Create(sizeof(DirectX::XMFLOAT4X4));

	// テクスチャ
	UINT width = 1280;
	UINT height = 720;

	m_ppRenderTarget = new Texture* [GBUF_MAX];

	// アルベド
	m_ppRenderTarget[GBUF_ALBEDO] = TextureFactory::CreateRenderTarget(DXGI_FORMAT_R8G8B8A8_UNORM, width, height);
	// 法線
	m_ppRenderTarget[GBUF_NORMAL] = TextureFactory::CreateRenderTarget(DXGI_FORMAT_R11G11B10_FLOAT, width, height);
	// 深度値
	m_ppRenderTarget[GBUF_DEPTH] = TextureFactory::CreateRenderTarget(DXGI_FORMAT_R32_FLOAT, width, height);

	// ライト
	for (int i = 0; i < LIGHT_NUM; ++i)
	{
		m_pLight[i] = new Light(Light::POINT_LIGHT);
		// ライトの初期値
		m_pLight[i]->SetPos(DirectX::XMFLOAT3(
			(rand() % 21 - 10) * 0.15f,
			(rand() % 11) * 0.05f,
			(rand() % 21 - 10) * 0.15f
		));
		m_pLight[i]->SetRange(
			(rand() % 6) * 0.3f + 0.5f
		);
		m_pLight[i]->SetColor(DirectX::XMFLOAT4(
			(rand() % 11) * 0.1f, (rand() % 11) * 0.1f, (rand() % 11) * 0.1f,
			1.0f
		));
	}

	return S_OK;
}
void GGDeferred::Release()
{
	for (int i = 0; i < LIGHT_NUM; ++i)
	{
		SAFE_DELETE(m_pLight[i]);
	}
	for (int i = 0; i < 3; ++i)
	{
		SAFE_DELETE(m_ppRenderTarget[i]);
	}
	SAFE_DELETE_ARRAY(m_ppRenderTarget);
	SAFE_DELETE(m_pInvVPS);
	SAFE_DELETE(m_pLightBuf);
	SAFE_DELETE(m_pColorBuf);
	SAFE_DELETE(m_pMatrix);
}
void GGDeferred::Update(float tick)
{
}
void GGDeferred::Draw()
{
	// メイン
	GGLevelMain* pMain = SCENE->GetScene<GGLevelMain>(0);
	// カメラ
	Camera* pCamera = CAMERA->Get();
	DirectX::XMFLOAT4X4 mat[3];
	DirectX::XMStoreFloat4x4(&mat[0], DirectX::XMMatrixIdentity());
	DirectX::XMStoreFloat4x4(&mat[1], DirectX::XMMatrixTranspose(pCamera->GetLookAtMatrix()));
	DirectX::XMStoreFloat4x4(&mat[2], DirectX::XMMatrixTranspose(pCamera->GetProjectionMatrix()));
	m_pMatrix->BindVS(0);

	//--- ディファードレンダリング
	float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	GRAPHICS->SetRenderTarget(m_ppRenderTarget, GBUF_MAX, clearColor);
	{
		// 地面を描画
		m_pMatrix->Write(mat);
		GetVertexShader(DEFERRED_WRITE_VS)->Bind();
		GetPixelShader(DEFERRED_WRITE_PS)->Bind();
		pMain->DrawPlane();
	}
	GRAPHICS->SetRenderTargetDefault();

	// GBufferの内容を元に2Dを表示
	// 行列の設定
	pCamera = CAMERA->Get<Camera2D>();
	DirectX::XMStoreFloat4x4(&mat[0], DirectX::XMMatrixTranspose(
		DirectX::XMMatrixScaling(127.f, 1.0f, -71.0f) *
		DirectX::XMMatrixRotationX(-DirectX::XM_PIDIV2) *
		DirectX::XMMatrixTranslation(640.f, 360.f, 0.0f)
	));
	DirectX::XMStoreFloat4x4(&mat[1], DirectX::XMMatrixTranspose(pCamera->GetLookAtMatrix()));
	DirectX::XMStoreFloat4x4(&mat[2], DirectX::XMMatrixTranspose(pCamera->GetProjectionMatrix()));
	m_pMatrix->Write(mat);
	// 逆行列
	DirectX::XMMATRIX screenMat = DirectX::XMMatrixSet(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f
	);
	pCamera = CAMERA->Get();
	DirectX::XMFLOAT4X4 invVPS;
	DirectX::XMStoreFloat4x4(&invVPS, DirectX::XMMatrixTranspose(
		DirectX::XMMatrixInverse(nullptr,
			pCamera->GetLookAtMatrix() * pCamera->GetProjectionMatrix() * screenMat
		)
	));
	m_pInvVPS->Write(&invVPS);
	m_pInvVPS->BindPS(1);
	// ライトの設定
	PointLightInfo lights[LIGHT_NUM];
	for (int i = 0; i < LIGHT_NUM; ++i)
	{
		lights[i].pos = m_pLight[i]->GetPos();
		lights[i].color = m_pLight[i]->GetColor();
		lights[i].range = m_pLight[i]->GetRange();
	}
	m_pLightBuf->Write(lights);
	m_pLightBuf->BindPS(0);
	// 描画
	DepthStencilState* pOldDS = GRAPHICS->GetDepthStencilState();
	GRAPHICS->SetDepthStencilState(Graphics::DEPTHSTENCIL_OFF);
	GetVertexShader(SAMPLE_VS)->Bind();
	GetPixelShader(DEFERRED_PS)->Bind();
	SetTexturePS(m_ppRenderTarget[GBUF_ALBEDO], 0);
	SetTexturePS(m_ppRenderTarget[GBUF_NORMAL], 1);
	SetTexturePS(m_ppRenderTarget[GBUF_DEPTH], 2);
	
	
	pMain->DrawPlane();
	GRAPHICS->SetDepthStencilState(Graphics::DEPTHSTENCIL_ON);

	// ライト描画
	GetVertexShader(SAMPLE_VS)->Bind();
	GetPixelShader(OBJECT_COLOR_PS)->Bind();
	pCamera = CAMERA->Get();
	DirectX::XMStoreFloat4x4(&mat[1], DirectX::XMMatrixTranspose(pCamera->GetLookAtMatrix()));
	DirectX::XMStoreFloat4x4(&mat[2], DirectX::XMMatrixTranspose(pCamera->GetProjectionMatrix()));
	m_pColorBuf->BindPS(0);
	for (int i = 0; i < LIGHT_NUM; ++i)
	{
		// 球を描画
		DirectX::XMFLOAT3 pos = lights[i].pos;
		DirectX::XMFLOAT4 color = lights[i].color;
		DirectX::XMStoreFloat4x4(&mat[0], DirectX::XMMatrixTranspose(
			DirectX::XMMatrixScaling(0.1f, 0.1f, 0.1f) *
			DirectX::XMMatrixTranslation(pos.x, pos.y, pos.z)));
		m_pMatrix->Write(mat);
		m_pColorBuf->Write(&color);
		pMain->DrawSphere();
	}
}