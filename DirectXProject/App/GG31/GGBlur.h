#ifndef __GG_BLUR_H__
#define __GG_BLUR_H__

#include <System/Scene/SceneBase.h>
#include <System/Graphics/Shader.h>
#include <System/Texture/Texture.h>
#include <App/GG31/GGLight.h>

class GGBlur : public Scene
{
	SCENE_DECLARATION(GGBlur);
private:
	ConstantBuffer* m_pMatrix;
	ConstantBuffer* m_pColorBuf;
	ConstantBuffer* m_pLightBuf;
	ConstantBuffer* m_pInvVPS;
	ConstantBuffer* m_pBlurParam;

	Texture** m_ppRenderTarget;
	Texture** m_ppBlurTexture;

	static const int LIGHT_NUM = 30;
	Light* m_pLight[LIGHT_NUM];
};


#endif // __GG_BLUR_H__