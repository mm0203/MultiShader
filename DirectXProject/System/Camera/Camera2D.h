#ifndef __CAMERA_2D_H__
#define __CAMERA_2D_H__

#include <System/Camera/Camera.h>

class Camera2D : public Camera
{
public:
	Camera2D();
	~Camera2D();
private:
	void Update() final;
	DirectX::XMMATRIX CalcProjection() final;
};

#endif // __CAMERA_2D_H__