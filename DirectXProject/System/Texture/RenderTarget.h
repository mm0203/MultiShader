#ifndef __RENDER_TARGET_H__
#define __RENDER_TARGET_H__

#include <System/Texture/Texture.h>

/**
 * @brief レンダーターゲット
 */
class RenderTarget : public Texture
{
public:
	friend class TextureFactory;
public:
	RenderTarget();
	~RenderTarget();
	virtual void Release();

	ID3D11RenderTargetView* GetView() const;

protected:
	virtual HRESULT CreateResource(D3D11_TEXTURE2D_DESC& desc, const void* pData = nullptr);

private:
	ID3D11RenderTargetView* m_pRTV;
};

#endif // __RENDER_TAGET_H__