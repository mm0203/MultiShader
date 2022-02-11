#include <System/Debug/Geometory.h>
#include <System/Graphics/Shader.h>
#include <System/Macro.h>

#if USE_DEBUG


const DirectX::XMFLOAT4 Geometory::SmokeColor(0.9f, 0.9f, 0.9f, 1.0f);

enum GeometoryPrimitiveKind
{
	GEOMETORY_PRIMITIVE_POINT,
	GEOMETORY_PRIMITIVE_ARROW,
	GEOMETORY_PRIMITIVE_MATRIX,
	GEOMETORY_PRIMITIVE_LINE,
	GEOMETORY_PRIMITIVE_SQUARE,
	GEOMETORY_PRIMITIVE_POLYGON,
	MAX_GEOMETORY_PRIMITIVE
};
enum GeometoryShaderKind
{
	GEOMETORY_VS,
	GEOMETORY_PS_OBJECT_COLOR,
	GEOMETORY_PS_PARAMETER_COLOR,
	MAX_GEOMETORY_SHADER
};
const DirectX::XMFLOAT4 GeometoryWhiteColor(0.1f, 0.1f, 0.1f, 1.0f);
const DirectX::XMFLOAT4 GeometorySmokeColor(0.9f, 0.9f, 0.9f, 1.0f);
const DirectX::XMFLOAT4 GeometoryAxisColor[] =
{
	{0.9f, 0.0f, 0.0f, 1.0f}, {0.0f, 0.9f, 0.0f, 1.0f},{0.0f, 0.0f, 0.9f, 1.0f}
};
const char GeometoryVS[] = R"EOT(
struct VS_IN
{
	float3 pos : POSITION0;
	float4 color : COLOR0;
};
struct VS_OUT
{
	float4 pos : SV_POSITION;
	float4 color : COLOR0;
};
cbuffer Matrix : register(b0)
{
	float4x4 world;
};
cbuffer ViewProj : register(b1)
{
	float4x4 view;
	float4x4 proj;
};
VS_OUT main(VS_IN vin)
{
	VS_OUT vout;
	vout.pos = float4(vin.pos, 1);
	vout.pos = mul(vout.pos, world);
	vout.pos = mul(vout.pos, view);
	vout.pos = mul(vout.pos, proj);
	vout.color = vin.color;
	return vout;
}
)EOT";
const char GeometoryObjectColorPS[] = R"EOT(
float4 main(float4 pos : SV_POSITION, float4 color : COLOR0) : SV_TARGET
{
	return color;
}
)EOT";
const char GeometoryParameterColorPS[] = R"EOT(
cbuffer Material : register(b0)
{
	float4 diffuse;
};
float4 main(float4 pos : SV_POSITION) : SV_TARGET
{
	return diffuse;
}
)EOT";

void Geometory::Init()
{
	m_pShaders.resize(MAX_GEOMETORY_SHADER);
	m_pGeometories.resize(MAX_GEOMETORY_PRIMITIVE);
	MakeShader();
	MakePoint();
	MakeArrow();
	MakeLine();
	MakeSquare();
}
void Geometory::Uninit()
{
	Geometories::iterator gIt = m_pGeometories.begin();
	while (gIt != m_pGeometories.end())
	{
		SAFE_DELETE(*gIt);
		gIt++;
	}
	Shaders::iterator sIt = m_pShaders.begin();
	while(sIt != m_pShaders.end())
	{
		SAFE_DELETE(*sIt);
		sIt++;
	}
	SAFE_DELETE(m_pWorld);
	SAFE_DELETE(m_pViewProj);
	SAFE_DELETE(m_pColor);
	SAFE_DELETE_ARRAY(m_pLineVertex);
}

