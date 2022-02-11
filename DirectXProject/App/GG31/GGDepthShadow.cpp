#include <App/GG31/GGDepthShadow.h>
#include <App/GG31/GGLevelMain.h>
#include <System/Scene/SceneManager.hpp>
#include <System/Macro.h>
#include <System/Texture/TextureFactory.h>
#include <DirectXMath.h>
#include <System/Camera/CameraManager.hpp>
#include <System/Graphics/Graphics.h>
#include <App/GG31/GGLightCamera.h>

HRESULT GGDepthShadow::Load()
{
	// 定数バッファ
	m_pMatrix = new ConstantBuffer;
	m_pMatrix->Create(sizeof(DirectX::XMFLOAT4X4) * 3);
	m_pShadowVPS = new ConstantBuffer;
	m_pShadowVPS->Create(sizeof(DirectX::XMFLOAT4X4));

	// レンダーターゲット
	m_pRenderTarget = TextureFactory::CreateRenderTarget(DXGI_FORMAT_R32_FLOAT, 1024, 1024);
	m_pDepthStencil = TextureFactory::CreateDepthStencil(1024, 1024);

	// ライト作成
	CAMERA->Create<GGLightCamera>();

	// 初期化
	for (int i = 0; i < 10; i++)
	{
		m_fTick[i] = rand() % 50 * 10.0f;
		m_Pos[i] = DirectX::XMFLOAT3((rand() % 7) - 3, 0.0f, (rand() % 7) - 3);
	}

	return S_OK;
}
void GGDepthShadow::Release()
{
	SAFE_DELETE(m_pDepthStencil);
	SAFE_DELETE(m_pRenderTarget);
	SAFE_DELETE(m_pShadowVPS);
	SAFE_DELETE(m_pMatrix);
}
void GGDepthShadow::Update(float tick)
{
	for (int i = 0; i < 10; i++)
	{
		m_fTick[i] += tick;
		m_Pos[i].y = sinf(m_fTick[i] * 0.01f) * 1.5f + 0.5f;
	}
}

void GGDepthShadow::Draw()
{
	GGLevelMain* pMain = SCENE->GetScene<GGLevelMain>(0);
	Camera* pCamera;
	DirectX::XMFLOAT4X4 mat[3];
	for (int i = 0; i < 10; i++)
		DirectX::XMStoreFloat4x4(&mat[0], DirectX::XMMatrixTranspose(DirectX::XMMatrixTranslation(0, m_Pos[i].y, 0)));

	// ---テクスチャへ書き込み---
	// 落ち影（モデルからできる影）の表現 1度別の視点から見た景色をテクスチャに書く

	// 色が距離の情報として扱われる
	// 全て0(黒色)->影がをさえぎる物体があると判断される
	float color[] = { 1,1,1,1 };
	GRAPHICS->SetRenderTarget(&m_pRenderTarget, 1, color);
	GRAPHICS->SetDepthStencilView(m_pDepthStencil, true);
	{
		// カメラから見た景色をテクスチャに書き込むために光源の視点を座標系に変換(行列設定)
		pCamera = CAMERA->Get<GGLightCamera>();
		DirectX::XMStoreFloat4x4(&mat[1], DirectX::XMMatrixTranspose(pCamera->GetLookAtMatrix()));
		DirectX::XMStoreFloat4x4(&mat[2], DirectX::XMMatrixTranspose(pCamera->GetProjectionMatrix()));

		m_pMatrix->Write(mat);

		// 事前の影の情報を計算しておく
		// テクスチャに書き込んだ情報は0～1を指定して取り出す
		// カメラ行列は-1～1の範囲
		// カメラから見た情報をテクスチャ取り出しはスクリーン座標->UV座標に変換
		DirectX::XMMATRIX screenMat = DirectX::XMMatrixSet(
			0.5f, 0.0f, 0.0f, 0.0f,
			0.0f, -0.5f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.5f, 0.5f, 0.0f, 1.0f
		);
		// 事前に計算した影の情報を[vps]に格納
		DirectX::XMFLOAT4X4 vps;
		// テクスチャ座標を正しく読み込む
		DirectX::XMStoreFloat4x4(&vps, DirectX::XMMatrixTranspose(pCamera->GetLookAtMatrix() * pCamera->GetProjectionMatrix() * screenMat));
		// 影のバッファを更新
		m_pShadowVPS->Write(&vps);

		// テクスチャ書き込み
		// 影を落としたいオブジェクトを描画

		// シェーダーのセット
		GetVertexShader(DEPTH_WRITE_VS)->Bind();
		GetPixelShader(DEPTH_WRITE_PS)->Bind();
		// 定数バッファレジスタの番号に書き込み
		m_pMatrix->BindVS(0);

		// 影
		for (int i = 0; i < 10; i++)
		{
			DirectX::XMStoreFloat4x4(&mat[0], DirectX::XMMatrixTranspose(DirectX::XMMatrixTranslation(m_Pos[i].x, m_Pos[i].y, m_Pos[i].z)));
			m_pMatrix->Write(mat);
			pMain->DrawSphere();
		}
	}
	// テクスチャ書き込みが終わったら元の描画に戻す
	GRAPHICS->SetRenderTargetDefault();
	GRAPHICS->SetDepthStencilViewDefault();

	// 通常のゲーム画面の表示に戻す(ゲームカメラ行列)
	pCamera = CAMERA->Get();
	DirectX::XMStoreFloat4x4(&mat[1], DirectX::XMMatrixTranspose(pCamera->GetLookAtMatrix()));
	DirectX::XMStoreFloat4x4(&mat[2], DirectX::XMMatrixTranspose(pCamera->GetProjectionMatrix()));
	m_pMatrix->Write(mat);

	// 各情報設定
	GetVertexShader(DEPTH_SHADOW_VS)->Bind();
	GetPixelShader(DEPTH_SHADOW_PS)->Bind();
	m_pShadowVPS->BindVS(1);
	SetTexturePS(m_pRenderTarget, 1);

	// オブジェクト
	for (int i = 0; i < 10; i++)
	{
		DirectX::XMStoreFloat4x4(&mat[0], DirectX::XMMatrixTranspose(DirectX::XMMatrixTranslation(m_Pos[i].x, m_Pos[i].y, m_Pos[i].z)));
		m_pMatrix->Write(mat);
		pMain->DrawSphere();
	}

	// 地面は固定
	DirectX::XMStoreFloat4x4(&mat[0], DirectX::XMMatrixIdentity());
	m_pMatrix->Write(&mat);
	pMain->DrawPlane();


	// レンダーターゲットに描き込んだ内容をデバッグ表示
	GetPixelShader(RENDER_SHADOW_PS)->Bind();
	SetTexturePS(m_pRenderTarget, 0);
	pMain->DrawPolygon();
}