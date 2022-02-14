#include <App/GG31/GGBlur.h>
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

enum GBufferKind
{
	GBUF_ALBEDO,
	GBUF_NORMAL,
	GBUF_DEPTH,
	GBUF_MAX
};
// ぼかし処理の画像
enum BlurTextureKind
{
	BLUR_BRIGHTNESS,	// 輝度を抽出した画像
	BLUR_X,				// 横方向にぼこしをかけた画像
	BLUR_Y,				// 縦方向にぼこしをかけた画像
	BLUR_MAX,
};

HRESULT GGBlur::Load()
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
	m_pBlurParam = new ConstantBuffer;
	m_pBlurParam->Create(sizeof(DirectX::XMFLOAT4));

	// GBuffer描き込み用のテクスチャ
	UINT width = 1280;
	UINT height = 720;
	DXGI_FORMAT format[GBUF_MAX] =
	{
		DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R11G11B10_FLOAT, DXGI_FORMAT_R32_FLOAT
	};
	m_ppRenderTarget = new Texture * [GBUF_MAX];
	for (int i = 0; i < GBUF_MAX; ++i)
	{
		m_ppRenderTarget[i] = TextureFactory::CreateRenderTarget(format[i], width, height);
	}

	// ブラー処理用のテクスチャ
	m_ppBlurTexture = new Texture * [BLUR_MAX];

	// ぼかしの段階で画像サイズが違う
	UINT blurTexsize[][2] =
	{
		{width,height},{width * 0.5f,height},{width * 0.5,height * 0.5},

	};

	for (int i = 0; i < BLUR_MAX; ++i)
	{
		//m_ppRenderTarget[i] = TextureFactory::CreateRenderTarget(DXGI_FORMAT_R8G8B8A8_UNORM, blurTexsize[i][0], blurTexsize[i][1]);
		m_ppBlurTexture[i] = TextureFactory::CreateRenderTarget(DXGI_FORMAT_R8G8B8A8_UNORM, blurTexsize[i][0], blurTexsize[i][1]);
	}

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




