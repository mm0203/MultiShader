#ifndef __TEXT_H__
#define __TEXT_H__

#include <DirectXMath.h>
#include <System/Graphics/DXBuffer.h>
#include <System/Graphics/Font.h>
#include <System/Graphics/Shader.h>


class Text
{
private:
	struct Vertex
	{
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT2 uv;
	};
	struct Matrix
	{
		DirectX::XMFLOAT4X4 view;
		DirectX::XMFLOAT4X4 proj;
	};
	struct Parameter
	{
		DirectX::XMFLOAT4 brush;
		DirectX::XMFLOAT4 pen;
		DirectX::XMFLOAT4 value;
	};
public:
	Text();
	~Text();

	HRESULT Create(float fontSize);
	HRESULT Create(float fontSize, const char* pStrings);

	void Print(const char* str);
	template<typename... Args>
	void Print(const char* format, Args&& ...args);
	template<typename... Args>
	void Print(const DirectX::XMFLOAT3& pos, const char* format, Args&& ...args);

	void SetPos(const DirectX::XMFLOAT3 &pos);
	void SetBrushColor(const DirectX::XMFLOAT4& color);
	void SetPenColor(const DirectX::XMFLOAT4& color);
	void SetSDF(float sdf);
	void SetEdge(float width);
	void SetAlpha(float alpha);
	void SetCharaSpace(float space);

	void Draw();
private:
	template<typename T, typename... Args>
	void Parse(std::string& str, const char* format, T value, Args... args);
	void Parse(std::string& str, const char* format);

private:
	DXBuffer* m_pBuffer;
	VertexShader* m_pVS;
	PixelShader* m_pPS;
	ConstantBuffer* m_pVSBuf;
	ConstantBuffer* m_pPSBuf;
	Matrix m_vsMatrix;
	Parameter m_psParam;
	Font* m_pFont;

	Vertex* m_pVertex;
	UINT m_polyNum;
	float m_fontSize;
	float m_space;
	DirectX::XMFLOAT3 m_pos;

};

template<typename... Args>
void Text::Print(const char* format, Args&& ...args)
{
	std::string str;
	Parse(str, format, args...);
	Print(str.c_str());
}
template<typename... Args>
void Text::Print(const DirectX::XMFLOAT3& pos, const char* format, Args&& ...args)
{
	SetPos(pos);
	Print(format, args...);
}
template<typename T, typename... Args>
void Text::Parse(std::string& str, const char* format, T value, Args... args)
{
	while (format && *format)
	{
		if (*format == '%' && *(++format) != '%')
		{
			switch (*format)
			{
			default:
				str += value;
				break;
			case 'f':
				str += std::to_string(*reinterpret_cast<float*>(&value));
				break;
			case 'd':
				str += std::to_string(*reinterpret_cast<int*>(&value));
				break;
			}
			Parse(str, ++format, args...);
			break;
		}
		str += *format++;
	}
}

#endif // __RENDER_TEXT_H__