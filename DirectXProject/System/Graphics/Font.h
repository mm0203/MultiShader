/**
 * @file Font.cpp
 * @brief winapiでフォント作成、sdfとしてテクスチャ書き込み
 * @author ryota.ssk
 */

#include <System/Texture/Texture.h>
#include <vector>

class Font
{
public:
	struct Word
	{
		BYTE byteLen; // バイト数
		WORD code;
	};
	struct WriteInfo
	{
		float* pImage;
		UINT texSize;
		UINT charaTexSize;
		BYTE* pBitmap;
		UINT bmpPitch;
		UINT bmpPixelSize;		
	};
public:
	struct UV
	{
		float u;
		float v;
		float width;
		float height;
	};
public:
	Font();
	~Font();

	HRESULT Create(UINT fontSize, const char* pStr);

	Texture* GetTexture();
	UV GetUV(const char* word);

private:
	Word GetWord(const char* c);
	void WriteFont(Word& word, WriteInfo &info, HDC hdc);
	void WriteUndefined(WriteInfo &info);

	HPEN MakePenHandle();
	HFONT MakeFontHandle(UINT fontSize);
	HBITMAP MakeWriteBitmapHandle(WriteInfo &info);
private:
	using Strings = std::vector<Word>;

	Strings		m_strings;			// 文字リスト
	UINT		m_strCountOfLine;	// 一行の文字数
	Texture*	m_pTexture;			// テクスチャ
};
