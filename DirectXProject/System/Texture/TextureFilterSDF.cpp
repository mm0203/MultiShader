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
	// �ϐ��錾
	float* pDst = reinterpret_cast<float*>(pixels.pDest);
	float* pSrc = reinterpret_cast<float*>(pixels.pSrc);
	const float cornerDist = 1.414f;
	Filter filter;
	filter.matrix._3x3[4] = 0.0f;

	// �A�h���X�t���b�v�֐�
	// �t�B���^�[�𕡐��񂩂��邽�߁A�������񂾏���ǂݎ���ɃR�s�[����̂ł͂Ȃ��A
	// �ǂݎ���Ə������ݐ�����݂ɓ���ւ��Ȃ��珈�����s���B
	auto AddressFlip = [&pDst, &pSrc]()
	{
		float* p = pDst;
		pDst = pSrc;
		pSrc = p;
	};

	// �t�B���^�[�K�p�֐�
	auto FilterApply = [this, &pDst, &pSrc](ApplyParam& param)
	{
		// ���łɃp�����[�^�ݒ�ς݂Ȃ�R�s�[�̂ݎ��s
		if (pSrc[param.pixelIndex] > 0.0f)
		{
			pDst[param.pixelIndex] = pSrc[param.pixelIndex];
			return;
		}

		// ���Ӄs�N�Z���ւ̋�����ݒ�
		// �߂�(1)�`����(0)�Ƃ��Ă邽�߁A��ԑ傫���l����ԋ߂��l�ƂȂ�
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

	// �O�֍L����t�B���^�[�̐ݒ�
	float outerScale = 1.0f / (m_outerIterator + 1);
	float outerCornerScale = cornerDist * outerScale;
	filter.size = Filter::MAT_3X3;
	filter.matrix._3x3[0] = filter.matrix._3x3[2] = filter.matrix._3x3[6] = filter.matrix._3x3[8] = outerCornerScale;
	filter.matrix._3x3[1] = filter.matrix._3x3[3] = filter.matrix._3x3[5] = filter.matrix._3x3[7] = outerScale;
	// �O���ɍL����
	for (int i = 0; i < m_outerIterator; ++i)
	{
		ApplyToAllPixels(pixels, filter, FilterApply);
		AddressFlip();
	}

	// �����̓����ɂ��t�B���^�[��K�p���邩
	if (!m_innerIterator)
	{
		if (pDst != pixels.pDest)
		{
			AddressFlip();
		}
		return;
	}

	// �����ɍL����t�B���^�[�̐ݒ�
	float innerScale = 1.0f / (m_outerIterator + 1 + m_innerIterator);
	float totalInnerScale = innerScale * m_innerIterator;
	float innerCornerScale = cornerDist * innerScale;
	filter.matrix._3x3[0] = filter.matrix._3x3[2] = filter.matrix._3x3[6] = filter.matrix._3x3[8] = innerCornerScale;
	filter.matrix._3x3[1] = filter.matrix._3x3[3] = filter.matrix._3x3[5] = filter.matrix._3x3[7] = innerScale;
	
	// �����ɍL���邽�߂ɁA�l�𔽓]
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
	// �����ɍL����
	filter.size = Filter::MAT_3X3;
	for (int i = 0; i < m_innerIterator; ++i)
	{
		ApplyToAllPixels(pixels, filter, FilterApply);
		AddressFlip();
	}

	// ������x���]���Č��̏�Ԃɖ߂�
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