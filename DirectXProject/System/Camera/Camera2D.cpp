#include <System/Camera/Camera2D.h>

Camera2D::Camera2D()
{
}
Camera2D::~Camera2D()
{
}

void Camera2D::Update()
{
}

DirectX::XMMATRIX Camera2D::CalcProjection()
{
	return DirectX::XMMatrixOrthographicOffCenterLH(
		0.0f, m_screenSize.x, m_screenSize.y, 0.0f, m_nearClip, m_farClip
	);
}