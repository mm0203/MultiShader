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

	// フィルターサイズの計算
	switch (filter.size)
	{
	default: param.filterSize = 1; break;
	case Filter::MAT_3X3: param.filterSize = 3; break;
	case Filter::MAT_5X5: param.filterSize = 5; break;
	}
	
	// フィルターサイズを元にループ回数の設定
	param.pPixels = &pixels;
	param.pFilter = &filter;
	param.filterLoopEnd = param.filterSize / 2;
	param.filterLoopStart = -param.filterLoopEnd;

	// すべての画素に対してフィルターの処理を実行
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
	// フィルターループ
	int filterIndex = 0;
	for (int filterY = param.filterLoopStart; filterY <= param.filterLoopEnd; ++filterY)
	{
		// ピクセルのY方向の範囲外でないか
		if (param.pixelY + filterY < 0 || param.pPixels->height <= param.pixelY + filterY)
		{
			filterIndex += param.filterSize;
			continue;
		}
		for (int filterX = param.filterLoopStart; filterX <= param.filterLoopEnd; ++filterX, ++ filterIndex)
		{
			// ピクセルのX方向の範囲外でないか
			if (param.pixelX + filterX < 0 || param.pPixels->width <= param.pixelX + filterX)
			{
				continue;
			}
			// フィルター値の取得
			param.filterValue = 1.0f;
			if (param.filterSize == 3)
			{
				param.filterValue = param.pFilter->matrix._3x3[filterIndex];
			}
			else if (param.filterSize == 5)
			{
				param.filterValue = param.pFilter->matrix._5x5[filterIndex];
			}
			// フィルター箇所のピクセルインデックス取得
			param.filterPixelIndex = param.pixelIndex + (filterY * param.pPixels->width + filterX);
			func(param);
		}
	}
}