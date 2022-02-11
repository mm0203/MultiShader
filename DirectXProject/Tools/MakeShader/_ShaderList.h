#ifndef __SHADER_LIST_H__
#define __SHADER_LIST_H__

#include <System/Graphics/Shader.h>

enum VertexShaderKind
{
__VS_KIND__	MAX_VERTEX_SHADER
};
enum PixelShaderKind
{
__PS_KIND__	MAX_PIXEL_SHADER
};

HRESULT InitShaderResource();
void UninitShaderResource();
Shader* GetVertexShader(VertexShaderKind vs);
Shader* GetPixelShader(PixelShaderKind ps);

#endif // __SHADER_LIST_H__