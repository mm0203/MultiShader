#ifndef __TEXTURE_FACTORY_H__
#define __TEXTURE_FACTORY_H__

#include <System/Texture/Texture.h>

/**
 * @brief テクスチャ生成
 */
class TextureFactory
{
public:
	static Texture* CreateFromFile(const char* fileName);
	static Texture* CreateFromData(DXGI_FORMAT format, UINT width, UINT height, const void* pData);
	static Texture* CreateRenderTarget(DXGI_FORMAT format, UINT width, UINT height);
	static Texture* CreateRenderTargetFromScreen();
	static Texture* CreateDepthStencil(UINT width, UINT height, bool useStencil = false);
};

#endif // __TEXTURE_FACTORY_H__