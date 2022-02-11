#ifndef __TANGENT_MODEL_H__
#define __TANGENT_MODEL_H__

#include <DirectXMath.h>
#include <System/Graphics/DXBuffer.h>
#include <vector>

class TangentModel
{
public:
	struct MakeDesc
	{
		DXBuffer::Desc bufDesc;
		UINT posOffset;
		UINT uvOffset;
	};
public:
	TangentModel();
	~TangentModel();

	HRESULT Make(MakeDesc desc);
	void Draw();

private:
	HRESULT CalcFaceNum(UINT *pOut, DXBuffer::Desc &desc);
	HRESULT CalcIndex(UINT *pOut, DXBuffer::Desc &desc, UINT face);
	DirectX::XMFLOAT3 CalcTangent(
		DirectX::XMFLOAT3 pos0, DirectX::XMFLOAT2 uv0,
		DirectX::XMFLOAT3 pos1, DirectX::XMFLOAT2 uv1,
		DirectX::XMFLOAT3 pos2, DirectX::XMFLOAT2 uv2);

private:
	DXBuffer* m_pBuffer;
	char* m_pVtx;
#ifdef _DEBUG
	DXBuffer::Desc m_dbgDesc;
	UINT m_dbgPosOffset;
	UINT m_dbgUVOffset;
	UINT m_dbgTangentOffset;
#endif
};

#endif // __TANGENT_MODEL_H__