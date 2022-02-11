#include <App/TangentModel.h>
#include <System/Debug/Geometory.h>
#include <System/Macro.h>
#ifdef _DEBUG
#include <System/Input.h>
#include <System/Collision.h>
#include <System/Camera/CameraManager.hpp>
#endif

TangentModel::TangentModel()
	: m_pBuffer(nullptr), m_pVtx(nullptr)
#ifdef _DEBUG
	, m_dbgPosOffset(0), m_dbgTangentOffset(0)
#endif
{
}
TangentModel::~TangentModel()
{
	SAFE_DELETE_ARRAY(m_pVtx);
	SAFE_DELETE(m_pBuffer);
}
HRESULT TangentModel::Make(MakeDesc desc)
{
	UINT tangentStride = desc.bufDesc.vtxSize + sizeof(DirectX::XMFLOAT3);
	m_pVtx = new char[tangentStride * desc.bufDesc.vtxCount];

	char* pDst = m_pVtx;
	char* pDstRoot = pDst;
	const char* pSrc = reinterpret_cast<const char*>(desc.bufDesc.pVtx);

	//--- 接ベクトル以外の頂点データをコピー
	for (UINT i = 0; i < desc.bufDesc.vtxCount; ++i)
	{
		memcpy(pDst, pSrc, desc.bufDesc.vtxSize);
		pDst += tangentStride;
		pSrc += desc.bufDesc.vtxSize;
	}
	pDst = pDstRoot;

	//--- 接ベクトルを計算
	bool useIndex = desc.bufDesc.pIdx;
	std::vector<UINT> duplicateVtxCount(desc.bufDesc.vtxCount); // 重複頂点数
	// 面数の計算
	UINT faceNum;
	if (FAILED(CalcFaceNum(&faceNum, desc.bufDesc)))
	{
		SAFE_DELETE_ARRAY(m_pVtx);
		return E_FAIL;
	}
	
	// 面ごとに接ベクトルを計算
	for (UINT i = 0; i < faceNum; ++i)
	{
		// インデックスの割り出し
		UINT index[3];
		if (FAILED(CalcIndex(index, desc.bufDesc, i)))
		{
			SAFE_DELETE_ARRAY(m_pVtx);
			return E_FAIL;
		}
		
		// 面を構成するデータを吸出し
		DirectX::XMFLOAT3 *pTangents[] =
		{
			reinterpret_cast<DirectX::XMFLOAT3*>(pDst + tangentStride * index[0] + desc.bufDesc.vtxSize),
			reinterpret_cast<DirectX::XMFLOAT3*>(pDst + tangentStride * index[1] + desc.bufDesc.vtxSize),
			reinterpret_cast<DirectX::XMFLOAT3*>(pDst + tangentStride * index[2] + desc.bufDesc.vtxSize),
		};
		DirectX::XMFLOAT3 pos[] =
		{
			*reinterpret_cast<DirectX::XMFLOAT3*>(pDst + tangentStride * index[0] + desc.posOffset),
			*reinterpret_cast<DirectX::XMFLOAT3*>(pDst + tangentStride * index[1] + desc.posOffset),
			*reinterpret_cast<DirectX::XMFLOAT3*>(pDst + tangentStride * index[2] + desc.posOffset),
		};
		DirectX::XMFLOAT2 uv[] =
		{
			*reinterpret_cast<DirectX::XMFLOAT2*>(pDst + tangentStride * index[0] + desc.uvOffset),
			*reinterpret_cast<DirectX::XMFLOAT2*>(pDst + tangentStride * index[1] + desc.uvOffset),
			*reinterpret_cast<DirectX::XMFLOAT2*>(pDst + tangentStride * index[2] + desc.uvOffset),
		};

		// 接ベクトルを計算
		DirectX::XMFLOAT3 T = CalcTangent(pos[0], uv[0], pos[1], uv[1], pos[2], uv[2]);
		*pTangents[0] = T;
		*pTangents[1] = T;
		*pTangents[2] = T;
		duplicateVtxCount[index[0]] ++;
		duplicateVtxCount[index[1]] ++;
		duplicateVtxCount[index[2]] ++;
	}
	// 重複数で接ベクトルを平均化
	for (UINT i = 0; i < desc.bufDesc.vtxCount; ++i)
	{
		DirectX::XMFLOAT3* pTangent =
			reinterpret_cast<DirectX::XMFLOAT3*>(pDst + tangentStride * i + desc.bufDesc.vtxSize);
		DirectX::XMVECTOR tangentVec = DirectX::XMLoadFloat3(pTangent);
		tangentVec = DirectX::XMVectorScale(tangentVec, 1.0f / duplicateVtxCount[i]);
		tangentVec = DirectX::XMVector3Normalize(tangentVec);
		DirectX::XMStoreFloat3(pTangent, tangentVec);
	}

	// 全部できたらバッファ作成
	DXBuffer::Desc bufDesc = desc.bufDesc;
	bufDesc.pVtx = m_pVtx;
	bufDesc.vtxSize = tangentStride;

	m_pBuffer = new DXBuffer;
	HRESULT hr = m_pBuffer->Create(bufDesc);
	if (FAILED(hr))
	{
		SAFE_DELETE_ARRAY(m_pVtx);
		SAFE_DELETE(m_pBuffer);
		return hr;
	}

#ifdef _DEBUG
	m_dbgDesc = bufDesc;
	m_dbgPosOffset = desc.posOffset;
	m_dbgUVOffset = desc.uvOffset;
	m_dbgTangentOffset = desc.bufDesc.vtxSize;
#endif

	return hr;
}