void Geometory::DrawPoint(const DirectX::XMFLOAT3& pos)
{
	SetPos(pos);
	m_pWorld->BindVS(0);
	m_pViewProj->BindVS(1);
	m_pColor->BindPS(0);
	m_pShaders[GEOMETORY_VS]->Bind();
	m_pShaders[GEOMETORY_PS_OBJECT_COLOR]->Bind();
	m_pGeometories[GEOMETORY_PRIMITIVE_POINT]->Draw();
}
void Geometory::DrawArrow(const DirectX::XMMATRIX& mat, const DirectX::XMFLOAT4& color)
{
	// 設定
	DirectX::XMFLOAT4X4 tmp;
	DirectX::XMStoreFloat4x4(&tmp, DirectX::XMMatrixTranspose(mat));
	m_pWorld->Write(&tmp);
	SetColor(color);
	// 描画
	m_pWorld->BindVS(0);
	m_pViewProj->BindVS(1);
	m_pColor->BindPS(0);
	m_pShaders[GEOMETORY_VS]->Bind();
	m_pShaders[GEOMETORY_PS_PARAMETER_COLOR]->Bind();
	m_pGeometories[GEOMETORY_PRIMITIVE_ARROW]->Draw();
}
void Geometory::DrawMatrix(const DirectX::XMMATRIX& mat)
{
	DirectX::XMFLOAT4X4 rotX, rotY, rotZ;
	DirectX::XMStoreFloat4x4(&rotX, mat);
	DirectX::XMStoreFloat4x4(&rotY, DirectX::XMMatrixRotationZ(DirectX::XM_PIDIV2) * mat);
	DirectX::XMStoreFloat4x4(&rotZ, DirectX::XMMatrixRotationY(-DirectX::XM_PIDIV2) * mat);

	m_pWorld->BindVS(0);
	m_pViewProj->BindVS(1);
	m_pColor->BindPS(0);
	m_pShaders[GEOMETORY_VS]->Bind();
	m_pShaders[GEOMETORY_PS_PARAMETER_COLOR]->Bind();
	SetWorld(rotX);
	SetColor(GeometoryAxisColor[0]);
	m_pGeometories[GEOMETORY_PRIMITIVE_ARROW]->Draw();
	SetWorld(rotY);
	SetColor(GeometoryAxisColor[1]);
	m_pGeometories[GEOMETORY_PRIMITIVE_ARROW]->Draw();
	SetWorld(rotZ);
	SetColor(GeometoryAxisColor[2]);
	m_pGeometories[GEOMETORY_PRIMITIVE_ARROW]->Draw();

}
void Geometory::AddLine(const DirectX::XMFLOAT3& start, const DirectX::XMFLOAT3& end, const DirectX::XMFLOAT4 &color)
{
	if (m_drawLineCount < LineNum)
	{
		int idx = m_drawLineCount * 2;
		m_pLineVertex[idx + 0].pos = start;
		m_pLineVertex[idx + 0].color = color;
		m_pLineVertex[idx + 1].pos = end;
		m_pLineVertex[idx + 1].color = color;
		++m_drawLineCount;
	}
}
void Geometory::DrawLines()
{
	SetPos(DirectX::XMFLOAT3(0, 0, 0));
	m_pWorld->BindVS(0);
	m_pViewProj->BindVS(1);
	m_pColor->BindPS(0);
	m_pShaders[GEOMETORY_VS]->Bind();
	m_pShaders[GEOMETORY_PS_OBJECT_COLOR]->Bind();
	m_pGeometories[GEOMETORY_PRIMITIVE_LINE]->Write(m_pLineVertex);
	m_pGeometories[GEOMETORY_PRIMITIVE_LINE]->Draw();
	m_drawLineCount = 0;
}
void Geometory::DrawSquare(const DirectX::XMMATRIX& mat, const DirectX::XMFLOAT4& color)
{
	// 設定
	DirectX::XMFLOAT4X4 tmp;
	DirectX::XMStoreFloat4x4(&tmp, DirectX::XMMatrixTranspose(mat));
	m_pWorld->Write(&tmp);
	SetColor(color);
	// 描画
	m_pWorld->BindVS(0);
	m_pViewProj->BindVS(1);
	m_pColor->BindPS(0);
	m_pShaders[GEOMETORY_VS]->Bind();
	m_pShaders[GEOMETORY_PS_PARAMETER_COLOR]->Bind();
	m_pGeometories[GEOMETORY_PRIMITIVE_SQUARE]->Draw();
}
void Geometory::DrawPolygon(const DirectX::XMMATRIX& mat, const DirectX::XMFLOAT4& color)
{
	// 設定
	DirectX::XMFLOAT4X4 tmp;
	DirectX::XMStoreFloat4x4(&tmp, DirectX::XMMatrixTranspose(mat));
	m_pWorld->Write(&tmp);
	SetColor(color);
	// 描画
	m_pWorld->BindVS(0);
	m_pViewProj->BindVS(1);
	m_pColor->BindPS(0);
	m_pShaders[GEOMETORY_VS]->Bind();
	m_pShaders[GEOMETORY_PS_PARAMETER_COLOR]->Bind();
	m_pGeometories[GEOMETORY_PRIMITIVE_POLYGON]->Draw();
}
void Geometory::DrawCube()
{
}
void Geometory::DrawCircle()
{
}
void Geometory::DrawSphere()
{
}
void Geometory::DrawCylinder()
{
}
void Geometory::SetPos(const DirectX::XMFLOAT3& pos)
{
	DirectX::XMFLOAT4X4 mat;
	DirectX::XMStoreFloat4x4(&mat, DirectX::XMMatrixTranspose(DirectX::XMMatrixTranslation(pos.x, pos.y, pos.z)));
	m_pWorld->Write(&mat);
}
void Geometory::SetWorld(const DirectX::XMFLOAT4X4& world)
{
	DirectX::XMFLOAT4X4 mat;
	DirectX::XMStoreFloat4x4(&mat, DirectX::XMMatrixTranspose(DirectX::XMLoadFloat4x4(&world)));
	m_pWorld->Write(&mat);
}
void Geometory::SetViewProj(const DirectX::XMFLOAT4X4& view, const DirectX::XMFLOAT4X4& proj)
{
	DirectX::XMFLOAT4X4 mat[2];
	DirectX::XMStoreFloat4x4(&mat[0], DirectX::XMMatrixTranspose(DirectX::XMLoadFloat4x4(&view)));
	DirectX::XMStoreFloat4x4(&mat[1], DirectX::XMMatrixTranspose(DirectX::XMLoadFloat4x4(&proj)));
	m_pViewProj->Write(mat);
}
void Geometory::SetColor(const DirectX::XMFLOAT4& color)
{
	m_pColor->Write(&color);
}

