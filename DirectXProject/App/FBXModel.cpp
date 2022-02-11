#include <App/FBXModel.h>
#include <System/FBX/FBXLoader.h>

FBXModel::FBXModel()
{
}
FBXModel::~FBXModel()
{
	Reset();
}
void FBXModel::Reset()
{
	MeshList::iterator it = m_meshList.begin();
	while(it != m_meshList.end())
	{
		SAFE_DELETE(it->pSkinList);
		SAFE_DELETE_ARRAY(it->pVtx);
		++it;
	}
	m_meshList.clear();
}

HRESULT FBXModel::Load(const char* fileName)
{
	// メモリのクリア
	Reset();

	// 読み込み
	HRESULT hr = E_FAIL;
	ggfbx::MeshList mesh;
	if (!ggfbx::Load(fileName, &mesh, nullptr, nullptr, nullptr))
	{
		return hr;
	}

	// メッシュごとに処理
	m_meshList.resize(mesh.size());
	MeshList::iterator modelIt = m_meshList.begin();
	ggfbx::MeshList::iterator fbxIt = mesh.begin();
	while (fbxIt != mesh.end())
	{
		// インデックスバッファを利用して描画するか検証
		bool useNormalIndex = fbxIt->normalInfo.mapping == ggfbx::MAPPING_VERTEX;
		bool useUVIndex = fbxIt->uvList.size() > 0 && fbxIt->uvList[0].mapping == ggfbx::MAPPING_VERTEX; // 単一uvのみ
		bool useIndex = useNormalIndex && useUVIndex;

		// スキンメッシュデータ確認
		if (fbxIt->skinInfo.weightList.size() > 0)
		{
			modelIt->pSkinList = new SkinList;
		}
		else
		{
			modelIt->pSkinList = nullptr;
		}

		// 頂点構造体設定
		UINT vtxNum = static_cast<UINT>(useIndex ? fbxIt->vertexList.size() : fbxIt->indexList.size());
		UINT vtxStride = sizeof(Vertex) + (modelIt->pSkinList ? sizeof(Blend) : 0);
		modelIt->pVtx = new char[vtxStride * vtxNum];
		Vertex* pVtx	= reinterpret_cast<Vertex*>(modelIt->pVtx);
		Blend* pBlend	= reinterpret_cast<Blend*>(pVtx+1);

		// 頂点データ読み込み
		DirectX::XMVECTOR vec;
		for (UINT i = 0; i < vtxNum; ++i)
		{
			// 読み取りインデックス設定
			UINT vtxIndex	 = i;
			UINT normalIndex = i;
			UINT uvIndex	 = i;
			if (!useIndex)
			{
				vtxIndex = fbxIt->indexList[i];
				if (useNormalIndex) { normalIndex = vtxIndex; }
				if (useUVIndex) { uvIndex = vtxIndex; }
			}

			// 座標
			vec = DirectX::XMVector3TransformCoord(
				DirectX::XMVectorSet(
					fbxIt->vertexList[vtxIndex].x,
					fbxIt->vertexList[vtxIndex].y,
					fbxIt->vertexList[vtxIndex].z, 0),
				DirectX::XMMatrixSet(
					fbxIt->transform._11, fbxIt->transform._12, fbxIt->transform._13, fbxIt->transform._14,
					fbxIt->transform._21, fbxIt->transform._22, fbxIt->transform._23, fbxIt->transform._24,
					fbxIt->transform._31, fbxIt->transform._32, fbxIt->transform._33, fbxIt->transform._34,
					fbxIt->transform._41, fbxIt->transform._42, fbxIt->transform._43, fbxIt->transform._44)
			);
			DirectX::XMStoreFloat3(&pVtx->pos, vec);
			// 法線
			if (fbxIt->normalInfo.list.size() > 0)
			{
				vec = DirectX::XMVector3TransformCoord(
					DirectX::XMVectorSet(
						fbxIt->normalInfo.list[normalIndex].x,
						fbxIt->normalInfo.list[normalIndex].y,
						fbxIt->normalInfo.list[normalIndex].z, 0),
					DirectX::XMMatrixSet(
						fbxIt->transform._11, fbxIt->transform._12, fbxIt->transform._13, 0,
						fbxIt->transform._21, fbxIt->transform._22, fbxIt->transform._23, 0,
						fbxIt->transform._31, fbxIt->transform._32, fbxIt->transform._33, 0,
						0, 0, 0, 1)
				);
				DirectX::XMStoreFloat3(&pVtx->normal, vec);
			}
			else
			{
				pVtx->normal = DirectX::XMFLOAT3(0, 0, 1);
			}
			// UV
			if (fbxIt->uvList.size() > 0)
			{
				pVtx->uv.x = fbxIt->uvList[0].list[uvIndex].x;
				pVtx->uv.y = 1.0f - fbxIt->uvList[0].list[uvIndex].y;
			}
			else
			{
				pVtx->uv = DirectX::XMFLOAT2(0.0f, 0.0f);
			}
			// ウェイト
			if (modelIt->pSkinList)
			{
				for (UINT j = 0; j < ggfbx::MeshInfo::SkinInfo::WeightInfo::WEIGHT_NUM_MAX; ++j)
				{
					auto value = fbxIt->skinInfo.weightList[vtxIndex].value[j];
					pBlend->weight[j] = value.weight;
					pBlend->index[j] = value.index;
				}
				pBlend = reinterpret_cast<Blend*>(reinterpret_cast<char*>(pBlend) + vtxStride);
			}

			pVtx = reinterpret_cast<Vertex*>(reinterpret_cast<char*>(pVtx) + vtxStride);
		}
		
		// バッファ構築
		DXBuffer::Desc desc;
		desc.pVtx = modelIt->pVtx;
		desc.vtxSize = vtxStride;
		desc.vtxCount = vtxNum;
		desc.isWrite = false;
		desc.topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		if (useIndex)
		{
			UINT* pIdx = new UINT[fbxIt->indexList.size()];
			for (UINT i = 0; i < fbxIt->indexList.size(); ++i)
			{
				pIdx[i] = fbxIt->indexList[i];
			}
			desc.pIdx = pIdx;
			desc.idxSize = sizeof(UINT);
			desc.idxCount = static_cast<UINT>(fbxIt->indexList.size());
		}
		else
		{
			desc.pIdx = nullptr;
		}
		hr = modelIt->buffer.Create(desc);
		SAFE_DELETE_ARRAY(desc.pIdx);
		if (FAILED(hr)) { break; }

		// 頂点ブレンド情報構築
		if (modelIt->pSkinList)
		{
			modelIt->pSkinList->resize(fbxIt->skinInfo.boneList.size());
			for(UINT i = 0; i < fbxIt->skinInfo.boneList.size(); ++ i)
			{
				(*modelIt->pSkinList)[i].bone = fbxIt->skinInfo.boneList[i].name;
			}
		}


		++fbxIt;
		++modelIt;
	}

	ggfbx::Cleanup(&mesh, nullptr, nullptr, nullptr);
	return hr;
}

void FBXModel::Draw(DrawSettingCallBack CallBackFunc)
{
	// モデルの描画
	for(UINT i = 0; i < m_meshList.size(); ++ i)
	{
		if (CallBackFunc != nullptr)
		{
			CallBackFunc(i);
		}
		m_meshList[i].buffer.Draw();
	}
}