void GGBlur::Release()
{
	for (int i = 0; i < LIGHT_NUM; ++i)
	{
		SAFE_DELETE(m_pLight[i]);
	}
	for (int i = 0; i < BLUR_MAX; ++i)
	{
		SAFE_DELETE(m_ppBlurTexture[i]);
	}
	SAFE_DELETE_ARRAY(m_ppBlurTexture);
	for (int i = 0; i < GBUF_MAX; ++i)
	{
		SAFE_DELETE(m_ppRenderTarget[i]);
	}
	SAFE_DELETE_ARRAY(m_ppRenderTarget);
	SAFE_DELETE(m_pBlurParam);
	SAFE_DELETE(m_pInvVPS);
	SAFE_DELETE(m_pLightBuf);
	SAFE_DELETE(m_pColorBuf);
	SAFE_DELETE(m_pMatrix);
}
void GGBlur::Update(float tick)
{
}
void GGBlur::Draw()
{
	// メイン
	GGLevelMain* pMain = SCENE->GetScene<GGLevelMain>(0);
	// カメラ
	Camera* pCamera3D = CAMERA->Get();
	DirectX::XMFLOAT4X4 mat3D[3];
	DirectX::XMStoreFloat4x4(&mat3D[0], DirectX::XMMatrixIdentity());
	DirectX::XMStoreFloat4x4(&mat3D[1], DirectX::XMMatrixTranspose(pCamera3D->GetLookAtMatrix()));
	DirectX::XMStoreFloat4x4(&mat3D[2], DirectX::XMMatrixTranspose(pCamera3D->GetProjectionMatrix()));
	Camera* pCamera2D = CAMERA->Get<Camera2D>();
	DirectX::XMFLOAT4X4 mat2D[3];
	DirectX::XMStoreFloat4x4(&mat2D[0], DirectX::XMMatrixIdentity());
	DirectX::XMStoreFloat4x4(&mat2D[1], DirectX::XMMatrixTranspose(pCamera2D->GetLookAtMatrix()));
	DirectX::XMStoreFloat4x4(&mat2D[2], DirectX::XMMatrixTranspose(pCamera2D->GetProjectionMatrix()));
	m_pMatrix->BindVS(0);

	//--- ディファードレンダリング
	float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	GRAPHICS->SetRenderTarget(m_ppRenderTarget, GBUF_MAX, clearColor);
	{
		// 地面を描画
		m_pMatrix->Write(mat3D);
		GetVertexShader(DEFERRED_WRITE_VS)->Bind();
		GetPixelShader(DEFERRED_WRITE_PS)->Bind();
		pMain->DrawPlane();
	}
	GRAPHICS->SetRenderTargetDefault();

	// GBufferの内容を元に2Dを表示
	// 行列の設定
	DirectX::XMStoreFloat4x4(&mat2D[0], DirectX::XMMatrixTranspose(
		DirectX::XMMatrixScaling(127.f, 1.0f, -71.0f) *
		DirectX::XMMatrixRotationX(-DirectX::XM_PIDIV2) *
		DirectX::XMMatrixTranslation(640.f, 360.f, 0.0f)
	));
	m_pMatrix->Write(mat2D);
	// 深度値(2D)からワールド座標(3D)を求めるための逆行列
	DirectX::XMMATRIX screenMat = DirectX::XMMatrixSet(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f
	);
	DirectX::XMFLOAT4X4 invVPS;
	DirectX::XMStoreFloat4x4(&invVPS, DirectX::XMMatrixTranspose(
		DirectX::XMMatrixInverse(nullptr,
			pCamera3D->GetLookAtMatrix() * pCamera3D->GetProjectionMatrix() * screenMat
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
	GRAPHICS->SetDepthStencilState(Graphics::DEPTHSTENCIL_OFF);
	GetVertexShader(SAMPLE_VS)->Bind();
	GetPixelShader(DEFERRED_PS)->Bind();
	SetTexturePS(m_ppRenderTarget[GBUF_ALBEDO], 0);
	SetTexturePS(m_ppRenderTarget[GBUF_NORMAL], 1);
	SetTexturePS(m_ppRenderTarget[GBUF_DEPTH], 2);
	pMain->DrawPlane();
	GRAPHICS->SetDepthStencilState(Graphics::DEPTHSTENCIL_ON);



	// ブラー処理を加えてライトを描画
	GetVertexShader(SAMPLE_VS)->Bind();
	GetPixelShader(OBJECT_COLOR_PS)->Bind();
	m_pColorBuf->BindPS(0);


	// ①テクスチャへ明るい部分の情報を描き込む
	// ライト　＝　明るいはずので、球をそにまま描き込む
	GRAPHICS->SetRenderTarget(&m_ppBlurTexture[BLUR_BRIGHTNESS], 1, clearColor);
	{
		for (int i = 0; i < LIGHT_NUM; ++i)
		{
			// 球を描画
			DirectX::XMFLOAT3 pos = lights[i].pos;
			DirectX::XMFLOAT4 color = lights[i].color;
			DirectX::XMStoreFloat4x4(&mat3D[0], DirectX::XMMatrixTranspose(
				DirectX::XMMatrixScaling(0.1f, 0.1f, 0.1f) *
				DirectX::XMMatrixTranslation(pos.x, pos.y, pos.z)));
			m_pMatrix->Write(mat3D);
			m_pColorBuf->Write(&color);
			pMain->DrawSphere();
		}
	}
	// ブラーを適用する前に2Dの描画設定
	GRAPHICS->SetDepthStencilState(Graphics::DEPTHSTENCIL_OFF); // 2Dの描画になるのでデプスステンシルを無効
	DirectX::XMStoreFloat4x4(&mat2D[0], DirectX::XMMatrixTranspose(
		DirectX::XMMatrixScaling(128.f, 1.0f, -72.0f) *
		DirectX::XMMatrixRotationX(-DirectX::XM_PIDIV2) *
		DirectX::XMMatrixTranslation(640.f, 360.f, 0.0f)
	));
	m_pMatrix->Write(mat2D);	
	m_pBlurParam->BindPS(0);

	// ②横
	GRAPHICS->SetRenderTarget(&m_ppBlurTexture[BLUR_X], 1, clearColor);
	{
		DirectX::XMFLOAT4 blurParam(640.0f, 720.0f,1.0f, 0.0f);

		m_pBlurParam->Write(&blurParam);
		GetPixelShader(BLUR_PS)->Bind();
		SetTexturePS(m_ppBlurTexture[BLUR_BRIGHTNESS]);
		pMain->DrawPlane();
	}
	// ③横
	GRAPHICS->SetRenderTarget(&m_ppBlurTexture[BLUR_Y], 1, clearColor);
	{
		DirectX::XMFLOAT4 blurParam(640.0f, 360.0f, 1.0f, 0.0f);

		m_pBlurParam->Write(&blurParam);
		GetPixelShader(BLUR_PS)->Bind();
		SetTexturePS(m_ppBlurTexture[BLUR_X]);
		pMain->DrawPlane();
	}

	// ④ぼかし終了画像に加算合成を加える
	GRAPHICS->SetRenderTargetDefault();
	GRAPHICS->SetBlendState(Graphics::BLEND_ADD);
	GetPixelShader(SAMPLE_PS)->Bind();
	SetTexturePS(m_ppBlurTexture[BLUR_Y]);
	pMain->DrawPlane();
	GRAPHICS->SetBlendState(Graphics::BLEND_ALPHA);

	GRAPHICS->SetDepthStencilState(Graphics::DEPTHSTENCIL_ON);
}