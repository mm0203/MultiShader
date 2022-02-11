#ifndef __SHADER_SCENE_H__
#define __SHADER_SCENE_H__

#include <System/Scene/SceneBase.h>
#include <System/Graphics/DXBuffer.h>
#include <System/Graphics/Shader.h>
#include <DirectXMath.h>
#include <System/Texture/Texture.h>
//#include <App/FBXModel.h>
#include <App/TangentModel.h>
//#include <Effekseer/Effekseer.h>
//#include <Effekseer/EffekseerRendererDX11.h>

//#pragma comment(lib, "Effekseer.lib")
//#pragma comment(lib, "EffekseerRendererDX11.lib")

class ShaderScene : public Scene
{
public:
	SCENE_DECLARATION(ShaderScene);
private:
	// 頂点シェーダに送る変換行列
	struct Matrix
	{
		DirectX::XMFLOAT4X4 world;
		DirectX::XMFLOAT4X4 view;
		DirectX::XMFLOAT4X4 proj;
	};
private:
	DXBuffer *m_pPrimitive;
	VertexShader *m_pSampleVS;
	PixelShader *m_pSamplePS;
	ConstantBuffer *m_pMatrixCB;
	Matrix m_vsMatrix;
	ConstantBuffer* m_pLightCB;
	ConstantBuffer* m_pCameraCB;
	ConstantBuffer* m_pParameterCB;
	DirectX::XMFLOAT4 m_psLight;
	DirectX::XMFLOAT4 m_psParameter;
	Texture* m_pTexture;
	Texture* m_pDisolveTex;

	//FBXModel* m_pModel;
	TangentModel *m_pTangentModel;


/*	EffekseerRendererDX11::RendererRef m_renderer;
	Effekseer::ManagerRef m_manager;
	Effekseer::EffectRef m_effect;
	Effekseer::Handle m_handle;
	*/
};

#endif // __SAMPLE_SCENE_H__