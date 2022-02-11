#include <Shader/ShaderList.h>
#include <System/Macro.h>

const char* VertexShaderFile[MAX_VERTEX_SHADER] =
{
__VS_FILE__};
const char* PixelShaderFile[MAX_PIXEL_SHADER] =
{
__PS_FILE__};

Shader* g_pVertexShaderList[MAX_VERTEX_SHADER];
Shader* g_pPixelShaderList[MAX_PIXEL_SHADER];

HRESULT InitShaderResource()
{
	HRESULT hr = S_OK;
	for (int i = 0; i < MAX_VERTEX_SHADER; ++i)
	{
		g_pVertexShaderList[i] = new VertexShader;
		hr = g_pVertexShaderList[i]->Load(VertexShaderFile[i]);
		if (FAILED(hr)) { return hr; }
	}
	for (int i = 0; i < MAX_PIXEL_SHADER; ++i)
	{
		g_pPixelShaderList[i] = new PixelShader;
		hr = g_pPixelShaderList[i]->Load(PixelShaderFile[i]);
		if (FAILED(hr)) { return hr; }
	}
	return hr;
}
void UninitShaderResource()
{
	for (int i = 0; i < MAX_PIXEL_SHADER; ++i)
	{
		SAFE_DELETE(g_pPixelShaderList[i]);
	}
	for (int i = 0; i < MAX_VERTEX_SHADER; ++i)
	{
		SAFE_DELETE(g_pVertexShaderList[i]);
	}
}
Shader* GetVertexShader(VertexShaderKind vs)
{
	return g_pVertexShaderList[vs];
}
Shader* GetPixelShader(PixelShaderKind ps)
{
	return g_pPixelShaderList[ps];
}