void Geometory::MakeShader()
{
	// シェーダ
	m_pShaders[GEOMETORY_VS] = new VertexShader;
	if (FAILED(m_pShaders[GEOMETORY_VS]->Compile(GeometoryVS))) { return; }
	m_pShaders[GEOMETORY_PS_OBJECT_COLOR] = new PixelShader;
	if (FAILED(m_pShaders[GEOMETORY_PS_OBJECT_COLOR]->Compile(GeometoryObjectColorPS))) { return; }
	m_pShaders[GEOMETORY_PS_PARAMETER_COLOR] = new PixelShader;
	if (FAILED(m_pShaders[GEOMETORY_PS_PARAMETER_COLOR]->Compile(GeometoryParameterColorPS))) { return; }

	// 定数バッファ
	m_pWorld = new ConstantBuffer;
	if (FAILED(m_pWorld->Create(sizeof(DirectX::XMFLOAT4X4)))) { return; }
	m_pViewProj = new ConstantBuffer;
	if (FAILED(m_pViewProj->Create(sizeof(DirectX::XMFLOAT4X4) * 2))) { return; }
	m_pColor = new ConstantBuffer;
	if (FAILED(m_pColor->Create(sizeof(DirectX::XMFLOAT4)))) { return; }

	SetColor(DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));
}
void Geometory::MakePoint()
{
	// 定義
	const int PointCenterSplitH		= 8;
	const float PointCenterDiameter	= CMETER(2.0f);
	const float PointAxisBold		= CMETER(0.3f);
	const float PointAxisLength		= CMETER(15.0f);
	// 計算済み定義
	const int PointCenterSplitV = (PointCenterSplitH - 2) / 2 + 2;
	const int PointCenterVertexCount = PointCenterSplitH * PointCenterSplitV;
	const int PointAxisVertexCount = 24;
	const int PointVertexCount = PointCenterVertexCount + PointAxisVertexCount;
	const int PointCenterFaceNum = PointCenterSplitH * (PointCenterSplitV - 1) * 2;
	const int PointCenterIndexCount = PointCenterFaceNum * 3;
	const int PointAxisIndexCount = 24 * 3;
	const int PointIndexCount = PointCenterIndexCount + PointAxisIndexCount;
	const float PointCenterRadian = DirectX::XM_2PI / PointCenterSplitH;
	const float PointCenterRadius = PointCenterDiameter * 0.5f;
	const float PointAxisRadius = PointAxisBold * 0.5f;
	const float PointAxisHalf = PointAxisLength * 0.5f;
	
	// データ
	ColorObjectVertex* pVtx = new ColorObjectVertex[PointVertexCount];
	WORD *pIdx = new WORD[PointIndexCount];

	// 球
	for (int v = 0; v < PointCenterSplitV; ++v)
	{
		float radV = PointCenterRadian * v;
		for (int h = 0; h < PointCenterSplitH; ++h)
		{
			int idx = v * PointCenterSplitH + h;
			float radH = PointCenterRadian * h;
			pVtx[idx].pos.x = sinf(radV) * -sinf(radH) * PointCenterRadius;
			pVtx[idx].pos.y = cosf(radV) * PointCenterRadius;
			pVtx[idx].pos.z = sinf(radV) * -cosf(radH) * PointCenterRadius;
			pVtx[idx].color = GeometorySmokeColor;
		}
	}
	// 球インデックス
	for (int i = 0; i < PointCenterFaceNum / 2; i ++)
	{
		int idx = i * 6;
		pIdx[idx + 0] = i;
		pIdx[idx + 1] = PointCenterSplitH * (i / PointCenterSplitH) + ((i + 1) % PointCenterSplitH);
		pIdx[idx + 2] = i + PointCenterSplitH;
		pIdx[idx + 3] = pIdx[idx + 1];
		pIdx[idx + 4] = pIdx[idx + 1] + PointCenterSplitH;
		pIdx[idx + 5] = pIdx[idx + 2];
	}
	
	// 軸芯
	int AxisParameter[PointAxisVertexCount][3] =
	{
		{ 2, 1,-1}, { 2, 1, 1}, { 2,-1, 1}, { 2,-1,-1}, // X軸+
		{-2, 1,-1}, {-2, 1, 1}, {-2,-1, 1}, {-2,-1,-1}, // X軸-
		{ 1, 2,-1}, { 1, 2, 1}, {-1, 2, 1}, {-1, 2,-1}, // Y軸+
		{ 1,-2,-1}, { 1,-2, 1}, {-1,-2, 1}, {-1,-2,-1}, // Y軸-
		{ 1, 1, 2}, {-1, 1, 2}, {-1,-1, 2}, { 1,-1, 2}, // Z軸+
		{ 1, 1,-2}, {-1, 1,-2}, {-1,-1,-2}, { 1,-1,-2}, // Z軸-
	};
	auto GetAxisSize = [&AxisParameter, &PointAxisHalf, &PointAxisRadius](int i, int axis)
	{
		return (abs(AxisParameter[i][axis]) == 2 ? PointAxisHalf : PointAxisRadius)* (AxisParameter[i][axis] > 0 ? 1.0f : -1.0f);
	};
	for (int i = 0; i < PointAxisVertexCount; ++i)
	{
		int idx = PointCenterVertexCount + i;
		pVtx[idx].pos.x = GetAxisSize(i, 0);
		pVtx[idx].pos.y = GetAxisSize(i, 1);
		pVtx[idx].pos.z = GetAxisSize(i, 2);
		pVtx[idx].color = GeometoryAxisColor[i / 8];
	}
	// 軸芯インデックス
	for (int j = 0; j < 3; ++j)
	{
		for (int i = 0; i < 4; ++i)
		{
			int idx = PointCenterIndexCount + (j * 4 + i) * 6;
			pIdx[idx + 0] = PointCenterVertexCount + j * 8 + i;
			pIdx[idx + 1] = pIdx[idx + 0] + 4;
			pIdx[idx + 2] = PointCenterVertexCount + j * 8 + (pIdx[idx + 0] + 1) % 4;
			pIdx[idx + 3] = pIdx[idx + 1];
			pIdx[idx + 4] = pIdx[idx + 2] + 4;
			pIdx[idx + 5] = pIdx[idx + 2];
		}
	}

	// バッファー作成
	DXBuffer::Desc desc = {};
	desc.pVtx = pVtx;
	desc.vtxSize = sizeof(ColorObjectVertex);
	desc.vtxCount = PointVertexCount;
	desc.topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	desc.pIdx = pIdx;
	desc.idxSize = sizeof(WORD);
	desc.idxCount = PointIndexCount;
	m_pGeometories[GEOMETORY_PRIMITIVE_POINT] = new DXBuffer;
	m_pGeometories[GEOMETORY_PRIMITIVE_POINT]->Create(desc);
	SAFE_DELETE_ARRAY(pVtx);
	SAFE_DELETE_ARRAY(pIdx);
}
void Geometory::MakeArrow()
{
	// 定義
	const int	ArrowCircleSplit	= 8;
	const float ArrowConeScale		= 0.2f;
	const float ArrowLength			= METER(1.0f);
	const float ArrowConeDiameter	= CMETER(10.0f);
	const float ArrowAxisDiameter	= CMETER(3.0f);
	// 計算済み定義
	const int ArrowConeVertexCount = ArrowCircleSplit + 1;
	const int ArrowAxisVertexCount = ArrowCircleSplit * 2;
	const int ArrowVertexCount = ArrowConeVertexCount + ArrowAxisVertexCount;
	const int ArrowConeIndexCount = ArrowCircleSplit * 3;
	const int ArrowAxisIndexCount = ArrowCircleSplit * 2 * 3;
	const int ArrowIndexCount = ArrowConeIndexCount + ArrowAxisIndexCount;
	const float ArrowAxisLength = ArrowLength * (1.0f - ArrowConeScale);
	const float ArrowConeRadius = ArrowConeDiameter * 0.5f;
	const float ArrowAxisRadius = ArrowAxisDiameter * 0.5f;
	const float ArrowRadian = DirectX::XM_2PI / ArrowCircleSplit;

	// データ
	DirectX::XMFLOAT3* pVtx = new DirectX::XMFLOAT3[ArrowVertexCount];
	WORD* pIdx = new WORD[ArrowIndexCount];

	// 矢印先端
	pVtx[0] = DirectX::XMFLOAT3(ArrowLength, 0.0f, 0.0f);
	for (int i = 1; i < ArrowConeVertexCount; ++i)
	{
		float rad = (i - 1) * ArrowRadian;
		pVtx[i].x = ArrowAxisLength;
		pVtx[i].y = sinf(rad) * ArrowConeRadius;
		pVtx[i].z = -cosf(rad) * ArrowConeRadius;
	}
	// 矢印インデックス
	for (int i = 0; i <  ArrowCircleSplit; i++)
	{
		int idx = i * 3;
		pIdx[idx + 0] = 0;
		pIdx[idx + 1] = i + 1;
		pIdx[idx + 2] = (i + 1) % ArrowCircleSplit + 1;
	}

	// 軸芯
	for (int i = 0; i < ArrowCircleSplit; ++i)
	{
		float rad = i * ArrowRadian;
		int idx = i * 2 + ArrowConeVertexCount;
		pVtx[idx].x = ArrowAxisLength;
		pVtx[idx].y = sinf(rad) * ArrowAxisRadius;
		pVtx[idx].z = -cosf(rad) * ArrowAxisRadius;
		pVtx[idx + 1] = pVtx[idx];
		pVtx[idx + 1].x = 0.0f;
	}
	// 軸芯インデックス
	for (int i = 0; i < ArrowCircleSplit; ++i)
	{
		int idx = i * 6 + ArrowConeIndexCount;
		pIdx[idx + 0] = ArrowConeVertexCount + i * 2;
		pIdx[idx + 1] = pIdx[idx + 0] + 1;
		pIdx[idx + 2] = ((i + 1) % ArrowCircleSplit) * 2 + ArrowConeVertexCount;
		pIdx[idx + 3] = pIdx[idx + 1];
		pIdx[idx + 4] = pIdx[idx + 2] + 1;
		pIdx[idx + 5] = pIdx[idx + 2];
	}

	// バッファー作成
	DXBuffer::Desc desc = {};
	desc.pVtx = pVtx;
	desc.vtxSize = sizeof(DirectX::XMFLOAT3);
	desc.vtxCount = ArrowVertexCount;
	desc.topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	desc.pIdx = pIdx;
	desc.idxSize = sizeof(WORD);
	desc.idxCount = ArrowIndexCount;
	m_pGeometories[GEOMETORY_PRIMITIVE_ARROW] = new DXBuffer;
	m_pGeometories[GEOMETORY_PRIMITIVE_ARROW]->Create(desc);
	SAFE_DELETE_ARRAY(pVtx);
	SAFE_DELETE_ARRAY(pIdx);
}
void Geometory::MakeLine()
{
	// 計算済み定義
	const int LineVertexCount = LineNum * 2;
	
	// 頂点作成
	m_pLineVertex = new ColorObjectVertex[LineVertexCount];
	ZeroMemory(m_pLineVertex, sizeof(ColorObjectVertex) * LineVertexCount);

	// バッファー作成
	DXBuffer::Desc desc = {};
	desc.pVtx = m_pLineVertex;
	desc.vtxSize = sizeof(ColorObjectVertex);
	desc.vtxCount = LineVertexCount;
	desc.topology = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
	desc.isWrite = true;
	m_pGeometories[GEOMETORY_PRIMITIVE_LINE] = new DXBuffer;
	m_pGeometories[GEOMETORY_PRIMITIVE_LINE]->Create(desc);
	m_drawLineCount = 0;
}
void Geometory::MakeSquare()
{
	// 定義
	const float SquareSize = METER(1.0f);
	// 計算済み定義
	const int SquareVertexCount = 4;
	const int SquareIndexCount = SquareVertexCount * 2;
	const float SquareHalfSize = SquareSize * 0.5f;

	// データ
	DirectX::XMFLOAT3* pVtx = new DirectX::XMFLOAT3[SquareVertexCount];
	BYTE* pIdx = new BYTE[SquareIndexCount];

	// 四隅
	pVtx[0] = DirectX::XMFLOAT3(-SquareHalfSize, SquareHalfSize, 0.0f);
	pVtx[1] = DirectX::XMFLOAT3( SquareHalfSize, SquareHalfSize, 0.0f);
	pVtx[2] = DirectX::XMFLOAT3(-SquareHalfSize,-SquareHalfSize, 0.0f);
	pVtx[3] = DirectX::XMFLOAT3( SquareHalfSize,-SquareHalfSize, 0.0f);
	// インデックス
	pIdx[0] = 0; pIdx[1] = 1;
	pIdx[0] = 1; pIdx[1] = 3;
	pIdx[0] = 3; pIdx[1] = 2;
	pIdx[0] = 2; pIdx[1] = 0;

	// バッファー作成
	DXBuffer::Desc desc = {};
	desc.pVtx = pVtx;
	desc.vtxSize = sizeof(DirectX::XMFLOAT3);
	desc.vtxCount = SquareVertexCount;
	desc.topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
	m_pGeometories[GEOMETORY_PRIMITIVE_POLYGON] = new DXBuffer;
	m_pGeometories[GEOMETORY_PRIMITIVE_POLYGON]->Create(desc);
	desc.pIdx = pIdx;
	desc.idxSize = sizeof(BYTE);
	desc.idxCount = SquareIndexCount;
	desc.topology = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
	m_pGeometories[GEOMETORY_PRIMITIVE_SQUARE] = new DXBuffer;
	m_pGeometories[GEOMETORY_PRIMITIVE_SQUARE]->Create(desc);
	SAFE_DELETE_ARRAY(pVtx);
	SAFE_DELETE_ARRAY(pIdx);
}
void Geometory::MakeCube()
{
}
void Geometory::MakeCircle()
{
}
void Geometory::MakeSphere()
{
}

#endif // USE_DEBUG