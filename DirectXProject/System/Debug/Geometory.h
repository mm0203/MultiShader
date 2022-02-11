#ifndef __GEOMETORY_H__
#define __GEOMETORY_H__

#include <System/Debug/Debug.h>
#include <System/Singleton.hpp>
#include <System/Graphics/DXBuffer.h>
#include <System/Graphics/Shader.h>
#include <DirectXMath.h>
#include <vector>

#define GEOMETORY (Geometory::GetInstance())

class Geometory : public Singleton<Geometory>
{
	SINGLETON_CONSTRACTOR(Geometory);
private:
	static const DirectX::XMFLOAT4 SmokeColor;
public:
	void Init() final;
	void Uninit() final;

	// 直線
	void DrawArrow(const DirectX::XMMATRIX& mat, const DirectX::XMFLOAT4& color = SmokeColor) DMODIFIER;
	void AddLine(const DirectX::XMFLOAT3& start, const DirectX::XMFLOAT3& end, const DirectX::XMFLOAT4& color = SmokeColor) DMODIFIER;
	void DrawLines() DMODIFIER;
	// 立方
	void DrawSquare(const DirectX::XMMATRIX& mat, const DirectX::XMFLOAT4& color = SmokeColor) DMODIFIER;
	void DrawPolygon(const DirectX::XMMATRIX& mat, const DirectX::XMFLOAT4& color = SmokeColor) DMODIFIER;
	void DrawCube() DMODIFIER;

	// 特殊
	void DrawPoint(const DirectX::XMFLOAT3& pos) DMODIFIER;
	void DrawMatrix(const DirectX::XMMATRIX& mat) DMODIFIER;


	void DrawCircle() DMODIFIER;
	void DrawSphere() DMODIFIER;

	void DrawCylinder() DMODIFIER;

	/// @note 描画コンポーネント作って消したい
	void SetPos(const DirectX::XMFLOAT3& pos) DMODIFIER;
	void SetWorld(const DirectX::XMFLOAT4X4& wrold) DMODIFIER;
	void SetViewProj(const DirectX::XMFLOAT4X4& view, const DirectX::XMFLOAT4X4& proj) DMODIFIER;
	void SetColor(const DirectX::XMFLOAT4& color);

private:
	void MakeShader();
	void MakePoint();
	void MakeArrow();
	void MakeLine();
	void MakeSquare();

	void MakeCube();
	void MakeCircle();
	void MakeSphere();

private:
	struct ColorObjectVertex
	{
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT4 color;
	};
	using Geometories = std::vector<DXBuffer*>;
	using Shaders = std::vector<Shader*>;

	static const int LineNum = 500;

private:
#if USE_DEBUG
	Geometories m_pGeometories;
	Shaders m_pShaders;
	ConstantBuffer* m_pWorld;
	ConstantBuffer* m_pViewProj;
	ConstantBuffer* m_pColor;

	ColorObjectVertex* m_pLineVertex;
	UINT m_drawLineCount;
#endif // USE_DEBUG
};

#endif // __GEOMETORY_H__