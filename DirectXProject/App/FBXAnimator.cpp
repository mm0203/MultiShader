#include <App/FBXAnimator.h>
#include <System/Graphics/Geometory.h>

FBXAnimator::FBXAnimator()
	: m_pFBXBone(nullptr)
	, m_pBones(nullptr)
	, m_playAnimeNo(-1)
	, m_blendAnimeNo(-1)
	, m_blendFrame(0)
{
	for (int i = 0; i < MaxAnime; ++i)
	{
		m_pAnimes[i] = nullptr;
	}
	for (int i = 0; i < MaxBlend; ++i)
	{
		m_pBlendBones[i] = nullptr;
	}
}
FBXAnimator::~FBXAnimator()
{
	Reset();
}

// �߂�l�Ŕz��̂ǂ̈ʒu�ɃA�j���[�V�����̏����i�[�������Ԃ�
// �ǂݍ��߂Ȃ���������-1��Ԃ�
int FBXAnimator::Load(const char* fileName)
{
	// �z��ɋ󂫂����邩
	int animeIndex = -1;
	for (int i = 0; i < MaxAnime; ++i)
	{
		if (m_pAnimes[i] == nullptr)
		{
			animeIndex = i;
			break;
		}
	}
	if (animeIndex == -1)
	{
		return animeIndex;
	}

	// �A�j���[�V�����̓ǂݍ���
	m_pAnimes[animeIndex] = new AnimeInfo;
	m_pAnimes[animeIndex]->pAnime = new ggfbx::AnimationInfo;
	if (!ggfbx::Load(fileName, nullptr, nullptr, nullptr, m_pAnimes[animeIndex]->pAnime))
	{
		// �ǂݍ��݂Ɏ��s������A�z��̊Y���ӏ��𑁋}�ɋ󂯂�
		delete m_pAnimes[animeIndex]->pAnime;
		delete m_pAnimes[animeIndex];
		m_pAnimes[animeIndex] = nullptr;
		return -1;
	}

	// ���̑�������
	m_pAnimes[animeIndex]->playFrame = 0.0f;
	m_pAnimes[animeIndex]->loop = false;
	m_pAnimes[animeIndex]->speed = 1.0f;

	return animeIndex;
}

// �A�j���[�V�����̍Đ��w��
void FBXAnimator::PlayAnime(int animeNo, bool isLoop)
{
	// �Đ��`�F�b�N
	if (animeNo < 0 || animeNo >= MaxAnime || !m_pAnimes[animeNo])
	{
		return;
	}

	m_playAnimeNo = animeNo;
	m_pAnimes[m_playAnimeNo]->playFrame = 0.0f;
	m_pAnimes[m_playAnimeNo]->loop = isLoop;
}
// �J�ڱ�Ұ��݂̎w��
void FBXAnimator::PlayBlendAnime(int blendNo, float time, bool isLoop)
{
	// �Đ��`�F�b�N
	if (m_playAnimeNo == blendNo || m_blendAnimeNo == blendNo)
	{
		return;
	}
	if (blendNo < 0 || blendNo >= MaxAnime || !m_pAnimes[blendNo])
	{
		return;
	}

	m_blendAnimeNo = blendNo;
	m_blendFrame = time * 60;
	m_pAnimes[m_blendAnimeNo]->playFrame = 0.0f;
	m_pAnimes[m_blendAnimeNo]->loop = isLoop;
}

// �A�j���[�V�����Đ�����
bool FBXAnimator::IsPlayAnime(int animeNo)
{
	// �w�肳�ꂽ�A�j���[�V�������Đ�����Ă��邩
	if (animeNo != -1)
	{
		if (m_playAnimeNo != animeNo)
		{
			return false;
		}
	}

	// ���[�v�A�j���Ȃ��΍Đ����Ă���
	if (m_pAnimes[m_playAnimeNo]->loop)
	{
		return true;
	}
	// �P���Đ��A�j���̏I���t���[���܂ōĐ�������
	if (m_pAnimes[m_playAnimeNo]->playFrame >=
		m_pAnimes[m_playAnimeNo]->pAnime->totalFrame)
	{
		return false;
	}

	// �܂��Đ����̃A�j��������
	return true;
}

