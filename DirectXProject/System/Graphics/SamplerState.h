#ifndef __SAMPLER_STATE_H__
#define __SAMPLER_STATE_H__

#include <System/Graphics/DirectX.h>

class SamplerState
{
public:
	SamplerState();
	~SamplerState();

	HRESULT Create(D3D11_FILTER filter, D3D11_TEXTURE_ADDRESS_MODE address);
	void Bind();

private:
	ID3D11SamplerState* m_pState;
};


#endif // __SAMPLER__STATE_H__