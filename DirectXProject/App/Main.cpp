#include <App/Main.h>
#include <Defines.h>
#include <System/Graphics/DirectX.h>
#include <System/Graphics/Graphics.h>
#include <System/Graphics/Shader.h>
#include <System/Input.h>
#include <System/Debug/Debug.h>
#include <System/Camera/CameraManager.hpp>
#include <System/Scene/SceneManager.hpp>
#include <App/GG31/GGLevelMain.h>
#include <Shader/ShaderList.h>

#define ASSERT_FAILED(fn) do {\
hr = fn; \
if(FAILED(hr)) { \
	MessageBox(nullptr, "Init failed : "#fn, "Error", MB_OK); \
	return hr; \
}} while(0)

HRESULT Init(HWND hWnd, UINT width, UINT height)
{
	HRESULT hr;
	ASSERT_FAILED(InitDX(hWnd, width, height, false));
	ASSERT_FAILED(InitInput());
	//ggfbx::Initialize();
	Graphics::CreateInstance();
	InitDebug();
	InitShaderResource();
	CameraManager::CreateInstance();
	SceneManager::CreateInstance();
	ASSERT_FAILED(SCENE->Load<GGLevelMain>());


	return hr;
}

void Uninit()
{
	SceneManager::DestroyInstance();
	CameraManager::DestroyInstance();
	UninitShaderResource();
	UninitDebug();
	VertexShader::ReleaseInputLayout();
	Graphics::DestroyInstance();
	//ggfbx::Terminate();
	UninitInput();
	UninitDX();
}

void Update()
{
	UpdateDebug();
	UpdateInput();
	CAMERA->Update();
	SCENE->Update(1.0f);
}

void Draw()
{
	GRAPHICS->BeginDraw();
	SCENE->Draw();
	CAMERA->Draw();
	DrawDebug();
	GRAPHICS->EndDraw();
}

// EOF