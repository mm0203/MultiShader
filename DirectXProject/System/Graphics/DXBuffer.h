#ifndef __DX_BUFFER_H__
#define __DX_BUFFER_H__

#include "DirectX.h"

class DXBuffer
{
public:
	struct Desc
	{
		const void* pVtx;
		UINT vtxSize;
		UINT vtxCount;
		bool isWrite;
		const void* pIdx;
		UINT idxSize;
		UINT idxCount;
		D3D11_PRIMITIVE_TOPOLOGY topology;
	};
public:
	DXBuffer();
	~DXBuffer();

	HRESULT Create(const Desc& desc);

	void Draw(int count = 0);
	HRESULT Write(void* pVtx);

private:
	HRESULT CreateVertexBuffer(const void* pIdx, UINT size, UINT count, bool isWrite);
	HRESULT CreateIndexBuffer(const void* pVtx, UINT size, UINT count);


private:
	ID3D11Buffer* m_pVtxBuffer;
	UINT m_vtxSize;
	UINT m_vtxCount;

	ID3D11Buffer* m_pIdxBuffer;
	UINT m_idxSize;
	UINT m_idxCount;

	D3D11_PRIMITIVE_TOPOLOGY m_topology;
};



#endif // __DRAW_BUFFER_H__