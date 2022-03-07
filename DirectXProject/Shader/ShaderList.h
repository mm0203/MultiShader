#ifndef __SHADER_LIST_H__
#define __SHADER_LIST_H__

#include <System/Graphics/Shader.h>

enum VertexShaderKind
{
	ALPHA_DITHER_VS,
	DEFERRED_WRITE_VS,
	DEPTH_SHADOW_VS,
	DEPTH_WRITE_VS,
	SAMPLE_VS,
	SCREEN_VS,
	WORLD_POS_VS,
	MAX_VERTEX_SHADER
};
enum PixelShaderKind
{
	ALPHA_DITHER_PS,
	BLUR_PS,
	DEFERRED_PS,
	DEFERRED_WRITE_PS,
	DEPTH_FADE_PS,
	DEPTH_OF_VIEW_PS,
	DEPTH_OF_VIEW_WRITE_PS,
	DEPTH_SHADOW_PS,
	DEPTH_WRITE_PS,
	OBJECT_COLOR_PS,
	POINT_LIGHT_PS,
	RENDER_SHADOW_PS,
	SAMPLE_PS,
	SPOT_LIGHT_PS,
	MAX_PIXEL_SHADER
};

HRESULT InitShaderResource();
void UninitShaderResource();
Shader* GetVertexShader(VertexShaderKind vs);
Shader* GetPixelShader(PixelShaderKind ps);

#endif // __SHADER_LIST_H__