void FBXAnimator::Step(float speed)
{
	// �A�j���[�V�����̍X�V
	if (m_playAnimeNo != -1)
	{
		// ���ݍĐ����̃A�j���[�V�������v�Z
		CalcAnimeFrame(m_playAnimeNo, speed);
		CalcAnime(m_playAnimeNo, m_pBlendBones[0]);

		// �J�ڃA�j���̍X�V����
		if (m_blendAnimeNo != -1)
		{
			CalcAnimeFrame(m_blendAnimeNo, speed);
			CalcAnime(m_blendAnimeNo, m_pBlendBones[1]);
			// �A�j���[�V�����̍���
			// �J�ڎ��Ԃ��獬�����킹�銄�����v�Z
			float blend =
				m_pAnimes[m_blendAnimeNo]->playFrame / m_blendFrame;
			for (int i = 0; i < m_pFBXBone->list.size(); ++i)
			{
				// �������킹�鎞�͕K��100%�ɂȂ�悤�Ɍv�Z
				// (1 - blend) + blend
				m_pBones[i].param.translate.x =
					m_pBlendBones[0][i].translate.x * (1.f - blend) +
					m_pBlendBones[1][i].translate.x * blend;
				m_pBones[i].param.translate.y =
					m_pBlendBones[0][i].translate.y * (1.f - blend) +
					m_pBlendBones[1][i].translate.y * blend;
				m_pBones[i].param.translate.z =
					m_pBlendBones[0][i].translate.z * (1.f - blend) +
					m_pBlendBones[1][i].translate.z * blend;
				m_pBones[i].param.rotation.x =
					m_pBlendBones[0][i].rotation.x * (1.f - blend) +
					m_pBlendBones[1][i].rotation.x * blend;
				m_pBones[i].param.rotation.y =
					m_pBlendBones[0][i].rotation.y * (1.f - blend) +
					m_pBlendBones[1][i].rotation.y * blend;
				m_pBones[i].param.rotation.z =
					m_pBlendBones[0][i].rotation.z * (1.f - blend) +
					m_pBlendBones[1][i].rotation.z * blend;
				m_pBones[i].param.scale.x =
					m_pBlendBones[0][i].scale.x * (1.f - blend) +
					m_pBlendBones[1][i].scale.x * blend;
				m_pBones[i].param.scale.y =
					m_pBlendBones[0][i].scale.y * (1.f - blend) +
					m_pBlendBones[1][i].scale.y * blend;
				m_pBones[i].param.scale.z =
					m_pBlendBones[0][i].scale.z * (1.f - blend) +
					m_pBlendBones[1][i].scale.z * blend;
			}
			// ���[�V�����̍������킹���I�������A
			// �ʏ�̃A�j���ɕύX
			if (blend >= 1.0f)
			{
				m_playAnimeNo = m_blendAnimeNo;
				m_blendAnimeNo = -1;
			}

		}
		else
		{
			// �J�ڂ��Ȃ��̂ŒP���f�[�^���R�s�[
			for (int i = 0; i < m_pFBXBone->list.size(); ++i)
			{
				m_pBones[i].param = m_pBlendBones[0][i];
			}
		}
	}

	// m_pBones[10].rotation.z = 90;

	// ���̎p���X�V
	CalcBone(m_pFBXBone->root, DirectX::XMMatrixIdentity());
}

DirectX::XMFLOAT4X4 FBXAnimator::GetBone(int index)
{
	if (index < 0 || m_pFBXBone->list.size() <= index)
	{
		return DirectX::XMFLOAT4X4(
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1);
	}

	return m_pBones[index].matrix;
}

void FBXAnimator::Reset()
{
	if (m_pFBXBone)
	{
		delete m_pFBXBone;
		m_pFBXBone = nullptr;
	}
	if (m_pBones)
	{
		delete[] m_pBones;
		m_pBones = nullptr;
	}
	for (int i = 0; i < MaxAnime; ++i)
	{
		if (m_pAnimes[i])
		{
			delete m_pAnimes[i]->pAnime;
			delete m_pAnimes[i];
			m_pAnimes[i] = nullptr;
		}
	}
	for (int i = 0; i < MaxBlend; ++i)
	{
		if (m_pBlendBones[i])
		{
			delete m_pBlendBones[i];
			m_pBlendBones[i] = nullptr;
		}
	}
}

