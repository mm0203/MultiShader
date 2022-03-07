#ifndef __GG_ALPHA_DITHER_H__
#define __GG_ALPHA_DITHER_H__

#include <System/Scene/SceneBase.h>
#include <System/Graphics/Shader.h>
#include <System/Texture/Texture.h>
#include <DirectXMath.h>

class GGAlphaDither : public Scene
{
	SCENE_DECLARATION(GGAlphaDither);
private:
	ConstantBuffer* m_pMatrix;
	ConstantBuffer* m_pColorBuf;
	ConstantBuffer* m_pCameraParam;


	static const int OBJECT_NUM = 30;
	DirectX::XMFLOAT4X4 m_world[OBJECT_NUM];
	DirectX::XMFLOAT4 m_color[OBJECT_NUM];
};

#endif // __GG_ALPHA_DITHER_H__