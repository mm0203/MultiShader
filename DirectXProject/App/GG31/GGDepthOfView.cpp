#include <App/GG31/GGDepthOfView.h>
#include <App/GG31/GGLevelMain.h>
#include <System/Scene/SceneManager.hpp>
#include <System/Macro.h>
#include <System/Camera/CameraManager.hpp>
#include <System/Camera/Camera2D.h>
#include <System/Texture/TextureFactory.h>
#include <System/Graphics/Graphics.h>


enum DOVTextureKind
{
	DOV_RENDER_TARGET, // ぼかす絵
	DOV_DEPTH,	// 深度値
	DOV_X_BLUR,	// 横ぼかし
	DOV_Y_BLUR,	// 縦ぼかし
	DOV_BLUR,
};

HRESULT GGDepthOfView::Load()
{
	// 定数バッファ
	m_pMatrix = new ConstantBuffer;
	m_pMatrix->Create(sizeof(DirectX::XMFLOAT4X4) * 3);
	m_pColorBuf = new ConstantBuffer;
	m_pColorBuf->Create(sizeof(DirectX::XMFLOAT4));
	m_pBlurParam = new ConstantBuffer;
	m_pBlurParam->Create(sizeof(DirectX::XMFLOAT4));
	m_pInvVPS = new ConstantBuffer;
	m_pInvVPS->Create(sizeof(DirectX::XMFLOAT4X4));
	m_pDOVParam = new ConstantBuffer;
	m_pDOVParam->Create(sizeof(DirectX::XMFLOAT4) * 2);

	// ブラー処理用のテクスチャ
	UINT width = 1280;
	UINT height = 720;
	UINT blurTexSize[][2] = { {width, height}, {width, height}, {width * 0.5f, height}, {width * 0.5f, height * 0.5f} };
	DXGI_FORMAT format[] = { DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R32_FLOAT, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM };
	m_ppDOVTexture = new Texture*[DOV_BLUR];
	for (int i = 0; i < DOV_BLUR; ++i)
	{
		m_ppDOVTexture[i] = TextureFactory::CreateRenderTarget(format[i], blurTexSize[i][0], blurTexSize[i][1]);
	}

	// オブジェクト
	for (int i = 0; i < OBJECT_NUM; ++i)
	{
		float scale = (rand() % 11) * 0.05f + 0.5f;
		DirectX::XMStoreFloat4x4(
			&m_world[i],
			DirectX::XMMatrixTranspose(
				DirectX::XMMatrixScaling(scale, scale, scale) *
				DirectX::XMMatrixTranslation(
					(rand() % 21 - 10) * 0.4f,
					(rand() % 11) * 0.3f,
					(rand() % 21 - 10) * 0.4f
				)
			)
		);
		m_color[i] = DirectX::XMFLOAT4(
			(rand() % 11) * 0.1f, (rand() % 11) * 0.1f, (rand() % 11) * 0.1f,
			1.0f
		);
	}

	return S_OK;
}


