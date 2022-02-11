#ifndef __CAMERA_DEBUG_H__
#define __CAMERA_DEBUG_H__

#include <System/Camera/Camera.h>
#include <Windows.h>

class CameraDebug : public Camera
{
public:
	CameraDebug();
	~CameraDebug();

	void OnActive(const Camera& camera);
	void Update();
private:
	POINT m_oldMousePos;
};

#endif // __CAMERA_DEBUG_H__