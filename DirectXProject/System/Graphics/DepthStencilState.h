#ifndef __DEPTH_STENCIL_STATE_H__
#define __DEPTH_STENCIL_STATE_H__

#include <System/Graphics/DirectX.h>

class DepthStencilState
{
public:
	DepthStencilState();
	~DepthStencilState();

	HRESULT Create(bool depthEnable, bool stencilEnable);
	void Bind();

private:
	ID3D11DepthStencilState *m_pState;
};

#endif // __DEPTH_STENCIL_STATE_H__