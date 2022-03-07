#include <App/GG31/GGAlphaDither.h>
#include <App/GG31/GGLevelMain.h>
#include <System/Scene/SceneManager.hpp>
#include <System/Macro.h>
#include <System/Camera/CameraManager.hpp>
#include <System/Camera/Camera2D.h>
#include <System/Texture/TextureFactory.h>
#include <System/Graphics/Graphics.h>

HRESULT GGAlphaDither::Load()
{
	// 定数バッファ
	m_pMatrix = new ConstantBuffer;
	m_pMatrix->Create(sizeof(DirectX::XMFLOAT4X4) * 3);
	m_pColorBuf = new ConstantBuffer;
	m_pColorBuf->Create(sizeof(DirectX::XMFLOAT4));
	m_pCameraParam = new ConstantBuffer;
	m_pCameraParam->Create(sizeof(DirectX::XMFLOAT4) * 2);

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


void GGAlphaDither::Release()
{
	SAFE_DELETE(m_pCameraParam);
	SAFE_DELETE(m_pColorBuf);
	SAFE_DELETE(m_pMatrix);
}
void GGAlphaDither::Update(float tick)
{
}
void GGAlphaDither::Draw()
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
	{
		GetVertexShader(ALPHA_DITHER_VS)->Bind();
		GetPixelShader(ALPHA_DITHER_PS)->Bind();
		m_pMatrix->BindVS(0);
		m_pColorBuf->BindPS(0);

		// パラメータ
		DirectX::XMFLOAT3 cameraPos = pCamera3D->GetPos();
		DirectX::XMFLOAT4 dither[2] = 
		{
			// ディザ用のパラメータ
			{cameraPos.x, cameraPos.y,cameraPos.z,0.0f},
			{
				0.5f,
				1.0f,
				0.0f,0.0f
			}

		};
		m_pCameraParam->Write(dither);
		m_pCameraParam->BindPS(1);


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
}