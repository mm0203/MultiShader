#include <App/ShaderScene.h>
#include <System/Debug/Geometory.h>
#include <System/Camera/CameraManager.hpp>
#include <System/Input.h>
#include <System/Graphics/Text.h>
#include <System/Texture/TextureFactory.h>
#include <System/Macro.h>
#include <System/Debug/Menu.h>


HRESULT ShaderScene::Load()
{


	// DirectXで画面にオブジェクトが表示されるまで
	// ①オブジェクトの頂点情報(面の構成情報)
	// ②頂点レイアウト(InputLayout)
	//   GPUに頂点構造を教えるためのデータ
	// ③頂点シェーダ(VertexShader)
	//   CPUからGPUに頂点データを送り、3D空間上の点を
	//   2D画面上の表示位置に変換する
	// ④ピクセルシェーダ(PixelShader)
	//   頂点シェーダで変換した表示位置に、
	//   どのような色が表示されるか指定する

	// 円柱オブジェクト作成
	struct CylinderVertex
	{
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT3 normal;
		DirectX::XMFLOAT2 uv;
	};
	const static int CylinderVtxCount = 16;
	CylinderVertex cylinderVtx[(CylinderVtxCount + 1) * 2];
	for (int i = 0; i <= CylinderVtxCount; ++i)
	{
		float rad = i * DirectX::XM_PI * 2.f / CylinderVtxCount;
		float u = static_cast<float>(i * 3.0f) / CylinderVtxCount;
		DirectX::XMFLOAT3 normal(sinf(rad), 0.0f, -cosf(rad));
		int idx = i * 2;
		// 上段
		cylinderVtx[idx + 0].normal = normal;
		cylinderVtx[idx + 0].pos = normal;
		cylinderVtx[idx + 0].pos.y = 1.0f;
		cylinderVtx[idx + 0].uv = DirectX::XMFLOAT2(u, 0.0f);
		// 下段
		cylinderVtx[idx + 1].normal = normal;
		cylinderVtx[idx + 1].pos = normal;
		cylinderVtx[idx + 1].pos.y = -1.0f;
		cylinderVtx[idx + 1].uv = DirectX::XMFLOAT2(u, 1.0f);
	}
	CylinderVertex polyVtx[4] = {
		{ DirectX::XMFLOAT3(-1, 1,0), DirectX::XMFLOAT3(0,0,1), DirectX::XMFLOAT2(0, 0) },
		{ DirectX::XMFLOAT3( 1, 1,0), DirectX::XMFLOAT3(0,0,1), DirectX::XMFLOAT2(1, 0) },
		{ DirectX::XMFLOAT3(-1,-1,0), DirectX::XMFLOAT3(0,0,1), DirectX::XMFLOAT2(0, 1) },
		{ DirectX::XMFLOAT3( 1,-1,0), DirectX::XMFLOAT3(0,0,1), DirectX::XMFLOAT2(1, 1) },
	};
	// バッファ作成
	DXBuffer::Desc desc = {};
	desc.pVtx = cylinderVtx;
	desc.vtxSize = sizeof(CylinderVertex);
	desc.vtxCount = _countof(cylinderVtx);
	desc.topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;

	HRESULT hr;
	m_pPrimitive = new DXBuffer;
	hr = m_pPrimitive->Create(desc);
	if (FAILED(hr)) { return hr; }


	// 定数バッファ作成
	Camera* pCamera = CAMERA->Get();
	DirectX::XMStoreFloat4x4(&m_vsMatrix.world, DirectX::XMMatrixIdentity());
	DirectX::XMStoreFloat4x4(&m_vsMatrix.view, pCamera->GetLookAtMatrix());
	DirectX::XMStoreFloat4x4(&m_vsMatrix.proj, pCamera->GetProjectionMatrix());

	m_pMatrixCB = new ConstantBuffer;
	hr = m_pMatrixCB->Create(sizeof(Matrix));
	if (FAILED(hr)) { return hr; }
	m_pLightCB = new ConstantBuffer;
	hr = m_pLightCB->Create(sizeof(m_psLight));
	if (FAILED(hr)) { return hr; }
	m_pCameraCB = new ConstantBuffer;
	hr = m_pCameraCB->Create(sizeof(DirectX::XMFLOAT4));
	if (FAILED(hr)) { return hr; }
	m_pParameterCB = new ConstantBuffer;
	hr = m_pParameterCB->Create(sizeof(m_psParameter));


	// シェーダ読み込み
	m_pSampleVS = new VertexShader();
	hr = m_pSampleVS->Load("Assets/SampleVS.cso");
	if (FAILED(hr)) { return hr; }
	m_pSamplePS = new PixelShader();
	hr = m_pSamplePS->Load("Assets/SamplePS.cso");
	if (FAILED(hr)) { return hr; }


	// テクスチャの読み込み

	TangentModel::MakeDesc tanDesc;
	tanDesc.bufDesc = desc;
	tanDesc.posOffset = offsetof(CylinderVertex, pos);
	tanDesc.uvOffset = offsetof(CylinderVertex, uv);
	m_pTangentModel = new TangentModel;
	hr = m_pTangentModel->Make(tanDesc);
	if (FAILED(hr)) { return hr; }

	// Effekseer初期化
	/*m_renderer = EffekseerRendererDX11::Renderer::Create(GetDevice(), GetContext(), 8000);
	m_manager = Effekseer::Manager::Create(8000);
	// 描画モジュールの設定
	m_manager->SetSpriteRenderer(m_renderer->CreateSpriteRenderer());
	m_manager->SetRibbonRenderer(m_renderer->CreateRibbonRenderer());
	m_manager->SetRingRenderer(m_renderer->CreateRingRenderer());
	m_manager->SetTrackRenderer(m_renderer->CreateTrackRenderer());
	m_manager->SetModelRenderer(m_renderer->CreateModelRenderer());
	// テクスチャ、モデル、カーブ、マテリアルローダーの設定する。
	// ユーザーが独自で拡張できる。現在はファイルから読み込んでいる。
	m_manager->SetTextureLoader(m_renderer->CreateTextureLoader());
	m_manager->SetModelLoader(m_renderer->CreateModelLoader());
	m_manager->SetMaterialLoader(m_renderer->CreateMaterialLoader());
	m_manager->SetCurveLoader(Effekseer::MakeRefPtr<Effekseer::CurveLoader>());
	// 行列
	Effekseer::Vector3D eye = ::Effekseer::Vector3D(10.0f, 5.0f, 20.0f);
	// 投影行列を設定
	m_renderer->SetProjectionMatrix(::Effekseer::Matrix44().PerspectiveFovRH(
		90.0f / 180.0f * 3.14f, 16.0f / 9.0f, 1.0f, 500.0f));
	// カメラ行列を設定
	m_renderer->SetCameraMatrix(
		::Effekseer::Matrix44().LookAtRH(eye, ::Effekseer::Vector3D(0.0f, 0.0f, 0.0f), ::Effekseer::Vector3D(0.0f, 1.0f, 0.0f)));

	m_effect = Effekseer::Effect::Create(m_manager, u"Assets/Laser01.efk");*/

	return S_OK;
}
void ShaderScene::Release()
{
	SAFE_DELETE(m_pPrimitive);
	SAFE_DELETE(m_pSampleVS);
	SAFE_DELETE(m_pSamplePS);
	SAFE_DELETE(m_pMatrixCB);
	SAFE_DELETE(m_pLightCB);
	SAFE_DELETE(m_pCameraCB);
	SAFE_DELETE(m_pParameterCB);
	SAFE_DELETE(m_pTexture);
	//SAFE_DELETE(m_pModel);
	SAFE_DELETE(m_pTangentModel);
}
void ShaderScene::Update(float tick)
{
	// 定数バッファ更新(行列は転置をかける
	Camera* pCamera = CAMERA->Get();
	DirectX::XMStoreFloat4x4(&m_vsMatrix.view,
		DirectX::XMMatrixTranspose(pCamera->GetLookAtMatrix()));
	DirectX::XMStoreFloat4x4(&m_vsMatrix.proj,
		DirectX::XMMatrixTranspose(pCamera->GetProjectionMatrix()));
	m_pMatrixCB->Write(&m_vsMatrix);
	/*
	// 行列
	DirectX::XMFLOAT3 cameraPos = pCamera->GetPos();
	DirectX::XMFLOAT3 cameraLook = pCamera->GetLook();
	DirectX::XMFLOAT3 cameraUp = pCamera->GetUp();
	Effekseer::Vector3D eye = ::Effekseer::Vector3D(cameraPos.x, cameraPos.y, cameraPos.z);
	Effekseer::Vector3D look = ::Effekseer::Vector3D(cameraLook.x, cameraLook.y, cameraLook.z);
	Effekseer::Vector3D up = ::Effekseer::Vector3D(cameraUp.x, cameraUp.y, cameraUp.z);
	// 投影行列を設定
	m_renderer->SetProjectionMatrix(::Effekseer::Matrix44().PerspectiveFovLH(
		pCamera->GetFOV(), pCamera->GetAspect(), pCamera->GetNearClip(), pCamera->GetFarClip()));
	// カメラ行列を設定
	m_renderer->SetCameraMatrix(
		::Effekseer::Matrix44().LookAtLH(eye, look, up));
		*/
	// ライトの移動
	static float rad = 0.0f;

	m_psLight = DirectX::XMFLOAT4(sinf(rad), 0, cosf(rad), 0);
	rad += 0.01f;
	m_pLightCB->Write(&m_psLight);


	DirectX::XMFLOAT3 pos = pCamera->GetPos();
	DirectX::XMFLOAT4 pos4(pos.x, pos.y, pos.z, 0.0f);
	m_pCameraCB->Write(&pos4);

	// パラメータ
	static int frame = 0;
	m_psParameter.x = frame / 360.0f;
	m_pParameterCB->Write(&m_psParameter);
	frame %= 420;
	frame++;


	GEOMETORY->SetViewProj(pCamera->GetLookAt(), pCamera->GetProjection());

}
void ShaderScene::Draw()
{

	// シェーダの切り替え
	m_pSampleVS->Bind();
	m_pSamplePS->Bind();
	m_pMatrixCB->BindVS(0);
	m_pLightCB->BindVS(1);
	m_pLightCB->BindPS(0);
	m_pCameraCB->BindPS(1);
	m_pParameterCB->BindPS(2);
	//SetTexture(m_pTexture->GetResource());
	//SetTexture(m_pDisolveTex->GetResource(), 1);
	//m_pPrimitive->Draw();
	//DrawPolygon();
	//m_pModel->Draw();
	//m_pTangentModel->Draw();

	GEOMETORY->DrawPoint(DirectX::XMFLOAT3(0, 0, -1.5f));
	GEOMETORY->DrawArrow(DirectX::XMMatrixTranslation(0,0,-1));
	GEOMETORY->DrawMatrix(DirectX::XMMatrixRotationZ(DirectX::XM_PIDIV4));
	DirectX::XMFLOAT4 color(0.7f, 0.7f, 0.7f, 1.0f);
	const int GridNum = 5;
	for (int i = 1; i <= GridNum; ++i)
	{
		GEOMETORY->AddLine(DirectX::XMFLOAT3(i, 0.0f, GridNum), DirectX::XMFLOAT3(i, 0.0f, -GridNum), color);
		GEOMETORY->AddLine(DirectX::XMFLOAT3(-i, 0.0f, GridNum), DirectX::XMFLOAT3(-i, 0.0f, -GridNum), color);
		GEOMETORY->AddLine(DirectX::XMFLOAT3(GridNum, 0.0f, i), DirectX::XMFLOAT3(-GridNum, 0.0f, i), color);
		GEOMETORY->AddLine(DirectX::XMFLOAT3(GridNum, 0.0f, -i), DirectX::XMFLOAT3(-GridNum, 0.0f, -i), color);
	}
	GEOMETORY->AddLine(DirectX::XMFLOAT3(GridNum, 0.0f, 0.0f), DirectX::XMFLOAT3(), DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f));
	GEOMETORY->AddLine(DirectX::XMFLOAT3(0.0f, GridNum, 0.0f), DirectX::XMFLOAT3(), DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f));
	GEOMETORY->AddLine(DirectX::XMFLOAT3(0.0f, 0.0f, GridNum), DirectX::XMFLOAT3(), DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f));
	GEOMETORY->AddLine(DirectX::XMFLOAT3(-GridNum, 0.0f, 0.0f), DirectX::XMFLOAT3(), DirectX::XMFLOAT4(0.9f, 0.7f, 0.7f, 1.0f));
	GEOMETORY->AddLine(DirectX::XMFLOAT3(0.0f, -GridNum, 0.0f), DirectX::XMFLOAT3(), DirectX::XMFLOAT4(0.7f, 0.8f, 0.7f, 1.0f));
	GEOMETORY->AddLine(DirectX::XMFLOAT3(0.0f, 0.0f, -GridNum), DirectX::XMFLOAT3(), DirectX::XMFLOAT4(0.7f, 0.7f, 1.0f, 1.0f));
	GEOMETORY->DrawLines();



	// Effekseer
	/*
	static int time = 0;
	if (IsKeyTrigger('A'))
	{
		m_handle = m_manager->Play(m_effect, 0, 0, 0);
	}
	m_manager->Update();
	m_renderer->SetTime(time++ / 60.0f);
	m_renderer->BeginRendering();
	m_manager->Draw();
	m_renderer->EndRendering();*/
}
