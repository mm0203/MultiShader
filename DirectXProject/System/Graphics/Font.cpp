/**
 * @file Font.cpp
 * @brief winapiでフォント作成、sdfとしてテクスチャ書き込み
 * @author ryota.ssk
 */
#include <System/Graphics/Font.h>
#include <System/Texture/TextureFactory.h>
#include <System/Texture/TextureFilterSDF.h>
#include <System/Macro.h>

const UINT SDF_FONT_OUTER_SIZE = 5;
const UINT SDF_FONT_INNER_SIZE = 1;

Font::Font()
	: m_strCountOfLine(0)
	, m_pTexture(nullptr)
{
}
Font::~Font()
{
	SAFE_DELETE(m_pTexture);
}
HRESULT Font::Create(UINT fontSize, const char* pStr)
{
	// 初期化
	m_strings.clear();
	SAFE_DELETE(m_pTexture);

	// 文字列を文字へ分解
	while (*pStr != '\0')
	{
		Word word = GetWord(pStr);
		m_strings.push_back(word);
		pStr += word.byteLen;
	}

	// テクスチャ、文字サイズ計算
	m_strCountOfLine = static_cast<UINT>(ceilf(sqrtf(m_strings.size() + 1)));
	WriteInfo writeInfo;
	writeInfo.charaTexSize = fontSize + SDF_FONT_OUTER_SIZE * 2;
	writeInfo.texSize = m_strCountOfLine * writeInfo.charaTexSize;
	writeInfo.pImage = new float[writeInfo.texSize * writeInfo.texSize];
	float* pImageRoot = writeInfo.pImage;

	// 書き込み設定
	HDC		hDC		= GetDC(NULL);
	HDC		memDC	= CreateCompatibleDC(hDC);
	HPEN	hPen	= MakePenHandle();
	HFONT	hFont	= MakeFontHandle(fontSize);
	HBITMAP	hBitMap	= MakeWriteBitmapHandle(writeInfo);
	HPEN	oldPen	= (HPEN)SelectObject(memDC, hPen);
	HFONT	oldFont	= (HFONT)SelectObject(memDC, hFont);
	HBITMAP	oldBMP	= (HBITMAP)SelectObject(memDC, hBitMap);
	SetBkMode(memDC, TRANSPARENT);

	// 書き込み
	UINT imageX = 0;
	UINT imageY = 0;
	Strings::iterator it = m_strings.begin();
	while (it != m_strings.end())
	{
		writeInfo.pImage = pImageRoot + imageY * writeInfo.texSize + imageX;
		WriteFont(*it, writeInfo, memDC);

		// 次の書き込み先へ移動
		imageX += writeInfo.charaTexSize;
		if (imageX >= writeInfo.texSize)
		{
			imageX = 0;
			imageY += writeInfo.charaTexSize;
		}
		++it;
	}

	// 不一致データ作成
	writeInfo.pImage = pImageRoot + imageY * writeInfo.texSize + imageX;
	WriteUndefined(writeInfo);

	// テクスチャ作成
	m_pTexture = TextureFactory::CreateFromData(
		DXGI_FORMAT_R32_FLOAT, writeInfo.texSize, writeInfo.texSize, pImageRoot);
	m_pTexture->Filter(TextureFilterSDF(SDF_FONT_OUTER_SIZE, 0));

	// 作成したオブジェクト削除
	SelectObject(hDC, oldBMP);
	SelectObject(memDC, oldFont);
	SelectObject(memDC, oldPen);
	DeleteObject(hBitMap);
	DeleteObject(hFont);
	DeleteObject(hPen);
	DeleteDC(memDC);
	ReleaseDC(NULL, hDC);
	delete[] pImageRoot;

	return S_OK;
}
Texture* Font::GetTexture()
{
	return m_pTexture;
}

