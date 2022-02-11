#ifndef __GG_LIGHT_H__
#define __GG_LIGHT_H__

#include <DirectXMath.h>

class Light
{
public:
	enum Kind
	{
		DIRECTIONAL_LIGHT,
		POINT_LIGHT,
		SPOT_LIGHT
	};
public:
	Light(Kind kind);
	~Light();

	void SetPos(DirectX::XMFLOAT3& pos);
	void SetPos(DirectX::XMFLOAT3&& pos);
	void SetColor(DirectX::XMFLOAT4& color);
	void SetColor(DirectX::XMFLOAT4&& color);
	void SetDirection(DirectX::XMFLOAT3& dir);
	void SetDirection(DirectX::XMFLOAT3&& dir);
	void SetAttenuation(DirectX::XMFLOAT3& attenuation);
	void SetAttenuation(DirectX::XMFLOAT3&& attenuation);
	void SetRange(float range);
	void SetAngle(float angle);

	const DirectX::XMFLOAT3& GetPos();
	const DirectX::XMFLOAT4& GetColor();
	const DirectX::XMFLOAT3& GetDirection();
	const DirectX::XMFLOAT3& GetAttenuation();
	float GetRange();
	float GetAngle();
private:
	Kind m_kind;
	DirectX::XMFLOAT3 m_pos;
	DirectX::XMFLOAT4 m_color;
	DirectX::XMFLOAT3 m_direction;
	DirectX::XMFLOAT3 m_attenuation;
	float m_range;
	float m_angle;
};

#endif // __GG_LIHGT_H__