void GGDepthOfView::Release()
{
	for (int i = 0; i < DOV_BLUR; ++i)
	{
		SAFE_DELETE(m_ppDOVTexture[i]);
	}
	SAFE_DELETE_ARRAY(m_ppDOVTexture);
	SAFE_DELETE(m_pDOVParam);
	SAFE_DELETE(m_pInvVPS);
	SAFE_DELETE(m_pBlurParam);
	SAFE_DELETE(m_pColorBuf);
	SAFE_DELETE(m_pMatrix);
}
void GGDepthOfView::Update(float tick)
{
}
void GGDepthOfView::Draw()
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


	// 描画
	float clearColor[] = {0,0,0,1};
	DirectX::XMFLOAT4 objColor(1,1,1,1);
	
	// ぼかし処理を行うオブジェクトをテクスチャに描き込む
	GRAPHICS->SetRenderTarget(&m_ppDOVTexture[DOV_RENDER_TARGET], 2, clearColor);
	{
		GetVertexShader(DEFERRED_WRITE_VS)->Bind();
		GetPixelShader(DEPTH_OF_VIEW_WRITE_PS)->Bind();
		m_pMatrix->BindVS(0);
		m_pColorBuf->BindPS(0);

		// 平面
		m_pMatrix->Write(mat3D);
		m_pColorBuf->Write(&objColor);
		pMain->DrawPlane();
		// 球
		for (int i = 0; i < OBJECT_NUM; ++i)
		{
			mat3D[0] = m_world[i];
			m_pMatrix->Write(mat3D);
			objColor = m_color[i];
			m_pColorBuf->Write(&objColor);
			pMain->DrawSphere();
		}
	}

	// ブラー
	DirectX::XMFLOAT4 blurParam;
	m_pBlurParam->BindPS(0);
	// ブラーを適用する前に2Dの描画設定
	GRAPHICS->SetDepthStencilState(Graphics::DEPTHSTENCIL_OFF); // 2Dの描画になるのでデプスステンシルを無効
	DirectX::XMStoreFloat4x4(&mat2D[0], DirectX::XMMatrixTranspose(
		DirectX::XMMatrixScaling(128.f, 1.0f, -72.0f) *
		DirectX::XMMatrixRotationX(-DirectX::XM_PIDIV2) *
		DirectX::XMMatrixTranslation(640.f, 360.f, 0.0f)
	));
	m_pMatrix->Write(mat2D);
	// 横方向のブラーを適用
	GRAPHICS->SetRenderTarget(&m_ppDOVTexture[DOV_X_BLUR], 1, clearColor);
	{
		blurParam = DirectX::XMFLOAT4(640.f, 720.f, 1.0f, 0.0f);
		m_pBlurParam->Write(&blurParam);
		GetPixelShader(BLUR_PS)->Bind();
		SetTexturePS(m_ppDOVTexture[DOV_RENDER_TARGET]);
		pMain->DrawPlane();
	}
	// 縦方向のブラーを適用
	GRAPHICS->SetRenderTarget(&m_ppDOVTexture[DOV_Y_BLUR], 1, clearColor);
	{
		blurParam = DirectX::XMFLOAT4(640.f, 360.f, 0.0f, 1.0f);
		m_pBlurParam->Write(&blurParam);
		GetPixelShader(BLUR_PS)->Bind();
		SetTexturePS(m_ppDOVTexture[DOV_X_BLUR]);
		pMain->DrawPlane();
	}

	// 最終結果
	// 深度値をワールド座標に変換するための逆行列
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
	// パラメータ
	DirectX::XMFLOAT3 cameraPos = pCamera3D->GetPos();
	DirectX::XMFLOAT4 dov[2] = {

		// カメラ座標
		DirectX::XMFLOAT4(cameraPos.x,cameraPos.y,cameraPos.z,0.0f),
		// 被写界深度　調整用パラメータ
		DirectX::XMFLOAT4(
			2.0f,	// カメラのフォーカス距離
			6.0f,	// フォーカス位置からボケ画像に変化する距離
			0.0f,	// 未使用
			0.0f)
 };
	m_pDOVParam->Write(dov);
	m_pDOVParam->BindPS(0);

	GRAPHICS->SetRenderTargetDefault();
	{
		GetPixelShader(DEPTH_OF_VIEW_PS)->Bind();
		// 被写界深度では通常の絵とぼかした絵を混ぜるため、
		// それぞれのテクスチャをシェーダに渡す

		SetTexturePS(m_ppDOVTexture[DOV_RENDER_TARGET], 0);
		//SetTexturePS(m_ppDOVTexture[DOV_Y_BLUR], 1);
		SetTexturePS(m_ppDOVTexture[DOV_X_BLUR], 1);

		// カメラとの距離を調べるために深度地を使う
		SetTexturePS(m_ppDOVTexture[DOV_DEPTH], 2);
		

		pMain->DrawPlane();
	}
	GRAPHICS->SetDepthStencilState(Graphics::DEPTHSTENCIL_ON);
}