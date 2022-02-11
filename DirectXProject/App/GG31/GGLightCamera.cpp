#include <App/GG31/GGLightCamera.h>
#include <System/Debug/Geometory.h>

GGLightCamera::GGLightCamera()
{
	m_pos = DirectX::XMFLOAT3(-3.0f, 3.0f, -3.0f);
	m_screenSize = DirectX::XMFLOAT2(10.f, 10.f);
	SetDirection(DirectX::XMFLOAT3(1.0f, -1.0f, 1.0f));
	m_farClip = 10.0f;
	Enable(true);
	Execute();
}
GGLightCamera::~GGLightCamera()
{
}

void GGLightCamera::Update()
{
}
void GGLightCamera::Draw()
{
	GEOMETORY->DrawPoint(m_pos);
}
DirectX::XMMATRIX GGLightCamera::CalcProjection()
{
	return DirectX::XMMatrixOrthographicLH(m_screenSize.x, m_screenSize.y, m_nearClip, m_farClip);
}

void GGLightCamera::SetDirection(DirectX::XMFLOAT3 dir)
{
	DirectX::XMStoreFloat3(&dir, DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&dir)));
	m_look.x = m_pos.x + dir.x;
	m_look.y = m_pos.y + dir.y;
	m_look.z = m_pos.z + dir.z;
}