#ifndef __GG_DEFERRED_H__
#define __GG_DEFERRED_H__

#include <System/Scene/SceneBase.h>
#include <System/Graphics/Shader.h>
#include <System/Texture/Texture.h>
#include <App/GG31/GGLight.h>

class GGDeferred : public Scene
{
	SCENE_DECLARATION(GGDeferred);
private:
	ConstantBuffer* m_pMatrix;
	ConstantBuffer* m_pColorBuf;
	ConstantBuffer* m_pLightBuf;
	ConstantBuffer* m_pInvVPS;

	Texture** m_ppRenderTarget;

	static const int LIGHT_NUM = 30;
	Light* m_pLight[LIGHT_NUM];
};

#endif // __GG_DEFERRED_H__