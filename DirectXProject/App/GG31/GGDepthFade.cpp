#include <App/GG31/GGDepthFade.h>
#include <App/GG31/GGLevelMain.h>
#include <System/Scene/SceneManager.hpp>
#include <System/Macro.h>
#include <System/Texture/TextureFactory.h>
#include <DirectXMath.h>
#include <System/Camera/CameraManager.hpp>
#include <System/Graphics/Graphics.h>
#include <App/GG31/GGLightCamera.h>

HRESULT GGDepthFade::Load()
{
	// 定数バッファ
	m_pMatrix = new ConstantBuffer;
	m_pMatrix->Create(sizeof(DirectX::XMFLOAT4X4) * 3);

	// レンダーターゲット
	m_pRenderTarget = TextureFactory::CreateRenderTarget(DXGI_FORMAT_R32_FLOAT, 1024, 1024);
	m_pDepthStencil = TextureFactory::CreateDepthStencil(1024, 1024);

	m_posY = 0.0f;
	m_tick = 0.0f;
	return S_OK;
}
void GGDepthFade::Release()
{
}
void GGDepthFade::Update(float tick)
{
	m_tick += tick;
	m_posY = sinf(m_tick * 0.01f) * 2.f + 1.3f;
}
void GGDepthFade::Draw()
{
	GGLevelMain* pMain = SCENE->GetScene<GGLevelMain>(0);
	Camera* pCamera;
	DirectX::XMFLOAT4X4 mat[3];
	DirectX::XMStoreFloat4x4(&mat[0], DirectX::XMMatrixIdentity());

	// テクスチャへ書き込み
	float color[] = { 1,1,1,1 };
	GRAPHICS->SetRenderTarget(&m_pRenderTarget, 1, color);
	GRAPHICS->SetDepthStencilView(m_pDepthStencil, true);
	{
		// 既存のカメラの深度値を描きこむ
		// デプスフェードでは、光源のカメラなどは関係なく
		// ゲームのカメラから見たときにめり込むか判断したい
		pCamera = CAMERA->Get();
		DirectX::XMStoreFloat4x4(&mat[1], DirectX::XMMatrixTranspose(pCamera->GetLookAtMatrix()));
		DirectX::XMStoreFloat4x4(&mat[2], DirectX::XMMatrixTranspose(pCamera->GetProjectionMatrix()));
		m_pMatrix->Write(mat);

		// 深度値描きこみ
		GetVertexShader(DEPTH_WRITE_VS)->Bind();
		GetPixelShader(DEPTH_WRITE_PS)->Bind();
		m_pMatrix->BindVS(0);

		pMain->DrawPlane();
	}

	GRAPHICS->SetRenderTargetDefault();
	GRAPHICS->SetDepthStencilViewDefault();


	// 地面を表示
	// TODO あとで球を表示するとい透過処理の関係で正しく表示されないので先に描画
	GetPixelShader(SAMPLE_PS)->Bind();
	DirectX::XMStoreFloat4x4(&mat[0], DirectX::XMMatrixIdentity());
	m_pMatrix->Write(mat);
	pMain->DrawPlane();

	// デプスフェードを適用して描画
	GetVertexShader(DEPTH_WRITE_VS)->Bind();
	GetPixelShader(DEPTH_FADE_PS)->Bind();
	SetTexturePS(m_pRenderTarget, 0);
	DirectX::XMStoreFloat4x4(&mat[0], DirectX::XMMatrixTranspose(DirectX::XMMatrixTranslation(0, m_posY, 0)));
	m_pMatrix->Write(mat);
	pMain->DrawSphere();


	// デバッグ表示
	GetPixelShader(RENDER_SHADOW_PS)->Bind();
	SetTexturePS(m_pRenderTarget, 0);
	pMain->DrawPolygon();
}