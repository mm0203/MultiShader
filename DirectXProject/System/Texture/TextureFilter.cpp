#include <System/Texture/TextureFilter.h>


TextureFilter::TextureFilter()
{
}
TextureFilter::~TextureFilter()
{
}
void TextureFilter::Apply(void* pData, unsigned int dataSize, unsigned int width, unsigned int height)
{
	Pixels pixels = {new char[dataSize], pData, width, height};
	ExecuteApply(pixels);
	memcpy(pData, pixels.pDest, dataSize);
	delete[] pixels.pDest;
}
void TextureFilter::ApplyToAllPixels(Pixels& pixels, Filter &filter, std::function<void(ApplyParam&)> func)
{
	ApplyParam param;

	// �t�B���^�[�T�C�Y�̌v�Z
	switch (filter.size)
	{
	default: param.filterSize = 1; break;
	case Filter::MAT_3X3: param.filterSize = 3; break;
	case Filter::MAT_5X5: param.filterSize = 5; break;
	}
	
	// �t�B���^�[�T�C�Y�����Ƀ��[�v�񐔂̐ݒ�
	param.pPixels = &pixels;
	param.pFilter = &filter;
	param.filterLoopEnd = param.filterSize / 2;
	param.filterLoopStart = -param.filterLoopEnd;

	// ���ׂẲ�f�ɑ΂��ăt�B���^�[�̏��������s
	param.pixelIndex = 0;
	for (param.pixelY = 0; param.pixelY < pixels.height; ++param.pixelY)
	{
		for (param.pixelX = 0; param.pixelX < pixels.width; ++param.pixelX)
		{
			func(param);
			++ param.pixelIndex;
		}
	}
}
void TextureFilter::ApplyFilterToPixel(ApplyParam& param, std::function<void(ApplyParam&)> func)
{
	// �t�B���^�[���[�v
	int filterIndex = 0;
	for (int filterY = param.filterLoopStart; filterY <= param.filterLoopEnd; ++filterY)
	{
		// �s�N�Z����Y�����͈̔͊O�łȂ���
		if (param.pixelY + filterY < 0 || param.pPixels->height <= param.pixelY + filterY)
		{
			filterIndex += param.filterSize;
			continue;
		}
		for (int filterX = param.filterLoopStart; filterX <= param.filterLoopEnd; ++filterX, ++ filterIndex)
		{
			// �s�N�Z����X�����͈̔͊O�łȂ���
			if (param.pixelX + filterX < 0 || param.pPixels->width <= param.pixelX + filterX)
			{
				continue;
			}
			// �t�B���^�[�l�̎擾
			param.filterValue = 1.0f;
			if (param.filterSize == 3)
			{
				param.filterValue = param.pFilter->matrix._3x3[filterIndex];
			}
			else if (param.filterSize == 5)
			{
				param.filterValue = param.pFilter->matrix._5x5[filterIndex];
			}
			// �t�B���^�[�ӏ��̃s�N�Z���C���f�b�N�X�擾
			param.filterPixelIndex = param.pixelIndex + (filterY * param.pPixels->width + filterX);
			func(param);
		}
	}
}