Font::UV Font::GetUV(const char* word)
{
	Word tmpWord = GetWord(word);
	Strings::iterator it = m_strings.begin();
	while (it != m_strings.end())
	{
		if (it->code == tmpWord.code)
		{
			break;
		}
		++it;
	}
	UINT index = it - m_strings.begin();
	float size = 1.0f / m_strCountOfLine;
	UV uv =
	{
		(index % m_strCountOfLine) * size,
		(index / m_strCountOfLine) * size,
		size,
		size,
	};
	return uv;
}

/**
 * @brief 文字情報取得
 * @param[in] c 先頭文字
 * @return 一文字の情報
 */
Font::Word Font::GetWord(const char* c)
{
	Word word;
	word.byteLen = IsDBCSLeadByte(c[0]) ? 2 : 1;
	if (word.byteLen == 2)
	{
		word.code = (unsigned char)c[0] << 8 | (unsigned char)c[1];
	}
	else
	{
		word.code = (unsigned char)c[0];
	}
	return word;
}

void Font::WriteFont(Word& word, WriteInfo &info, HDC hdc)
{
	// 文字描画
	char string[] = { word.code >> 8, word.code & 0xff };
	if (word.byteLen == 1) { string[0] = word.code; }
	BeginPath(hdc);
	PatBlt(hdc, 0, 0, info.charaTexSize, info.charaTexSize, BLACKNESS);
	TextOutA(hdc, SDF_FONT_OUTER_SIZE, SDF_FONT_OUTER_SIZE, string, word.byteLen);
	EndPath(hdc);
	StrokeAndFillPath(hdc);

	// BMPデータをテクスチャデータに変換
	UINT dstIdx = 0;
	UINT srcIdx = 0;
	for (UINT y = 0; y < info.charaTexSize; ++y)
	{
		dstIdx = info.texSize * y;
		srcIdx = info.bmpPitch * y;
		for (UINT x = 0; x < info.charaTexSize; ++x)
		{
			info.pImage[dstIdx] = info.pBitmap[srcIdx] / 255.f;
			++dstIdx;
			srcIdx += info.bmpPixelSize;
		}
	}
}
void Font::WriteUndefined(WriteInfo &info)
{
	UINT dstIdx = 0;
	for (UINT y = SDF_FONT_OUTER_SIZE; y < info.charaTexSize - SDF_FONT_OUTER_SIZE; ++y)
	{
		dstIdx = info.texSize * y + SDF_FONT_OUTER_SIZE;
		for (UINT x = SDF_FONT_OUTER_SIZE; x < info.charaTexSize - SDF_FONT_OUTER_SIZE; ++x)
		{
			info.pImage[dstIdx] = ((x + y) / 7) % 2;
			++dstIdx;
		}
	}
}

HPEN Font::MakePenHandle()
{
	return (HPEN)CreatePen(PS_NULL, 0, 0);
}
HFONT Font::MakeFontHandle(UINT fontSize)
{
	const char* faceName = "MSゴシック";
	LOGFONTA logFont = {};
	logFont.lfHeight = fontSize;
	logFont.lfCharSet = SHIFTJIS_CHARSET;
	logFont.lfWeight = 0;
	logFont.lfQuality = PROOF_QUALITY;
	memcpy(logFont.lfFaceName, faceName, strlen(faceName));
	return CreateFontIndirectA(&logFont);
}
HBITMAP Font::MakeWriteBitmapHandle(WriteInfo& info)
{
	// api書き込み先の領域を作成
	BITMAPINFO bmpInfo = {};
	bmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmpInfo.bmiHeader.biWidth = info.charaTexSize;
	bmpInfo.bmiHeader.biHeight = -info.charaTexSize;
	bmpInfo.bmiHeader.biPlanes = 1;
	bmpInfo.bmiHeader.biBitCount = 24;
	const UINT Align = 4;
	info.bmpPixelSize = 3;
	info.bmpPitch = (info.charaTexSize * info.bmpPixelSize + (Align - 1)) / Align * Align;
	return CreateDIBSection(0, &bmpInfo, DIB_RGB_COLORS, reinterpret_cast<void**>(&info.pBitmap), 0, 0);
}