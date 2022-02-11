#ifndef __GG_DEPTH_SHADOW_H__
#define __GG_DEPTH_SHADOW_H__

#include <System/Scene/SceneBase.h>
#include <System/Graphics/Shader.h>
#include <System/Texture/Texture.h>
#include <DirectXMath.h>

class GGDepthShadow : public Scene
{
	SCENE_DECLARATION(GGDepthShadow);
private:
	float m_fTick[10];
	DirectX::XMFLOAT3 m_Pos[10];
	// float m_fPosY;

	Texture* m_pRenderTarget;
	Texture* m_pDepthStencil;
	ConstantBuffer* m_pMatrix;
	ConstantBuffer* m_pShadowVPS;
};

#endif // __GG_DEPTH_SHADOW_H__