void TangentModel::Draw()
{
	if (m_pBuffer)
	{
		m_pBuffer->Draw();
	}

#if 1
#ifdef _DEBUG
	// 接ベクトルデバッグ表示
	UINT faceNum;
	if (FAILED(CalcFaceNum(&faceNum, m_dbgDesc)))
	{
		return;
	}

	Camera* pCamera = CAMERA->Get();
	POINT mousePos = GetMousePos();
	DirectX::XMFLOAT3 nearPos = pCamera->CalcWorldPos(DirectX::XMFLOAT3(mousePos.x, mousePos.y, 0.0f));
	DirectX::XMFLOAT3 farPos = pCamera->CalcWorldPos(DirectX::XMFLOAT3(mousePos.x, mousePos.y, 1.0f));
	//CollisionLine line = { nearPos, farPos };


	char* pDst = m_pVtx;
	CollisionTriangle triangle;
	for (UINT i = 0; i < faceNum; ++i)
	{
		// インデックス
		UINT index[3];
		if (FAILED(CalcIndex(index, m_dbgDesc, i)))
		{
			return;
		}
		// uv
		DirectX::XMFLOAT2 uv[] =
		{
			*reinterpret_cast<DirectX::XMFLOAT2*>(pDst + m_dbgDesc.vtxSize * index[0] + m_dbgUVOffset),
			*reinterpret_cast<DirectX::XMFLOAT2*>(pDst + m_dbgDesc.vtxSize * index[1] + m_dbgUVOffset),
			*reinterpret_cast<DirectX::XMFLOAT2*>(pDst + m_dbgDesc.vtxSize * index[2] + m_dbgUVOffset),
		};
		// 頂点
		triangle.pos0 = *reinterpret_cast<DirectX::XMFLOAT3*>(pDst + m_dbgDesc.vtxSize * index[0] + m_dbgPosOffset);
		triangle.pos1 = *reinterpret_cast<DirectX::XMFLOAT3*>(pDst + m_dbgDesc.vtxSize * index[1] + m_dbgPosOffset);
		triangle.pos2 = *reinterpret_cast<DirectX::XMFLOAT3*>(pDst + m_dbgDesc.vtxSize * index[2] + m_dbgPosOffset);
		DirectX::XMVECTOR vPos[] =
		{
			DirectX::XMLoadFloat3(&triangle.pos0),
			DirectX::XMLoadFloat3(&triangle.pos1),
			DirectX::XMLoadFloat3(&triangle.pos2),
		};
		DirectX::XMVECTOR edge[] =
		{
			DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(vPos[1], vPos[0])),
			DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(vPos[2], vPos[0])),
		};
		DirectX::XMStoreFloat3(&triangle.normal, DirectX::XMVector3Cross(edge[0], edge[1]));

		/*
		DirectX::XMFLOAT3 hit;
		if (Collision(line, triangle, &hit))
		{
			DirectX::XMFLOAT4 r(1, 0, 0, 1);
			DirectX::XMFLOAT4 g(0, 1, 0, 1);
			DirectX::XMFLOAT4 b(0, 0, 1, 1);
			AddLine(triangle.pos0, triangle.pos1, r);
			AddLine(triangle.pos0, triangle.pos2, g);
			DirectX::XMFLOAT3 uv1(hit.x + (uv[1].x - uv[0].x), hit.y + (uv[1].y - uv[0].y), hit.z);
			DirectX::XMFLOAT3 uv2(hit.x + (uv[2].x - uv[0].x), hit.y + (uv[2].y - uv[0].y), hit.z);
			AddLine(hit, uv1, r);
			AddLine(hit, uv2, g);
			AddLine(DirectX::XMFLOAT3(), hit, b);
		}*/
	}
	//DrawLines();


