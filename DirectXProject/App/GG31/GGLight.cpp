#include <App/GG31/GGLight.h>
#include <App/GG31/GGLevelMain.h>
#include <System/Scene/SceneManager.hpp>

Light::Light(Kind kind)
	: m_kind(kind)
	, m_pos(0,0,0)
	, m_color(1,1,1,1)
	, m_direction(0,-1,0)
	, m_attenuation(1,1,1)
	, m_range(1.0f)
	, m_angle(DirectX::XM_PIDIV4)

{
}
Light::~Light()
{
}

void Light::SetPos(DirectX::XMFLOAT3& pos)
{
	m_pos = pos;
}
void Light::SetPos(DirectX::XMFLOAT3&& pos)
{
	m_pos = pos;
}
void Light::SetColor(DirectX::XMFLOAT4& color)
{
	m_color = color;
}
void Light::SetColor(DirectX::XMFLOAT4&& color)
{
	m_color = color;
}
void Light::SetDirection(DirectX::XMFLOAT3& dir)
{
	m_direction = dir;
}
void Light::SetDirection(DirectX::XMFLOAT3&& dir)
{
	m_direction = dir;
}
void Light::SetAttenuation(DirectX::XMFLOAT3& attenuation)
{
	m_attenuation = attenuation;
}
void Light::SetAttenuation(DirectX::XMFLOAT3&& attenuation)
{
	m_attenuation = attenuation;
}
void Light::SetRange(float range)
{
	m_range = range;
}
void Light::SetAngle(float angle)
{
	m_angle = angle;
}

const DirectX::XMFLOAT3& Light::GetPos()
{
	return m_pos;
}
const DirectX::XMFLOAT3& Light::GetDirection()
{
	return m_direction;
}
const DirectX::XMFLOAT4& Light::GetColor()
{
	return m_color;
}
float Light::GetRange()
{
	return m_range;
}
float Light::GetAngle()
{
	return m_angle;
}