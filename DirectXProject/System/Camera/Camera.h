#ifndef __CAMERA_H__
#define __CAMERA_H__

#include <DirectXMath.h>

class Camera
{
public:
	Camera();
	virtual ~Camera();

protected:
	virtual void Update() = 0;
	virtual DirectX::XMMATRIX CalcProjection();

public:
	void Execute();
	virtual void Draw() {}
	virtual void OnActive(const Camera& camera);
	Camera& operator=(const Camera& camera);

	DirectX::XMFLOAT3 CalcScreenPos(DirectX::XMFLOAT3 worldPos);
	DirectX::XMFLOAT3 CalcWorldPos(DirectX::XMFLOAT3 screenPos);
	
	void Enable(bool enable);
	bool IsEnable() const;
	
	const DirectX::XMFLOAT3& GetPos() const;
	const DirectX::XMFLOAT3& GetLook() const;
	const DirectX::XMFLOAT3& GetUp() const;
	const DirectX::XMFLOAT2& GetScreenSize() const;
	float GetAspect() const;
	float GetNearClip() const;
	float GetFarClip() const;
	float GetFOV() const;

	const DirectX::XMFLOAT4X4& GetLookAt() const;
	const DirectX::XMFLOAT4X4& GetProjection() const;
	const DirectX::XMMATRIX& GetLookAtMatrix() const;
	const DirectX::XMMATRIX& GetProjectionMatrix() const;
	
private:
	bool m_isEnable;
	DirectX::XMFLOAT4X4 m_lookAt;
	DirectX::XMFLOAT4X4 m_projection;
	DirectX::XMMATRIX m_lookAtMatrix;
	DirectX::XMMATRIX m_projectionMatrix;
protected:
	DirectX::XMFLOAT3 m_pos;
	DirectX::XMFLOAT3 m_look;
	DirectX::XMFLOAT3 m_up;
	DirectX::XMFLOAT2 m_screenSize;
	float m_nearClip;
	float m_farClip;
	float m_fov;
};

#endif // __CAMERA_H__