/// @param index ���̔ԍ�
/// @param parent �e�̎p�����
void FBXAnimator::CalcBone(int index, DirectX::XMMATRIX parent)
{
	// �p���̌v�Z
	DirectX::XMMATRIX T =
		DirectX::XMMatrixTranslation(
			m_pBones[index].param.translate.x,
			m_pBones[index].param.translate.y,
			m_pBones[index].param.translate.z);
	DirectX::XMMATRIX R =
		DirectX::XMMatrixRotationX(
			DirectX::XMConvertToRadians(m_pBones[index].param.rotation.x)) *
		DirectX::XMMatrixRotationY(
			DirectX::XMConvertToRadians(m_pBones[index].param.rotation.y)) *
		DirectX::XMMatrixRotationZ(
			DirectX::XMConvertToRadians(m_pBones[index].param.rotation.z));
	DirectX::XMMATRIX S =
		DirectX::XMMatrixScaling(
			m_pBones[index].param.scale.x,
			m_pBones[index].param.scale.y,
			m_pBones[index].param.scale.z);
	// ���݂̎p�� = ���ڂ��Ă��鍜�̎p�� * �e�̍��̎p��
	DirectX::XMMATRIX mat =
		S * R * m_pBones[index].preRotate * T * parent;
	DirectX::XMStoreFloat4x4(&m_pBones[index].matrix, mat);

	// ���̍��̌v�Z(�Ⴆ�΍������v�Z�������
	// �e�w�A�l���w�A���w�A��w�A���w�ƂT�v�Z���Ȃ��Ƃ����Ȃ���������
	for (int i = 0; i < m_pFBXBone->list[index].children.size(); ++i)
	{
		CalcBone(m_pFBXBone->list[index].children[i], mat);
	}
}
// ����`�悷��֐�
void FBXAnimator::DrawBone(int index, DirectX::XMFLOAT3 parent)
{
	// ���̕`��
	DirectX::XMFLOAT3 pos(0,0,0);
	DirectX::XMVECTOR vPos = DirectX::XMLoadFloat3(&pos);
	DirectX::XMMATRIX mMat = DirectX::XMLoadFloat4x4(&m_pBones[index].matrix);
	vPos = DirectX::XMVector3TransformCoord(vPos, mMat);
	DirectX::XMStoreFloat3(&pos, vPos);
	AddLine(parent, pos);

	// ���������悤�ł���Ζ��[�܂ŏ���
	for (int i = 0; i < m_pFBXBone->list[index].children.size(); ++i)
	{
		DrawBone(m_pFBXBone->list[index].children[i], pos);
	}
}

