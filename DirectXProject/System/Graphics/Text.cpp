#include <System/Graphics/Text.h>
#include <System/Macro.h>
#include <System/Graphics/Graphics.h>

const char GraphicsTextVS[] = R"EOT(
struct VS_IN
{
	float3 pos : POSITION0;
	float2 uv : TEXCOORD0;
};
struct VS_OUT
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD0;
};
cbuffer Matrix : register(b0)
{
	float4x4 view;
	float4x4 proj;
};
VS_OUT main(VS_IN vin)
{
	VS_OUT vout;
	vout.pos = float4(vin.pos, 1.0f);
	vout.pos = mul(vout.pos, view);
	vout.pos = mul(vout.pos, proj);
	vout.uv = vin.uv;
	return vout;
}
)EOT";
const char GraphicsTextPS[] = R"EOT(
struct PS_IN
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD0;
};
Texture2D tex : register(t0);
SamplerState samp : register(s0);
cbuffer Parameter : register(b0)
{
	float4 brush;
	float4 pen;
	float4 value;
};
float4 main(PS_IN pin) : SV_TARGET
{
	float4 color = float4(1.0f, 1.0f, 1.0f, 1.0f);
	float sdf = tex.Sample(samp, pin.uv).r;
	color = lerp(pen, brush, step(value.x, sdf));
	color.a = step(value.x - saturate(value.y), sdf);
	color.a *= value.z;
	return color;
}
)EOT";

Text::Text()
	: m_pBuffer(nullptr)
	, m_pVS(nullptr)
	, m_pPS(nullptr)
	, m_pVSBuf(nullptr)
	, m_pPSBuf(nullptr)
	, m_pFont(nullptr)
	, m_pVertex(nullptr)
	, m_polyNum(0)
	, m_space(0.0f)
{
}
Text::~Text()
{
	SAFE_DELETE_ARRAY(m_pVertex);
	SAFE_DELETE(m_pFont);
	SAFE_DELETE(m_pPSBuf);
	SAFE_DELETE(m_pVSBuf);
	SAFE_DELETE(m_pPS);
	SAFE_DELETE(m_pVS);
	SAFE_DELETE(m_pBuffer);
}
HRESULT Text::Create(float fontSize)
{
	return Create(fontSize, 
		"0123456789"
		"abcdefghijklmnopqrstuvwxyz"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"+-*/|:()[]{}<>,.!?");
}
HRESULT Text::Create(float fontSize, const char* pStrings)
{
	HRESULT hr;

	// テキスト表示用ポリゴンデータ
	int textCount = 100;
	UINT vtxCount = textCount * 4;
	UINT idxCount = textCount * 6;
	m_pVertex = new Vertex[vtxCount];
	WORD *pIdx = new WORD[idxCount];
	for(UINT i = 0; i < textCount; ++ i)
	{
		UINT idx = i * 6;
		UINT vtxIdx = i * 4;
		m_pVertex[vtxIdx + 0] = { DirectX::XMFLOAT3(), DirectX::XMFLOAT2() };
		m_pVertex[vtxIdx + 1] = { DirectX::XMFLOAT3(), DirectX::XMFLOAT2() };
		m_pVertex[vtxIdx + 2] = { DirectX::XMFLOAT3(), DirectX::XMFLOAT2() };
		m_pVertex[vtxIdx + 3] = { DirectX::XMFLOAT3(), DirectX::XMFLOAT2() };
		pIdx[idx + 0] = vtxIdx + 0;
		pIdx[idx + 1] = vtxIdx + 1;
		pIdx[idx + 2] = vtxIdx + 2;
		pIdx[idx + 3] = vtxIdx + 1;
		pIdx[idx + 4] = vtxIdx + 3;
		pIdx[idx + 5] = vtxIdx + 2;
	}

	// バッファ作成
	DXBuffer::Desc desc =
	{
		m_pVertex, sizeof(Vertex), vtxCount, true,
		pIdx, sizeof(WORD), idxCount, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST
	};
	m_pBuffer = new DXBuffer;
	hr = m_pBuffer->Create(desc);
	SAFE_DELETE_ARRAY(pIdx);
	if (FAILED(hr)) { return hr; }

	// シェーダ
	m_pVS = new VertexShader;
	hr = m_pVS->Compile(GraphicsTextVS);
	if (FAILED(hr)) { return hr; }
	m_pPS = new PixelShader;
	hr = m_pPS->Compile(GraphicsTextPS);
	if (FAILED(hr)) { return hr; }

	// 定数バッファ
	m_pVSBuf = new ConstantBuffer;
	hr = m_pVSBuf->Create(sizeof(m_vsMatrix));
	if (FAILED(hr)) { return hr; }
	m_pPSBuf = new ConstantBuffer;
	hr = m_pPSBuf->Create(sizeof(m_vsMatrix));
	if (FAILED(hr)) { return hr; }
	DirectX::XMStoreFloat4x4(&m_vsMatrix.view, DirectX::XMMatrixIdentity());
	DirectX::XMStoreFloat4x4(&m_vsMatrix.proj,
		DirectX::XMMatrixTranspose(DirectX::XMMatrixOrthographicOffCenterLH(
			0.0f, 1280.0f, 720.0f, 0.0f, 0.1f, 10.0f
		)));
	m_pVSBuf->Write(&m_vsMatrix);
	m_psParam.brush = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_psParam.pen = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	m_psParam.value = DirectX::XMFLOAT4(0.95f, 0.05f, 1.0f, 0.0f);
	m_pPSBuf->Write(&m_psParam);

	// フォント
	m_pFont = new Font;
	hr = m_pFont->Create(24, pStrings);
	if (FAILED(hr)) { return hr; }

	m_fontSize = fontSize;

	return hr;
}
void Text::SetPos(const DirectX::XMFLOAT3& pos)
{
	m_pos = pos;
}
void Text::SetBrushColor(const DirectX::XMFLOAT4& color)
{
	m_psParam.brush = color;
	m_pPSBuf->Write(&m_psParam);
}
void Text::SetPenColor(const DirectX::XMFLOAT4& color)
{
	m_psParam.pen = color;
	m_pPSBuf->Write(&m_psParam);
}
void Text::SetSDF(float sdf)
{
	m_psParam.value.x = sdf;
	m_pPSBuf->Write(&m_psParam);
}
void Text::SetEdge(float width)
{
	m_psParam.value.y = width;
	m_pPSBuf->Write(&m_psParam);
}
void Text::SetAlpha(float alpha)
{
	m_psParam.value.z = alpha;
	m_pPSBuf->Write(&m_psParam);
}
void Text::SetCharaSpace(float space)
{
	m_space = space;
}
void Text::Draw()
{
	// 描画設定変更
	SamplerState* pOldSampler = GRAPHICS->GetSamplerState();
	DepthStencilState* pOldDepthStencil = GRAPHICS->GetDepthStencilState();
	GRAPHICS->SetSamplerState(Graphics::SAMPLER_LINEAR);
	GRAPHICS->SetDepthStencilState(Graphics::DEPTHSTENCIL_OFF);

	// 頂点バッファ書き換え
	m_pBuffer->Write(m_pVertex);
	
	// テキスト用シェーダ設定
	m_pVS->Bind();
	m_pPS->Bind();
	m_pVSBuf->BindVS(0);
	m_pPSBuf->BindPS(0);
	SetTexturePS(m_pFont->GetTexture());

	// 描画
	m_pBuffer->Draw(m_polyNum * 6);

	// 描画前の状態に戻す
	pOldDepthStencil->Bind();
	pOldSampler->Bind();
	m_polyNum = 0;
}


