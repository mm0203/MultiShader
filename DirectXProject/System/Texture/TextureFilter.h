#ifndef __TEXTURE_FILTER_H__
#define __TEXTRUE_FILTER_H__

#include <dxgiformat.h>
#include <functional>

class TextureFilter
{
protected:
	struct Filter
	{
		enum Size
		{
			MAT_NONE,
			MAT_3X3,
			MAT_5X5,
		};
		union Matrix
		{
			float _3x3[9];
			float _5x5[25];
		};

		Size size;
		Matrix matrix;
	};
	struct Pixels
	{
		void* pDest;
		void* pSrc;
		unsigned int width;
		unsigned int height;
	};
	struct ApplyParam
	{
		Pixels* pPixels;
		Filter* pFilter;
		int pixelX;
		int pixelY;
		int pixelIndex;

		int filterSize;
		int filterLoopStart;
		int filterLoopEnd;
		float filterValue;
		int filterPixelIndex;
	};
public:
	TextureFilter();
	virtual ~TextureFilter();
	void Apply(void* pData, unsigned int dataSize, unsigned int width, unsigned int height);

	virtual bool CheckFormat(DXGI_FORMAT format) = 0;

protected:
	virtual void ExecuteApply(Pixels &pixels) = 0;
	
	void ApplyToAllPixels(Pixels &pixels, Filter &filter, std::function<void(ApplyParam&)> func);
	void ApplyFilterToPixel(ApplyParam& param, std::function<void(ApplyParam&)> func);
};

#endif