#endif
#endif
}

HRESULT TangentModel::CalcFaceNum(UINT* pOut, DXBuffer::Desc& desc)
{
	bool useIndex = desc.pIdx;
	switch (desc.topology)
	{
	case D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST:
		*pOut = (useIndex ? desc.idxCount : desc.vtxCount) / 3;
		break;
	case D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP:
		*pOut = (useIndex ? desc.idxCount : desc.vtxCount) - 2;
		break;
	default:
		SAFE_DELETE_ARRAY(m_pVtx);
		return E_FAIL; // 未対応
	}
	return S_OK;
}
HRESULT TangentModel::CalcIndex(UINT *pOut, DXBuffer::Desc &desc, UINT face)
{
	bool useIndex = desc.pIdx;
	switch (desc.topology)
	{
	case D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST:
		if (useIndex)
		{
			for (UINT i = 0; i < 3; ++i)
			{
				switch (desc.idxSize)
				{
					case 1: pOut[i] = reinterpret_cast<const BYTE*>(desc.pIdx)[face * 3 + i]; break;
					case 2: pOut[i] = reinterpret_cast<const WORD*>(desc.pIdx)[face * 3 + i]; break;
					case 4: pOut[i] = reinterpret_cast<const DWORD*>(desc.pIdx)[face * 3 + i]; break;
				}
			}
		}
		else
		{
			for (UINT i = 0; i < 3; ++i)
				pOut[i] = face * 3 + i;
		}
		break;
	case D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP:
		if (useIndex)
		{
			for (UINT i = 0; i < 3; ++i)
			{
				switch (desc.idxSize)
				{
				case 1: pOut[i] = reinterpret_cast<const BYTE*>(desc.pIdx)[face + i]; break;
				case 2: pOut[i] = reinterpret_cast<const WORD*>(desc.pIdx)[face + i]; break;
				case 4: pOut[i] = reinterpret_cast<const DWORD*>(desc.pIdx)[face + i]; break;
				}
			}
		}
		else
		{
			for (UINT i = 0; i < 3; ++i)
				pOut[i] = face + i;
		}
		break;
	default:
		return E_FAIL;
	}

	return S_OK;
}

DirectX::XMFLOAT3 TangentModel::CalcTangent(
	DirectX::XMFLOAT3 pos0, DirectX::XMFLOAT2 uv0,
	DirectX::XMFLOAT3 pos1, DirectX::XMFLOAT2 uv1,
	DirectX::XMFLOAT3 pos2, DirectX::XMFLOAT2 uv2)
{
	DirectX::XMFLOAT3 v0(pos1.x - pos0.x, pos1.y - pos0.y, pos1.z - pos0.z);
	DirectX::XMFLOAT3 v1(pos2.x - pos0.x, pos2.y - pos0.y, pos2.z - pos0.z);
	DirectX::XMFLOAT2 t0(uv1.x - uv0.x, uv1.y - uv0.y);
	DirectX::XMFLOAT2 t1(uv2.x - uv0.x, uv2.y - uv0.y);
	float t = t0.x * t1.y - t1.x * t0.y;
	DirectX::XMFLOAT3 tangent(
		(t1.y * v0.x - t0.y * v1.x) / t,
		(t1.y * v0.y - t0.y * v1.y) / t,
		(t1.y * v0.z - t0.y * v1.z) / t
	);
	DirectX::XMStoreFloat3(&tangent, DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&tangent)));

	return tangent;
}