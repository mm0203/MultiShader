#ifndef __DEBUG_MANIPULATOR_H__
#define __DEBUG_MANIPULATOR_H__

#include <System/Debug/Debug.h>
#include <System/Singleton.hpp>
#include <DirectXMath.h>

#ifdef USE_DEBUG

#define MANIPULATOR Manipulator::GetInstance()

class Manipulator : public Singleton<Manipulator>
{
	SINGLETON_CONSTRACTOR(Manipulator);
private:
	struct InputParam
	{
		struct Mouse
		{
			DirectX::XMFLOAT3 rayStart;
			DirectX::XMFLOAT3 rayEnd;
		};

		Mouse mouse;

		DirectX::XMFLOAT3 mouseToCenter;
		DirectX::XMFLOAT3 mouseDir;
		DirectX::XMFLOAT3 cameraDir;
		
		DirectX::XMFLOAT2 moveValue;
		DirectX::XMFLOAT3 recordMouseStart;
		DirectX::XMFLOAT3 clickPoint;
		DirectX::XMFLOAT3 cameraForward;
		DirectX::XMFLOAT3 cameraRight;
		DirectX::XMFLOAT3 cameraUp;

		float startScale;
		float scale;
		float size;
	};

public:

	void Init() final;
	void Uninit() final;
	void Update();
	void Draw();

	void SetPose(
		const DirectX::XMFLOAT3 &pos,
		const DirectX::XMFLOAT3 &rotate,
		const DirectX::XMFLOAT3 &scale);

	const DirectX::XMFLOAT3 &GetPos() const;
	const DirectX::XMFLOAT3 &GetRotate() const;
	const DirectX::XMFLOAT3 &GetScale() const;
	const DirectX::XMMATRIX GetMatrix() const;

private:
	InputParam MakeParam();

	void UpdateChangeEdit();

	//void UpdateChange();

	void HitCheckTranslateGuide(const InputParam& input);
	//void HitCheckCircle(const Vector3 &mouseStart, const Vector3 &mouseEnd);

	//void ChangeTranslate();
	//void ChangeRotation();
	//void ChangeScaling();

	void DrawTranslate();
	void DrawRotation();
	void DrawScaling();

	//const Vector3 CalcPlanePos(int index);
	//const float CalcMoveValue(int index);
	//const Vector4 &HitColor(int index);

private:
	char m_editKind;
	char m_hitKind;
	bool m_isEnableMove;

	bool m_bHitGuide[3];
	int m_mouseLoopX;
	int m_mouseLoopY;

	//Vector3 m_preParam;

	DirectX::XMFLOAT3 m_pos;
	DirectX::XMFLOAT3 m_rotate;
	DirectX::XMFLOAT3 m_scale;
};
#endif // USE_DEBUG

#endif // __DEBUG_MANIPULATOR_H__