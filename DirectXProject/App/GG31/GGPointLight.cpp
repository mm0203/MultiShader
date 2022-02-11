#include <App/GG31/GGPointLight.h>
#include <App/GG31/GGLevelMain.h>
#include <System/Scene/SceneManager.hpp>
#include <System/Macro.h>
#include <System/Camera/CameraManager.hpp>

struct PointLightInfo
{
	DirectX::XMFLOAT3 pos;	 // 座標
	float range;			 // 光の届く範囲
	DirectX::XMFLOAT4 color; // 色
};

HRESULT GGPointLight::Load()
{
	// 定数バッファ
	m_pMatrix = new ConstantBuffer;
	m_pMatrix->Create(sizeof(DirectX::XMFLOAT4X4) * 3);
	m_pColorBuf = new ConstantBuffer;
	m_pColorBuf->Create(sizeof(DirectX::XMFLOAT4));
	m_pLightBuf = new ConstantBuffer;
	m_pLightBuf->Create(sizeof(PointLightInfo) * LIGHT_NUM);

	// ライト
	for (int i = 0; i < LIGHT_NUM; ++i)
	{
		m_pLight[i] = new Light(Light::POINT_LIGHT);

		// ライトの初期値
		m_pLight[i]->SetPos(DirectX::XMFLOAT3(
			(rand() % 21 - 10) * 0.15f,
			(rand() % 11) * 0.05f,
			(rand() % 21 - 10) * 0.15f));

		m_pLight[i]->SetAngle((rand() % 6 ) * 0.3f + 0.5f);

		m_pLight[i]->SetColor(DirectX::XMFLOAT4(
			(rand() % 11) * 0.1f, 
			(rand() % 11) * 0.1f, 
			(rand() % 11) * 0.1f, 
			 1.0f));
		
	}

	return S_OK;
}
void GGPointLight::Release()
{
	for (int i = 0; i < LIGHT_NUM; ++i)
	{
		SAFE_DELETE(m_pLight[i]);
	}
	SAFE_DELETE(m_pLightBuf);
	SAFE_DELETE(m_pColorBuf);
	SAFE_DELETE(m_pMatrix);
}
void GGPointLight::Update(float tick)
{
}
void GGPointLight::Draw()
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

	// ライト描画
	GetVertexShader(SAMPLE_VS)->Bind();
	GetPixelShader(OBJECT_COLOR_PS)->Bind();
	m_pColorBuf->BindPS(0);
	PointLightInfo lights[LIGHT_NUM];
	for (int i = 0; i < LIGHT_NUM; ++i)
	{
		// 球を描画
		DirectX::XMFLOAT3 pos = m_pLight[i]->GetPos();
		DirectX::XMFLOAT4 color = m_pLight[i]->GetColor();
		DirectX::XMStoreFloat4x4(&mat[0], DirectX::XMMatrixTranspose(
			DirectX::XMMatrixScaling(0.1f, 0.1f, 0.1f) *
			DirectX::XMMatrixTranslation(pos.x, pos.y, pos.z)));
		m_pMatrix->Write(mat);
		m_pColorBuf->Write(&color);
		pMain->DrawSphere();
		// 点光源の値を保存
		lights[i].pos = pos;
		lights[i].color = color;
		lights[i].range = m_pLight[i]->GetRange(); 
		
	}

	// 点光源を考慮して地面を描画
	DirectX::XMStoreFloat4x4(&mat[0], DirectX::XMMatrixIdentity());
	m_pMatrix->Write(mat);
	m_pLightBuf->Write(lights);
	m_pLightBuf->BindPS(0);
	GetVertexShader(WORLD_POS_VS)->Bind();
	GetPixelShader(POINT_LIGHT_PS)->Bind();
	pMain->DrawPlane();
}