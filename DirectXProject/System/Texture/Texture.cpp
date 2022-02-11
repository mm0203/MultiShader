#include <system/Texture/Texture.h>

Texture::Texture(): m_width(0), m_height(0), m_pTex(nullptr), m_pSRV(nullptr)
{
}

Texture::~Texture()
{
	Release();
}

void Texture::Release()
{
	SAFE_RELEASE(m_pSRV);
	SAFE_RELEASE(m_pTex);
}

UINT Texture::GetWidth() const
{
	return m_width;
}

UINT Texture::GetHeight() const
{
	return m_height;
}

ID3D11ShaderResourceView* Texture::GetResource() const
{
	return m_pSRV;
}

void Texture::Clone(std::function<void(void* pData, UINT dataSize, UINT width, UINT height)> func)
{
	//https://docs.microsoft.com/en-us/windows/win32/api/d3d11/ne-d3d11-d3d11_usage
	ID3D11Device* pDevice = GetDevice();
	ID3D11DeviceContext* pContext = GetContext();
	HRESULT hr;

	// コピー先リソース作成のために、リソースの情報を取得
	D3D11_TEXTURE2D_DESC texDesc;
	m_pTex->GetDesc(&texDesc);

	// コピー先リソース作成
	texDesc.BindFlags = 0;
	texDesc.Usage = D3D11_USAGE_STAGING;
	texDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	ID3D11Texture2D* pCopyTex;
	hr = pDevice->CreateTexture2D(&texDesc, nullptr, &pCopyTex);
	if (FAILED(hr)) { return; }

	// リソースをコピー
	pContext->CopyResource(pCopyTex, m_pTex);

	// コピーしたリソースから読み取り
	D3D11_MAPPED_SUBRESOURCE mapped;
	hr = pContext->Map(pCopyTex, 0, D3D11_MAP_READ, 0, &mapped);
	if (FAILED(hr)) { return; }
	UINT rowPitch = m_width * GetPixelSize(texDesc.Format);
	BYTE* pData = new BYTE[rowPitch * m_height];
	for (UINT i = 0; i < m_height; ++i)
	{
		BYTE* pDst = pData + i * rowPitch;
		BYTE* pSrc = reinterpret_cast<BYTE*>(mapped.pData) + i * mapped.RowPitch;
		memcpy(pDst, pSrc, rowPitch);
	}
	pContext->Unmap(pCopyTex, 0);

	// 読み取ったデータに対して処理を実行
	func(pData, rowPitch * m_height, m_width, m_height);
	delete[] pData;
	SAFE_RELEASE(pCopyTex);
}

BYTE Texture::GetPixelSize(DXGI_FORMAT format)
{
	switch(format)
	{
	default:
		return 0;
	case DXGI_FORMAT_R32_SINT:
	case DXGI_FORMAT_R32_FLOAT:
	case DXGI_FORMAT_R32_UINT:
	case DXGI_FORMAT_R32_TYPELESS:
		return 4;
	}
}
HRESULT Texture::CreateResource(D3D11_TEXTURE2D_DESC& desc, const void* pData)
{
	// テクスチャ作成
	D3D11_SUBRESOURCE_DATA data = {};
	data.pSysMem = pData;
	data.SysMemPitch = desc.Width * 4;
	GetDevice()->CreateTexture2D(&desc, pData ? &data : nullptr, &m_pTex);

	// 設定
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = desc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	// 生成
	return GetDevice()->CreateShaderResourceView(m_pTex, &srvDesc, &m_pSRV);
}