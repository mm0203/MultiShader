#include <System/Texture/DepthStencil.h>

DepthStencil::DepthStencil()
	: m_pDSV(nullptr)
{
}

DepthStencil::~DepthStencil()
{
}

void DepthStencil::Release()
{
	Texture::Release();
	SAFE_RELEASE(m_pDSV);
}

ID3D11DepthStencilView* DepthStencil::GetView() const
{
	return m_pDSV;
}

HRESULT DepthStencil::CreateResource(D3D11_TEXTURE2D_DESC& desc, const void* pData)
{
	// ステンシル使用判定
	bool useStencil = (desc.Format == DXGI_FORMAT_D24_UNORM_S8_UINT);

	// リソース生成
	desc.Format = useStencil ? DXGI_FORMAT_R24_UNORM_X8_TYPELESS : DXGI_FORMAT_R32_TYPELESS;
	HRESULT hr = Texture::CreateResource(desc, nullptr);

	// 設定
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = useStencil ? DXGI_FORMAT_D24_UNORM_S8_UINT : DXGI_FORMAT_D32_FLOAT;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

	// 生成
	return GetDevice()->CreateDepthStencilView(m_pTex, &dsvDesc, &m_pDSV);
}