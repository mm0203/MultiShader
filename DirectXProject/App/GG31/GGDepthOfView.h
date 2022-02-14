#ifndef __GG_DEPTH_OF_VIEW_H__
#define __GG_DEPTH_OF_VIEW_H__

#include <System/Scene/SceneBase.h>
#include <System/Graphics/Shader.h>
#include <System/Texture/Texture.h>
#include <DirectXMath.h>

class GGDepthOfView : public Scene
{
	SCENE_DECLARATION(GGDepthOfView);
private:
	ConstantBuffer* m_pMatrix;
	ConstantBuffer* m_pColorBuf;
	ConstantBuffer* m_pBlurParam;
	ConstantBuffer* m_pInvVPS;
	ConstantBuffer* m_pDOVParam;

	Texture** m_ppDOVTexture;

	static const int OBJECT_NUM = 30;
	DirectX::XMFLOAT4X4 m_world[OBJECT_NUM];
	DirectX::XMFLOAT4 m_color[OBJECT_NUM];
};


#endif // __GG_BLUR_H__