#ifndef __GG_DEPTH_FADE_H__
#define __GG_DEPTH_FADE_H__

#include <System/Scene/SceneBase.h>
#include <System/Graphics/Shader.h>
#include <System/Texture/Texture.h>

class GGDepthFade : public Scene
{
	SCENE_DECLARATION(GGDepthFade);
private:
	float m_posY;
	float m_tick;
	Texture* m_pRenderTarget;
	Texture* m_pDepthStencil;
	ConstantBuffer* m_pMatrix;
};

#endif // __GG_DEPTH_FADE_H__