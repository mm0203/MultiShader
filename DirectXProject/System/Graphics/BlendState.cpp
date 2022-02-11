#include <System/Graphics/BlendState.h>
#include <System/Graphics/Graphics.h>

BlendState::BlendState()
	: m_pState(nullptr)
{
}
BlendState::~BlendState()
{
	SAFE_RELEASE(m_pState);
}
HRESULT BlendState::Create(D3D11_RENDER_TARGET_BLEND_DESC desc)
{
	D3D11_BLEND_DESC blendDesc;
	ZeroMemory(&blendDesc, sizeof(blendDesc));
	blendDesc.RenderTarget[0] = desc;
	return GetDevice()->CreateBlendState(&blendDesc, &m_pState);
}
void BlendState::Bind()
{
	float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	GetContext()->OMSetBlendState(m_pState, blendFactor, 0xffffffff);
	GRAPHICS->SetBlendState(this);
}