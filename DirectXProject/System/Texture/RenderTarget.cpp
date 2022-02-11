#include <System/Texture/RenderTarget.h>

RenderTarget::RenderTarget()
	: m_pRTV(nullptr)
{
}
RenderTarget::~RenderTarget()
{
}

void RenderTarget::Release()
{
	Texture::Release();
	SAFE_RELEASE(m_pRTV);
}

ID3D11RenderTargetView* RenderTarget::GetView() const
{
	return m_pRTV;
}

HRESULT RenderTarget::CreateResource(D3D11_TEXTURE2D_DESC& desc, const void* pData)
{
	// テクスチャリソース作成
	HRESULT hr = Texture::CreateResource(desc, nullptr);
	if (FAILED(hr)) { return hr; }

	// 設定
	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = desc.Format;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

	// 生成
	return GetDevice()->CreateRenderTargetView(m_pTex, &rtvDesc, &m_pRTV);
}