// �A�j���[�V�����̎��Ԍo�߂��v�Z
void FBXAnimator::CalcAnimeFrame(int animeNo, float speed)
{
	if (animeNo < 0 || animeNo >= MaxAnime || !m_pAnimes[animeNo])
	{
		return;
	}

	m_pAnimes[animeNo]->playFrame += speed;
	// ���[�v����
	if (m_pAnimes[animeNo]->loop)
	{
		while (m_pAnimes[animeNo]->playFrame >=
			m_pAnimes[animeNo]->pAnime->totalFrame)
		{
			m_pAnimes[animeNo]->playFrame -=
				m_pAnimes[animeNo]->pAnime->totalFrame;
		}
	}
}
// �A�j���[�V�����̃t���[���ɉ����č��̃|�[�Y���v�Z
void FBXAnimator::CalcAnime(int animeNo, MatrixParam* pBones)
{
	// �Đ��`�F�b�N
	if (animeNo < 0 || animeNo >= MaxAnime || !m_pAnimes[animeNo])
	{
		return;
	}

	// �A�j���[�V�����œ������Ȃ��Ƃ����Ȃ�����
	// �ǂݍ��ݍς݂̑S�g�̍��̒�����T��
	// curveList ... �������Ȃ��Ƃ����Ȃ��������̓��I�z��i�����Ȃ����͊܂܂�Ȃ�
	float frame = m_pAnimes[animeNo]->playFrame;
	ggfbx::AnimationInfo* pAnime = m_pAnimes[animeNo]->pAnime;
	for (int i = 0; i < pAnime->curveList.size(); ++i)
	{
		// ���������Ƃ��Ă��鍜���ǂ��ɊY������̂�����
		int boneIdx = -1;
		for (int j = 0; j < m_pFBXBone->list.size(); ++j)
		{
			if (pAnime->curveList[i].name == m_pFBXBone->list[j].name)
			{
				boneIdx = j;
				break;
			}
		}
		if (boneIdx == -1)
		{
			continue;
		}


		// �����������ɑ΂��ăA�j���[�V�����̏��𗬂�����
		/*
		�A�j���[�V�����̏�� = �����ǂ������� �Ȃ̂ŁA�A�j���[�V����������
		���͈ړ��A��]�A�g�k�̂R�B����Ɋe�R�̏��ɂ�x,y,z��3�̐�����
		�܂݁A�v9�̃f�[�^���ۑ�����Ă���B
		�Ⴆ�Ύ�����ɐU��A�j���[�V�����̏ꍇ�A�ړ��Ɗg�k�͍s��Ȃ��B�܂��A
		��]��x����z���ł̉�]�͍s��Ȃ��B(y������ɉ�]����
		*/
		MatrixParam* pBone = &pBones[boneIdx];
		float* defValue[] = {
			&pBone->translate.x, &pBone->translate.y, &pBone->translate.z,
			&pBone->rotation.x, &pBone->rotation.y, &pBone->rotation.z,
			&pBone->scale.x, &pBone->scale.y, &pBone->scale.z
		};
		for (int j = 0; j < 9; ++j)
		{
			*defValue[j] = CalcKey(
				pAnime->curveList[i].keyList[j], frame, *defValue[j]);
		}
	}
}

// �A�j���[�V�����̃t���[���ɑΉ������p�����v�Z
float FBXAnimator::CalcKey(ggfbx::AnimationInfo::CurveInfo::List& keys, float frame, float defValue)
{
	/*
	�A�j���[�V�����̊�{�̓L�[�t���[��(�����A���X�ؒ���
	�L�[�t���[���́w�ǂ̎��ԂɁx�w�ǂ������|�[�Y�ɂȂ邩�x�Ƃ���
	�Q�̏������B���̏�񂪑����i�[����Ă���B
	*/

	// �L�[�t���[��������Ȃ��ꍇ�́A�f�t�H���g�̒l���g�p
	if (keys.size() == 0)
	{
		return defValue;
	}

	// ���݂̎��Ԃ���g�p����L�[�t���[���̃f�[�^��T������
	// ��Ұ��݂��̂���   ������      9�̃f�[�^  ���̷��ڰ�
	//    pAnime   -> curveList[i].  keyList[0]    [0]

	for (int i = 1; i < keys.size(); i++)
	{
		// �L�[�t���[���̊J�n���ԂƏI�����Ԃ̊Ԃɋ��܂�Ă��邩
		float startF = keys[i - 1].frame;
		float endF = keys[i].frame;
		if (startF <= frame && frame < endF)
		{
			// ���`��Ԃ𗘗p���āA�t���[���Ԃ̃f�[�^(�p��)��
			// �ǂ��Ȃ��Ă��邩�v�Z����

			// ���`��Ԃ̎�
			// �p�� = (�I���̎p�� - �J�n�̎p��) * ���ׂ������� + �J�n�̎p��
			float startV = keys[i - 1].value;
			float endV = keys[i].value;
			// 0~1�ɃX�P�[�����ꂽ���� = (���݂̎��� - �J�n����) / (�I������ - �J�n����)
			float rate = (frame - startF) / (endF - startF);

			return (endV - startV) * rate + startV;
		}
	}

	// ������Ȃ������ꍇ�A�Ō�̎p�������̂܂܎g�p
	return keys[keys.size() - 1].value;
}