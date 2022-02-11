#ifndef __GG_LIGHT_CAMERA_H__
#define __GG_LIGHT_CAMERA_H__

#include <System/Camera/Camera.h>

class GGLightCamera : public Camera
{
protected:
	void Update() final;
public:
	GGLightCamera();
	~GGLightCamera();
	void Draw() final;

	DirectX::XMMATRIX CalcProjection() final;

	void SetDirection(DirectX::XMFLOAT3 dir);
};

#endif // __GG_LIGHT_CAMERA_H__