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

// 戻り値で配列のどの位置にアニメーションの情報を格納したか返す
// 読み込めなかった時は-1を返す
int FBXAnimator::Load(const char* fileName)
{
	// 配列に空きがあるか
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

	// アニメーションの読み込み
	m_pAnimes[animeIndex] = new AnimeInfo;
	m_pAnimes[animeIndex]->pAnime = new ggfbx::AnimationInfo;
	if (!ggfbx::Load(fileName, nullptr, nullptr, nullptr, m_pAnimes[animeIndex]->pAnime))
	{
		// 読み込みに失敗したら、配列の該当箇所を早急に空ける
		delete m_pAnimes[animeIndex]->pAnime;
		delete m_pAnimes[animeIndex];
		m_pAnimes[animeIndex] = nullptr;
		return -1;
	}

	// その他初期化
	m_pAnimes[animeIndex]->playFrame = 0.0f;
	m_pAnimes[animeIndex]->loop = false;
	m_pAnimes[animeIndex]->speed = 1.0f;

	return animeIndex;
}

// アニメーションの再生指定
void FBXAnimator::PlayAnime(int animeNo, bool isLoop)
{
	// 再生チェック
	if (animeNo < 0 || animeNo >= MaxAnime || !m_pAnimes[animeNo])
	{
		return;
	}

	m_playAnimeNo = animeNo;
	m_pAnimes[m_playAnimeNo]->playFrame = 0.0f;
	m_pAnimes[m_playAnimeNo]->loop = isLoop;
}
// 遷移ｱﾆﾒｰｼｮﾝの指定
void FBXAnimator::PlayBlendAnime(int blendNo, float time, bool isLoop)
{
	// 再生チェック
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

// アニメーション再生判定
bool FBXAnimator::IsPlayAnime(int animeNo)
{
	// 指定されたアニメーションが再生されているか
	if (animeNo != -1)
	{
		if (m_playAnimeNo != animeNo)
		{
			return false;
		}
	}

	// ループアニメなら絶対再生している
	if (m_pAnimes[m_playAnimeNo]->loop)
	{
		return true;
	}
	// 単発再生アニメの終了フレームまで再生したか
	if (m_pAnimes[m_playAnimeNo]->playFrame >=
		m_pAnimes[m_playAnimeNo]->pAnime->totalFrame)
	{
		return false;
	}

	// まだ再生中のアニメだった
	return true;
}

void FBXAnimator::Step(float speed)
{
	// アニメーションの更新
	if (m_playAnimeNo != -1)
	{
		// 現在再生中のアニメーションを計算
		CalcAnimeFrame(m_playAnimeNo, speed);
		CalcAnime(m_playAnimeNo, m_pBlendBones[0]);

		// 遷移アニメの更新処理
		if (m_blendAnimeNo != -1)
		{
			CalcAnimeFrame(m_blendAnimeNo, speed);
			CalcAnime(m_blendAnimeNo, m_pBlendBones[1]);
			// アニメーションの合成
			// 遷移時間から混ぜ合わせる割合を計算
			float blend =
				m_pAnimes[m_blendAnimeNo]->playFrame / m_blendFrame;
			for (int i = 0; i < m_pFBXBone->list.size(); ++i)
			{
				// 混ぜ合わせる時は必ず100%になるように計算
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
			// モーションの混ぜ合わせが終わったら、
			// 通常のアニメに変更
			if (blend >= 1.0f)
			{
				m_playAnimeNo = m_blendAnimeNo;
				m_blendAnimeNo = -1;
			}

		}
		else
		{
			// 遷移しないので単発データをコピー
			for (int i = 0; i < m_pFBXBone->list.size(); ++i)
			{
				m_pBones[i].param = m_pBlendBones[0][i];
			}
		}
	}

	// m_pBones[10].rotation.z = 90;

	// 骨の姿勢更新
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

/// @param index 骨の番号
/// @param parent 親の姿勢情報
void FBXAnimator::CalcBone(int index, DirectX::XMMATRIX parent)
{
	// 姿勢の計算
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
	// 現在の姿勢 = 注目している骨の姿勢 * 親の骨の姿勢
	DirectX::XMMATRIX mat =
		S * R * m_pBones[index].preRotate * T * parent;
	DirectX::XMStoreFloat4x4(&m_pBones[index].matrix, mat);

	// 次の骨の計算(例えば左手首を計算した後は
	// 親指、人差指、中指、薬指、小指と５個計算しないといけない骨がある
	for (int i = 0; i < m_pFBXBone->list[index].children.size(); ++i)
	{
		CalcBone(m_pFBXBone->list[index].children[i], mat);
	}
}
// 骨を描画する関数
void FBXAnimator::DrawBone(int index, DirectX::XMFLOAT3 parent)
{
	// 骨の描画
	DirectX::XMFLOAT3 pos(0,0,0);
	DirectX::XMVECTOR vPos = DirectX::XMLoadFloat3(&pos);
	DirectX::XMMATRIX mMat = DirectX::XMLoadFloat4x4(&m_pBones[index].matrix);
	vPos = DirectX::XMVector3TransformCoord(vPos, mMat);
	DirectX::XMStoreFloat3(&pos, vPos);
	AddLine(parent, pos);

	// 骨が続くようであれば末端まで処理
	for (int i = 0; i < m_pFBXBone->list[index].children.size(); ++i)
	{
		DrawBone(m_pFBXBone->list[index].children[i], pos);
	}
}

// アニメーションの時間経過を計算
void FBXAnimator::CalcAnimeFrame(int animeNo, float speed)
{
	if (animeNo < 0 || animeNo >= MaxAnime || !m_pAnimes[animeNo])
	{
		return;
	}

	m_pAnimes[animeNo]->playFrame += speed;
	// ループ処理
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
// アニメーションのフレームに応じて骨のポーズを計算
void FBXAnimator::CalcAnime(int animeNo, MatrixParam* pBones)
{
	// 再生チェック
	if (animeNo < 0 || animeNo >= MaxAnime || !m_pAnimes[animeNo])
	{
		return;
	}

	// アニメーションで動かさないといけない骨を
	// 読み込み済みの全身の骨の中から探す
	// curveList ... 動かさないといけない骨だけの動的配列（動かない骨は含まれない
	float frame = m_pAnimes[animeNo]->playFrame;
	ggfbx::AnimationInfo* pAnime = m_pAnimes[animeNo]->pAnime;
	for (int i = 0; i < pAnime->curveList.size(); ++i)
	{
		// 動かそうとしている骨がどこに該当するのか検索
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


		// 見つかった骨に対してアニメーションの情報を流し込む
		/*
		アニメーションの情報 = 骨がどう動くか なので、アニメーションが持つ
		情報は移動、回転、拡縮の３つ。さらに各３つの情報にはx,y,zの3つの成分を
		含み、計9個のデータが保存されている。
		例えば首を横に振るアニメーションの場合、移動と拡縮は行わない。また、
		回転もx軸とz軸での回転は行わない。(y軸を基準に回転する
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

// アニメーションのフレームに対応した姿勢を計算
float FBXAnimator::CalcKey(ggfbx::AnimationInfo::CurveInfo::List& keys, float frame, float defValue)
{
	/*
	アニメーションの基本はキーフレーム(多分、佐々木調べ
	キーフレームは『どの時間に』『どういうポーズになるか』という
	２つの情報をもつ。この情報が多数格納されている。
	*/

	// キーフレームが一つもない場合は、デフォルトの値を使用
	if (keys.size() == 0)
	{
		return defValue;
	}

	// 現在の時間から使用するキーフレームのデータを探索する
	// ｱﾆﾒｰｼｮﾝそのもの   動く骨      9個のデータ  そのｷｰﾌﾚｰﾑ
	//    pAnime   -> curveList[i].  keyList[0]    [0]

	for (int i = 1; i < keys.size(); i++)
	{
		// キーフレームの開始時間と終了時間の間に挟まれているか
		float startF = keys[i - 1].frame;
		float endF = keys[i].frame;
		if (startF <= frame && frame < endF)
		{
			// 線形補間を利用して、フレーム間のデータ(姿勢)が
			// どうなっているか計算する

			// 線形補間の式
			// 姿勢 = (終了の姿勢 - 開始の姿勢) * 調べたい時間 + 開始の姿勢
			float startV = keys[i - 1].value;
			float endV = keys[i].value;
			// 0~1にスケールされた時間 = (現在の時間 - 開始時間) / (終了時間 - 開始時間)
			float rate = (frame - startF) / (endF - startF);

			return (endV - startV) * rate + startV;
		}
	}

	// 見つからなかった場合、最後の姿勢をそのまま使用
	return keys[keys.size() - 1].value;
}