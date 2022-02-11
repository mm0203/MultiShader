#ifndef __GG_LEVEL_MAIN_H__
#define __GG_LEVEL_MAIN_H__

#include <System/Scene/SceneBase.h>
#include <System/Graphics/DXBuffer.h>
#include <System/Graphics/Shader.h>
#include <Shader/ShaderList.h>

class GGLevelMain : public Scene
{
	SCENE_DECLARATION(GGLevelMain);
public:
	void DrawPolygon();
	void DrawSphere();
	void DrawPlane();

private:
	DXBuffer* m_pPolygon;
	DXBuffer* m_pSphere;
	DXBuffer* m_pPlane;
	ConstantBuffer* m_pScreen;
};

#endif // __SAMPLE_SCENE_H__