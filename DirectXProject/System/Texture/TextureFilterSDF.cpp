#include <System/Texture/TextureFilterSDF.h>


TextureFilterSDF::TextureFilterSDF(int iterator)
	: TextureFilterSDF(iterator, iterator / 4 + 1)
{
}
TextureFilterSDF::TextureFilterSDF(int outerIterator, int innerIterator)
	: m_outerIterator(outerIterator), m_innerIterator(innerIterator)
{
}
TextureFilterSDF::~TextureFilterSDF()
{
}
bool TextureFilterSDF::CheckFormat(DXGI_FORMAT format)
{
	return format == DXGI_FORMAT_R32_FLOAT;
}
void TextureFilterSDF::ExecuteApply(Pixels& pixels)
{
	// 変数宣言
	float* pDst = reinterpret_cast<float*>(pixels.pDest);
	float* pSrc = reinterpret_cast<float*>(pixels.pSrc);
	const float cornerDist = 1.414f;
	Filter filter;
	filter.matrix._3x3[4] = 0.0f;

	// アドレスフリップ関数
	// フィルターを複数回かけるため、書き込んだ情報を読み取り先にコピーするのではなく、
	// 読み取り先と書き込み先を交互に入れ替えながら処理を行う。
	auto AddressFlip = [&pDst, &pSrc]()
	{
		float* p = pDst;
		pDst = pSrc;
		pSrc = p;
	};

	// フィルター適用関数
	auto FilterApply = [this, &pDst, &pSrc](ApplyParam& param)
	{
		// すでにパラメータ設定済みならコピーのみ実行
		if (pSrc[param.pixelIndex] > 0.0f)
		{
			pDst[param.pixelIndex] = pSrc[param.pixelIndex];
			return;
		}

		// 周辺ピクセルへの距離を設定
		// 近い(1)～遠い(0)としてるため、一番大きい値が一番近い値となる
		float maxNear = 0.0f;
		ApplyFilterToPixel(param,
			[&maxNear, &pSrc](ApplyParam& param)->void
			{
				float dist = pSrc[param.filterPixelIndex] - param.filterValue;
				if (dist > maxNear)
				{
					maxNear = dist;
				}
			}
		);
		pDst[param.pixelIndex] = maxNear;
	};

	// 外へ広がるフィルターの設定
	float outerScale = 1.0f / (m_outerIterator + 1);
	float outerCornerScale = cornerDist * outerScale;
	filter.size = Filter::MAT_3X3;
	filter.matrix._3x3[0] = filter.matrix._3x3[2] = filter.matrix._3x3[6] = filter.matrix._3x3[8] = outerCornerScale;
	filter.matrix._3x3[1] = filter.matrix._3x3[3] = filter.matrix._3x3[5] = filter.matrix._3x3[7] = outerScale;
	// 外側に広げる
	for (int i = 0; i < m_outerIterator; ++i)
	{
		ApplyToAllPixels(pixels, filter, FilterApply);
		AddressFlip();
	}

	// 文字の内側にもフィルターを適用するか
	if (!m_innerIterator)
	{
		if (pDst != pixels.pDest)
		{
			AddressFlip();
		}
		return;
	}

	// 内側に広げるフィルターの設定
	float innerScale = 1.0f / (m_outerIterator + 1 + m_innerIterator);
	float totalInnerScale = innerScale * m_innerIterator;
	float innerCornerScale = cornerDist * innerScale;
	filter.matrix._3x3[0] = filter.matrix._3x3[2] = filter.matrix._3x3[6] = filter.matrix._3x3[8] = innerCornerScale;
	filter.matrix._3x3[1] = filter.matrix._3x3[3] = filter.matrix._3x3[5] = filter.matrix._3x3[7] = innerScale;
	
	// 内側に広げるために、値を反転
	filter.size = Filter::MAT_NONE;
	ApplyToAllPixels(pixels, filter,
		[&pDst, &pSrc, totalInnerScale](ApplyParam &param)
		{
			if (pSrc[param.pixelIndex] >= 1.0f)
			{
				pDst[param.pixelIndex] = 0.0f;
			}
			else
			{
				pDst[param.pixelIndex] = 1.f - pSrc[param.pixelIndex] * (1.0f - totalInnerScale);
			}
		}
	);
	AddressFlip();
	// 内側に広げる
	filter.size = Filter::MAT_3X3;
	for (int i = 0; i < m_innerIterator; ++i)
	{
		ApplyToAllPixels(pixels, filter, FilterApply);
		AddressFlip();
	}

	// もう一度反転して元の状態に戻す
	if (pDst != pixels.pDest)
	{
		AddressFlip();
	}
	filter.size = Filter::MAT_NONE;
	ApplyToAllPixels(pixels, filter,
		[&pDst](ApplyParam& param)
		{
			pDst[param.pixelIndex] = 1.f - pDst[param.pixelIndex];
		}
	);
}