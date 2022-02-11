#ifndef __TEXTURE_FILTER_SDF_H__
#define __TEXTURE_FILTER_SDF_H__

#include <system/Texture/TextureFilter.h>

class TextureFilterSDF : public TextureFilter
{
private:
public:
	TextureFilterSDF(int iterator);
	TextureFilterSDF(int outerIterator, int innerIterator);
	~TextureFilterSDF();

	bool CheckFormat(DXGI_FORMAT format);

private:
	void ExecuteApply(Pixels& pixels);

private:
	int m_outerIterator; // �O�g�J��Ԃ���
	int m_innerIterator; // ���g�J��Ԃ���
};


#endif