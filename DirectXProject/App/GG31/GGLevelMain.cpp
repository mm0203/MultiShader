#include <App/GG31/GGLevelMain.h>
#include <System/Scene/SceneManager.hpp>
#include <System/Debug/Menu.h>
#include <System/Debug/Geometory.h>
#include <System/Camera/CameraManager.hpp>
#include <System/Camera/Camera2D.h>
#include <System/Macro.h>
#include <System/Graphics/Graphics.h>
#include <App/GG31/GGDepthShadow.h>
#include <App/GG31/GGDepthFade.h>
#include <App/GG31/GGPointLight.h>
#include <App/GG31/GGSpotLight.h>
#include <App/GG31/GGDeferred.h>
#include <App/GG31/GGBlur.h>
#include <App/GG31/GGDepthOfView.h>
#include <App/GG31/GGAlphaDither.h>

HRESULT GGLevelMain::Load()
{
	// 設定ファイル読込
	int sceneNo = 0;
	FILE* fp = fopen("Assets/GGLevelSetting.dat", "rb");
	if (fp)
	{
		fread(&sceneNo, sizeof(sceneNo), 1, fp);
		fclose(fp);
	}

	// メニュー構築
	Menu::Table table;
	table.headers.push_back("DepthShadow");
	table.headers.push_back("DepthFade");
	table.headers.push_back("PointLight");
	table.headers.push_back("SpotLight");
	table.headers.push_back("Deferred");
	table.headers.push_back("Blur");
	table.headers.push_back("DepthOfView");
	table.headers.push_back("AlphaDither");
	table.no = sceneNo;
	auto changeFunc = []()
	{
		SCENE->Unload(1);
		switch (MENU->GetTable("Scene/Kind"))
		{
		case 0: SCENE->LoadSub<GGDepthShadow>(); break;
		case 1: SCENE->LoadSub<GGDepthFade>(); break;
		case 2: SCENE->LoadSub<GGPointLight>(); break;
		case 3: SCENE->LoadSub<GGSpotLight>(); break;
		case 4: SCENE->LoadSub<GGDeferred>(); break;
		case 5: SCENE->LoadSub<GGBlur>(); break;
		case 6: SCENE->LoadSub<GGDepthOfView>(); break;
		case 7: SCENE->LoadSub<GGAlphaDither>(); break;
		}
	};
	MENU->Registry("Scene/Kind", table);
	MENU->Registry("Scene/Change", changeFunc);
	MENU->Registry("Scene/DrwaPolygon", false);
	changeFunc();

	// 定数バッファ作成
	m_pScreen = new ConstantBuffer;
	m_pScreen->Create(sizeof(DirectX::XMFLOAT4X4));

	// ジオメトリ構築
	struct Vertex
	{
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT2 uv;
		DirectX::XMFLOAT3 normal;
	};
	DXBuffer::Desc desc;
	desc.vtxSize = sizeof(Vertex);
	desc.idxSize = sizeof(WORD);
	desc.isWrite = false;

	// 球
	const int SphereSplitH = 32;
	const int SphereSplitV = SphereSplitH / 2 + 1;
	const int SphereVtxCount = (SphereSplitH + 1) * SphereSplitV;
	const int SphereIdxCount = SphereSplitH * (SphereSplitV - 1) * 6;
	Vertex* pSphereVtx = new Vertex[SphereVtxCount];
	WORD* pSphereIdx = new WORD[SphereIdxCount];
	for (int v = 0; v < SphereSplitV; ++v)
	{
		float radV = DirectX::XM_PI / (SphereSplitV - 1) * v;
		for (int h = 0; h <= SphereSplitH; ++h)
		{
			int idx = v * (SphereSplitH + 1) + h;
			float radH = DirectX::XM_2PI / SphereSplitH * h;
			pSphereVtx[idx].pos.x = sinf(radV) * -sinf(radH) * 0.5f;
			pSphereVtx[idx].pos.y = cosf(radV) * 0.5f;
			pSphereVtx[idx].pos.z = sinf(radV) * -cosf(radH) * 0.5f;
			pSphereVtx[idx].uv.x = static_cast<float>(h) / SphereSplitH;
			pSphereVtx[idx].uv.y = static_cast<float>(v) / (SphereSplitV - 1);
			pSphereVtx[idx].normal.x = pSphereVtx[idx].pos.x * 2.0f;
			pSphereVtx[idx].normal.y = pSphereVtx[idx].pos.y * 2.0f;
			pSphereVtx[idx].normal.z = pSphereVtx[idx].pos.z * 2.0f;
		}
	}
	// 球インデックス
	for (int v = 0, idx = 0; v < SphereSplitV - 1; ++v)
	{
		for (int h = 0; h < SphereSplitH; ++h)
		{
			pSphereIdx[idx + 0] = (SphereSplitH + 1) * v + h;
			pSphereIdx[idx + 1] = pSphereIdx[idx + 0] + 1;
			pSphereIdx[idx + 2] = pSphereIdx[idx + 0] + SphereSplitH + 1;
			pSphereIdx[idx + 3] = pSphereIdx[idx + 1];
			pSphereIdx[idx + 4] = pSphereIdx[idx + 2] + 1;
			pSphereIdx[idx + 5] = pSphereIdx[idx + 2];
			idx += 6;
		}
	}
	// 板
	Vertex planeVtx[] =
	{
		{ DirectX::XMFLOAT3(-5.0f, 0.0f, 5.0f), DirectX::XMFLOAT2(0,0), DirectX::XMFLOAT3(0,1,0)},
		{ DirectX::XMFLOAT3( 5.0f, 0.0f, 5.0f), DirectX::XMFLOAT2(1,0), DirectX::XMFLOAT3(0,1,0)},
		{ DirectX::XMFLOAT3(-5.0f, 0.0f,-5.0f), DirectX::XMFLOAT2(0,1), DirectX::XMFLOAT3(0,1,0)},
		{ DirectX::XMFLOAT3( 5.0f, 0.0f,-5.0f), DirectX::XMFLOAT2(1,1), DirectX::XMFLOAT3(0,1,0)},
	};
	Vertex polygonVtx[] =
	{
		{ DirectX::XMFLOAT3(-0.5f,-0.5f, 0.0f), DirectX::XMFLOAT2(0,0), DirectX::XMFLOAT3(0,0,-1)},
		{ DirectX::XMFLOAT3( 0.5f,-0.5f, 0.0f), DirectX::XMFLOAT2(1,0), DirectX::XMFLOAT3(0,0,-1)},
		{ DirectX::XMFLOAT3(-0.5f, 0.5f, 0.0f), DirectX::XMFLOAT2(0,1), DirectX::XMFLOAT3(0,0,-1)},
		{ DirectX::XMFLOAT3( 0.5f, 0.5f, 0.0f), DirectX::XMFLOAT2(1,1), DirectX::XMFLOAT3(0,0,-1)},
	};

	// 作成
	desc.pVtx = pSphereVtx;
	desc.vtxCount = SphereVtxCount;
	desc.pIdx = pSphereIdx;
	desc.idxCount = SphereIdxCount;
	desc.topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	m_pSphere = new DXBuffer;
	m_pSphere->Create(desc);

	desc.pVtx = planeVtx;
	desc.vtxCount = _countof(planeVtx);
	desc.pIdx = nullptr;
	desc.topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
	m_pPlane = new DXBuffer;
	m_pPlane->Create(desc);

	desc.pVtx = polygonVtx;
	desc.vtxCount = _countof(polygonVtx);
	m_pPolygon = new DXBuffer;
	m_pPolygon->Create(desc);
	
	SAFE_DELETE_ARRAY(pSphereVtx);
	SAFE_DELETE_ARRAY(pSphereIdx);
	return S_OK;
}
void GGLevelMain::Release()
{
	SAFE_DELETE(m_pPlane);
	SAFE_DELETE(m_pSphere);
	SAFE_DELETE(m_pPolygon);
	SAFE_DELETE(m_pScreen);

	// 設定ファイル保存
	int sceneNo = MENU->GetTable("Scene/Kind");
	FILE* fp = fopen("Assets/GGLevelSetting.dat", "wb");
	if (fp)
	{
		fwrite(&sceneNo, sizeof(sceneNo), 1, fp);
		fclose(fp);
	}
}
void GGLevelMain::Update(float tick)
{
	Camera* pCamera = CAMERA->Get();
	GEOMETORY->SetViewProj(pCamera->GetLookAt(), pCamera->GetProjection());
}
void GGLevelMain::Draw()
{
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
}
void GGLevelMain::DrawPolygon()
{
	if (!MENU->IsFlag("Scene/DrwaPolygon"))
	{
		return;
	}

	DepthStencilState* pOldDS = GRAPHICS->GetDepthStencilState();
	GRAPHICS->SetDepthStencilState(Graphics::DEPTHSTENCIL_OFF);
	SetCulling(CULL_NONE);
	Camera* pCamera = CAMERA->Get<Camera2D>();
	DirectX::XMMATRIX world = DirectX::XMMatrixScaling(200.0f, 200.0f, 1.0f) * DirectX::XMMatrixTranslation(1100.0f, 120.0f, 0.0f);
	DirectX::XMMATRIX proj = pCamera->GetProjectionMatrix();
	DirectX::XMFLOAT4X4 screen;
	DirectX::XMStoreFloat4x4(&screen, DirectX::XMMatrixTranspose(world * proj));
	m_pScreen->Write(&screen);

	GetVertexShader(SCREEN_VS)->Bind();
	m_pScreen->BindVS(0);
	m_pPolygon->Draw();

	pOldDS->Bind();
}
void GGLevelMain::DrawSphere()
{
	m_pSphere->Draw();
}
void GGLevelMain::DrawPlane()
{
	m_pPlane->Draw();
}