void Text::Parse(std::string& str, const char* format)
{
	str += format;
}
void Text::Print(const char* str)
{
	DirectX::XMFLOAT3 pos = m_pos;
	while (*str != '\0')
	{
		BYTE byte = IsDBCSLeadByte(str[0]) ? 2 : 1;
		do {
			if (byte == 1)
			{
				switch (*str)
				{
				case ' ':
					pos.x += m_fontSize + m_space;
					continue;
				case '\t':
					pos.x += (m_fontSize + m_space) * 2.0f;
					continue;
				case '\n':
					pos.x = m_pos.x;
					pos.y += m_fontSize;
					continue;
				}
			}
			else if (byte == 2)
			{
				if (str[0] == 129 && str[1] == 64) // 全角スペース
				{
					pos.x += m_fontSize + m_space;
					continue;
				}
			}

			UINT vtxIdx = m_polyNum * 4;
			Font::UV uv = m_pFont->GetUV(str);
			m_pVertex[vtxIdx + 0].pos = DirectX::XMFLOAT3(pos.x, pos.y, pos.z);
			m_pVertex[vtxIdx + 1].pos = DirectX::XMFLOAT3(pos.x + m_fontSize, pos.y, pos.z);
			m_pVertex[vtxIdx + 2].pos = DirectX::XMFLOAT3(pos.x, pos.y + m_fontSize, pos.z);
			m_pVertex[vtxIdx + 3].pos = DirectX::XMFLOAT3(pos.x + m_fontSize, pos.y + m_fontSize, pos.z);
			m_pVertex[vtxIdx + 0].uv = DirectX::XMFLOAT2(uv.u, uv.v);
			m_pVertex[vtxIdx + 1].uv = DirectX::XMFLOAT2(uv.u + uv.width, uv.v);
			m_pVertex[vtxIdx + 2].uv = DirectX::XMFLOAT2(uv.u, uv.v + uv.height);
			m_pVertex[vtxIdx + 3].uv = DirectX::XMFLOAT2(uv.u + uv.width, uv.v + uv.height);

			pos.x += m_fontSize + m_space;
			++m_polyNum;
		} while (0);
		str += byte;
	}
	m_pos.y += m_fontSize;
}