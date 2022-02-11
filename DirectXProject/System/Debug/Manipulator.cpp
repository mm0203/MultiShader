#include <System/Debug/Manipulator.h>
#include <System/Debug/Geometory.h>
#include <System/Input.h>
#include <System/Camera/CameraManager.hpp>
#include <System/Collision.h>
#include <System/Macro.h>

#ifdef USE_DEBUG

//--- 定数
enum ManipulatorEditKind
{
	MANIPULATOR_EDIT_NONE,
	MANIPULATOR_EDIT_TRANSLATE,
	MANIPULATOR_EDIT_ROTATION,
	MANIPULATOR_EDIT_SCALING
};
enum ManipulatorHitKind
{
	MANIPULATOR_HIT_NONE,
	MANIPULATOR_HIT_PLANE_YZ,
	MANIPULATOR_HIT_PLANE_XZ,
	MANIPULATOR_HIT_PLANE_XY,
	MANIPULATOR_HIT_AXIS_X,
	MANIPULATOR_HIT_AXIS_Y,
	MANIPULATOR_HIT_AXIS_Z,
	MANIPULATOR_HIT_BOX,
	MANIPULATOR_HIT_CIRCLE,

	MANIPULATOR_HIT_PLANE_START = MANIPULATOR_HIT_PLANE_YZ,
	MANIPULATOR_HIT_AXIS_START = MANIPULATOR_HIT_AXIS_X,
};

const DirectX::XMFLOAT4 ManipulatorAxisColor[] =
{
	{0.9f, 0.0f, 0.0f, 1.0f}, {0.0f, 0.9f, 0.0f, 1.0f}, {0.0f, 0.0f, 0.9f, 1.0f}
};
const float ManipulatorAxisPlaneSize = 0.2f;

/*
const Vector3 PLANE_NORMAL[] =
{
	Vector3::Right,
	Vector3::Up,
	Vector3::Forward
};
const Vector4 AXIS_COLOR[] =
{
	Vector4::Red,
	Vector4::Green,
	Vector4::Blue
};
const Vector4 HIT_COLOR = Vector4::Yellow;
const Vector4 NO_HIT_COLOR = Vector4(0.7f, 0.7f, 0.7f, 1.0f);

const float		MNPLTR_SIZE					= 0.2f;
const float		MNPLTR_AXIS_BOLD			= 0.02f;
const float		MNPLTR_TRANS_PLANE_SCALE	= 0.25f;
const float		MNPLTR_TRANS_PLANE_HALF		= MNPLTR_TRANS_PLANE_SCALE * 0.5f;
const float		MNPLTR_TRANS_PLANE_CHECK	= 0.03f + MNPLTR_TRANS_PLANE_SCALE;
const float		MNPLTR_SCALE_BOX_SIZE		= 0.11f;
const float		MNPLTR_SCALE_BOX_CHECK		= 0.02f + MNPLTR_SCALE_BOX_SIZE;
const Vector3	MNPLTR_SCALE_BOX_SIZE_V		= Vector3(MNPLTR_SCALE_BOX_SIZE, MNPLTR_SCALE_BOX_SIZE, MNPLTR_SCALE_BOX_SIZE);
const Vector3	MNPLTR_SCALE_BOX_CHECK_V	= Vector3(MNPLTR_SCALE_BOX_CHECK, MNPLTR_SCALE_BOX_CHECK, MNPLTR_SCALE_BOX_CHECK);
const float		MNPLTR_ROT_CHECK			= 0.01f;
*/

void Manipulator::Init()
{
	m_editKind = MANIPULATOR_EDIT_NONE;
	m_hitKind = MANIPULATOR_HIT_NONE;
	m_isEnableMove = false;
	m_pos = DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f);
	/*
	m_bHitGuide = { false, false, false }
	m_pos(0.0f, 0.0f, 0.0f)
	m_rotate(0.0f, 0.0f, 0.0f)
	m_scale(1.0f, 1.0f, 1.0f)
	m_mouseLoopX(0)
	m_mouseLoopY(0)*/
}
void Manipulator::Uninit()
{
}

void Manipulator::Update()
{
	// 編集状態の更新
	UpdateChangeEdit();
	if (m_editKind == MANIPULATOR_EDIT_NONE)
	{
		return;
	}

	// パラメータ更新
	InputParam input = MakeParam();

	if (!m_isEnableMove)
	{
		// ガイドとの当たり判定
		switch(m_editKind)
		{
		case MANIPULATOR_EDIT_TRANSLATE:
			HitCheckTranslateGuide(input);
			break;
		}
	}
	else
	{
		// マニピュレーター移動中の動作
	}

	/*
	// 処理用のパラメータ更新
	const CameraInterface *pCamera = CAMERA->GetMain();
	const Matrix &view = pCamera->GetView();
	const Matrix &proj = pCamera->GetProjection();
	m_param.scale =
		Vec4TransformCoord(Vector4(m_pos.x, m_pos.y, m_pos.z, 1), view * proj).w;
	m_param.size = MNPLTR_SIZE * m_param.scale;

	Vector2 mousePos = GetMousePos();
	Vector2 screenSize(
		static_cast<float>(GetSystemMetrics(SM_CXSCREEN)), 
		static_cast<float>(GetSystemMetrics(SM_CYSCREEN)));
	Vector2 mouseShift = screenSize;
	mouseShift.x *= m_mouseLoopX;
	mouseShift.y *= m_mouseLoopY;
	m_param.mouseStart = ScreenToCamera(mousePos + mouseShift, 0.0f, Vector2(1280, 720));
	m_param.mouseEnd = ScreenToCamera(mousePos + mouseShift, 1.0f, Vector2(1280, 720));
	m_param.mouseToCenter = m_pos - m_param.mouseStart;
	m_param.mouseDir = m_param.mouseEnd - m_param.mouseStart;

	Vector3 cameraDir = pCamera->GetPos() - m_pos;
	m_param.cameraDir =
		Vector3(Signf(cameraDir.x), Signf(cameraDir.y), Signf(cameraDir.z));
	
	// クリックしていないときの処理
	if (m_click == NO_CLICK)
	{
		UpdateHitCheck();
	}
	// クリック時
	else
	{
		UpdateChange();
	}*/

}

void Manipulator::Draw()
{
	switch (m_editKind)
	{
	default: break;
	case MANIPULATOR_EDIT_TRANSLATE:	DrawTranslate();	break;
	case MANIPULATOR_EDIT_ROTATION:		DrawRotation();		break;
	case MANIPULATOR_EDIT_SCALING:		DrawScaling();		break;
	}
}

void Manipulator::SetPose(
	const DirectX::XMFLOAT3 &pos,
	const DirectX::XMFLOAT3 &rotate,
	const DirectX::XMFLOAT3 &scale)
{
	m_pos = pos;
	m_rotate = rotate;
	m_scale = scale;
}

const DirectX::XMFLOAT3 &Manipulator::GetPos() const
{
	return m_pos;
}
const DirectX::XMFLOAT3 &Manipulator::GetRotate() const
{
	return m_rotate;
}
const DirectX::XMFLOAT3 &Manipulator::GetScale() const
{
	return m_scale;
}
const DirectX::XMMATRIX Manipulator::GetMatrix() const
{
	return
		DirectX::XMMatrixScaling(m_scale.x, m_scale.y, m_scale.z) *
		DirectX::XMMatrixRotationZ(m_rotate.z) *
		DirectX::XMMatrixRotationX(m_rotate.x) *
		DirectX::XMMatrixRotationY(m_rotate.y) *
		DirectX::XMMatrixTranslation(m_pos.x, m_pos.y, m_pos.z);
}

Manipulator::InputParam Manipulator::MakeParam()
{
	Camera* pCamera = CAMERA->Get();

	POINT mouse = GetMousePos();

	InputParam input;
	input.mouse.rayStart = pCamera->CalcWorldPos(DirectX::XMFLOAT3(mouse.x, mouse.y, 0.0f));
	input.mouse.rayEnd = pCamera->CalcWorldPos(DirectX::XMFLOAT3(mouse.x, mouse.y, 1.0f));

	return input;
}

void Manipulator::UpdateChangeEdit()
{
	if (m_isEnableMove) { return; }
	if (IsKeyTrigger('Q'))
	{
		m_editKind = MANIPULATOR_EDIT_NONE;
	}
	if (IsKeyTrigger('W'))
	{
		m_editKind = MANIPULATOR_EDIT_TRANSLATE;
	}
	if (IsKeyTrigger('E'))
	{
		m_editKind = MANIPULATOR_EDIT_ROTATION;
	}
	if (IsKeyTrigger('R'))
	{
		m_editKind = MANIPULATOR_EDIT_SCALING;
	}
}

/*
void Manipulator::UpdateHitCheck()
{
	for (int i = 0; i < 3; ++i)
	{
		m_bHitGuide[i] = false;
	}
	switch (m_edit)
	{
	case TRANSLATE:
		HitCheckAxis(m_param.mouseStart, m_param.mouseEnd);
		m_preParam = m_pos;
		break;
	case SCALING:
		HitCheckAxis(m_param.mouseStart, m_param.mouseEnd);
		m_preParam = m_scale;
		break;

	case ROTATION:
		HitCheckCircle(m_param.mouseStart, m_param.mouseEnd);
		m_preParam = m_rotate;
		break;
	}

	// クリック判定
	if (IsTriggerKeyboard(VK_LBUTTON) && !IsPressKeyboard(VK_LMENU))
	{
		int hitCount = 0;
		hitCount += m_bHitGuide[0];
		hitCount += m_bHitGuide[1];
		hitCount += m_bHitGuide[2];
		switch (hitCount)
		{
		case 0:
			m_click = NO_HIT;
			break;
		case 1:
			m_click = m_edit == ROTATION ? HIT_CIRCLE : HIT_AXIS;
			for (int i = 0; i < 3; ++i)
			{
				if (m_bHitGuide[i]) {
					m_param.clickPoint = CalcPlanePos(i);
					break;
				}
			}
			break;
		case 2:
			m_click = HIT_PLANE;
			if (!m_bHitGuide[0]) { m_param.clickPoint = CalcPlanePos(0); }
			else if (!m_bHitGuide[1]) { m_param.clickPoint = CalcPlanePos(1); }
			else if (!m_bHitGuide[2]) { m_param.clickPoint = CalcPlanePos(2); }
			break;
		case 3:
			m_click = HIT_BOX;
			break;
		}
		m_param.startScale = m_param.scale;
		m_param.recordMouseStart = m_param.mouseStart;
		m_mouseLoopX = 0;
		m_mouseLoopY = 0;
	}
}
/*
void Manipulator::UpdateChange()
{
	const CameraInterface *pCamera = CAMERA->GetMain();

	// カメラ平面
	m_param.cameraForward = pCamera->GetLook() - pCamera->GetPos();
	m_param.cameraUp = pCamera->GetUp();
	m_param.cameraRight = Vec3Normalize(Vec3Cross(m_param.cameraUp, m_param.cameraForward));
	m_param.cameraUp = Vec3Normalize(Vec3Cross(m_param.cameraForward, m_param.cameraRight));

	// 移動量
	Vector3 dir = m_param.mouseStart - m_param.recordMouseStart;
	m_param.moveValue = Vector2(Vec3Dot(m_param.cameraRight, dir), Vec3Dot(m_param.cameraUp, dir));

	// マウス補正
	const float MARGIN = 2;
	const float WARP = MARGIN * 1.5;
	Vector2 mousePos = GetMousePos(false);


	Vector2 screenSize(
		static_cast<float>(GetSystemMetrics(SM_CXSCREEN)), 
		static_cast<float>(GetSystemMetrics(SM_CYSCREEN)));
	if (mousePos.x < MARGIN)
	{
		--m_mouseLoopX;
		SetMousePos(Vector2(screenSize.x - WARP, mousePos.y));
	}
	else if (screenSize.x - MARGIN < mousePos.x)
	{
		++m_mouseLoopX;
		SetMousePos(Vector2(WARP, mousePos.y));
	}
	if (mousePos.y < MARGIN)
	{
		--m_mouseLoopY;
		SetMousePos(Vector2(mousePos.x, screenSize.y - WARP));
	}
	else if (screenSize.y - MARGIN < mousePos.y)
	{
		++m_mouseLoopY;
		SetMousePos(Vector2(mousePos.x, WARP));
	}

	// 移動処理
	switch (m_edit)
	{
	case TRANSLATE:	ChangeTranslate(); break;
	case ROTATION:	ChangeRotation();	break;
	case SCALING:	ChangeScaling(); break;
	}

	// 終わり
	if (IsReleaseKeyboard(VK_LBUTTON))
	{
		m_click = NO_CLICK;
		m_mouseLoopX = 0;
		m_mouseLoopY = 0;
	}
}*/

void Manipulator::HitCheckTranslateGuide(const InputParam& input)
{
	CollisionSegment eyeLine;
	eyeLine.start = input.mouse.rayStart;
	eyeLine.end = input.mouse.rayEnd;

	// 軸との当たり判定
	float radius = CMETER(4.0f);
	DirectX::XMFLOAT3 axisEnd[] =
	{
		DirectX::XMFLOAT3(m_pos.x + 1.0f, m_pos.y, m_pos.z),
		DirectX::XMFLOAT3(m_pos.x, m_pos.y + 1.0f, m_pos.z),
		DirectX::XMFLOAT3(m_pos.x, m_pos.y, m_pos.z + 1.0f),
	};
	for (int i = 0; i < 3; ++i)
	{
		CollisionSegment axis = { m_pos, axisEnd[i] };
		float distance = CollisionCheckDistance(eyeLine, axis, nullptr, nullptr);
		if (distance < radius)
		{
			m_hitKind = MANIPULATOR_HIT_AXIS_START + i;
			return;
		}
	}
	m_hitKind = MANIPULATOR_HIT_NONE;
}
/*
void Manipulator::HitCheckCircle(const Vector3 &mouseStart, const Vector3 &mouseEnd)
{
	const float checkMargin = MNPLTR_ROT_CHECK * m_param.scale;
	ResultRayCrossPoint result;


	// 球と線分の当たり判定
	HitCount hitCount = CollisionRaySphere(
		mouseStart, mouseEnd,
		m_pos, m_param.size, &result);
	Vector3 rayHitDir[] = {
		result.front - m_pos,
		result.back - m_pos
	};


	for (int i = 0; i < 3; ++i)
	{		
		if (hitCount != HIT_COUNT_NONE)
		{
			float d[] = {
				Absf(Vec3Dot(PLANE_NORMAL[i], rayHitDir[0])),
				Absf(Vec3Dot(PLANE_NORMAL[i], rayHitDir[1])),
			};
			if (d[0] < checkMargin || d[1] < checkMargin)
			{
				m_bHitGuide[i] = true;
				break;
			}
		}

		Vector3 mouseHit = CalcPlanePos(i);
		float hitDistance = Vec3Length(mouseHit - m_pos);
		if (Absf(hitDistance - m_param.size) < checkMargin)
		{
			m_bHitGuide[i] = true;
			break;
		}
	}
}


void Manipulator::ChangeTranslate()
{
	if (m_click == HIT_AXIS)
	{
		// 移動
		for (int i = 0; i < 3; ++i)
		{
			if (!m_bHitGuide[i])
			{
				continue;
			}
			m_pos = m_preParam + PLANE_NORMAL[i] * CalcMoveValue(i);
			break;
		}
	}
	else if (m_click == HIT_PLANE)
	{
		// 平面チェック
		for (int i = 0; i < 3; ++i)
		{
			if (m_bHitGuide[0] == (i != 0) &&
				m_bHitGuide[1] == (i != 1) &&
				m_bHitGuide[2] == (i != 2))
			{
				m_pos = CalcPlanePos(i) + (m_preParam - m_param.clickPoint);
				break;
			}
		}
	}
}
void Manipulator::ChangeRotation()
{
	// 移動
	Vector3 F = Vec3Normalize(CAMERA->GetMain()->GetPos() - m_pos);
	Vector3 U = CAMERA->GetMain()->GetUp();
	Vector3 R = Vec3Normalize(Vec3Cross(U, F));
	U = Vec3Normalize(Vec3Cross(F, R));
	Vector3 startVec = m_param.clickPoint - m_pos;
	for (int i = 0; i < 3; ++i)
	{
		if (!m_bHitGuide[i])
		{
			continue;
		}
		float dirX = m_param.moveValue.x;
		float dirY = m_param.moveValue.y;
		dirX *= Signf(Vec3Dot(U, startVec) * Vec3Dot(F, PLANE_NORMAL[i]));
		dirY *= Signf(Vec3Dot(R, startVec) * Vec3Dot(F, PLANE_NORMAL[i]));
		float move = dirX + dirY;
		move = (move / CAMERA->GetMain()->GetNear()) * 180.0f;
		m_rotate.v[i] = m_preParam.v[i] + move;
		break;
	}
}
void Manipulator::ChangeScaling()
{
	if (m_click == HIT_AXIS)
	{
		// 移動
		for (int i = 0; i < 3; ++i)
		{
			if (!m_bHitGuide[i])
			{
				continue;
			}
			m_scale.v[i] = m_preParam.v[i] + CalcMoveValue(i);
			break;
		}

	}
	else if (m_click == HIT_BOX)
	{
		float move = m_param.moveValue.x + m_param.moveValue.y;
		move = (move / CAMERA->GetMain()->GetNear()) * m_param.startScale;
		m_scale = m_preParam + Vector3(move, move, move);
	}
}*/

void Manipulator::DrawTranslate()
{
	// 行列
	DirectX::XMMATRIX transMat = DirectX::XMMatrixTranslation(m_pos.x, m_pos.y, m_pos.z);
	DirectX::XMMATRIX rotX = DirectX::XMMatrixRotationX(DirectX::XM_PIDIV2);
	DirectX::XMMATRIX rotY = DirectX::XMMatrixRotationY(-DirectX::XM_PIDIV2);
	DirectX::XMMATRIX rotZ = DirectX::XMMatrixRotationZ(DirectX::XM_PIDIV2);
	DirectX::XMMATRIX planeTrans =
		DirectX::XMMatrixTranslation(0.5f, 0.5f, 0.0f) *
		DirectX::XMMatrixScaling(ManipulatorAxisPlaneSize, ManipulatorAxisPlaneSize, ManipulatorAxisPlaneSize);
	DirectX::XMMATRIX axisMat[] = { transMat, rotZ * transMat, rotY * transMat };
	DirectX::XMMATRIX planeMat[] = { planeTrans * rotY * transMat, planeTrans * rotX * transMat, planeTrans * transMat };

	// 矢印描画
	for (int i = 0; i < 3; ++i)
	{
		if (m_hitKind - MANIPULATOR_HIT_AXIS_START == i)
		{
			GEOMETORY->DrawArrow(axisMat[i], DirectX::XMFLOAT4(1.0f ,1.0f, 0.0f, 1.0f));
		}
		else
		{
			GEOMETORY->DrawArrow(axisMat[i], ManipulatorAxisColor[i]);
		}
	}
	// 軸平面描画
	for (int i = 0; i < 3; ++i)
	{
		if (m_hitKind - MANIPULATOR_HIT_PLANE_START == i)
		{
			GEOMETORY->DrawPolygon(planeMat[i], DirectX::XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f));
		}
		else
		{
			GEOMETORY->DrawPolygon(planeMat[i], ManipulatorAxisColor[i]);
		}
	}

}
void Manipulator::DrawRotation()
{
	/*const float radius = m_param.size * 2.0f;
	const Matrix translate = MatrixTranslationVec3(m_pos);
	const Matrix mat[] =
	{
		MatrixRotationY(90),
		MatrixRotationX(90),
		MatrixIdentity()
	};
	int hit = 0;
	for (; hit < 3 && !m_bHitGuide[hit]; ++hit) {}
	Matrix changeMat = MatrixIdentity();
	if (hit < 3)
	{
		Vector3 change = m_rotate - m_preParam;
		changeMat = MatrixRotationAxis(PLANE_NORMAL[hit], change.v[hit]);
	}
	for (int i = 0; i < 3; ++i)
	{
		debug::DrawCircle(mat[i] * changeMat * translate, radius, HitColor(i), true, false);
	}*/
}
void Manipulator::DrawScaling()
{
	/*const Vector3 boxSize = MNPLTR_SCALE_BOX_SIZE_V * m_param.size;
	Vector3 pos;
	Vector4 color;

	// 軸先のボックス
	Vector3 change = m_scale - m_preParam;
	for (int i = 0; i < 3; ++i)
	{
		pos = m_pos + PLANE_NORMAL[i] * m_param.size * (change.v[i] + 1.0f);
		color = HitColor(i);
		debug::DrawLine(m_pos, pos, color, false);
		debug::DrawBox(pos, boxSize, color, false, false);
	}
	// 中心のボックス
	if (m_bHitGuide[0] && m_bHitGuide[1] && m_bHitGuide[2])
	{
		debug::DrawBox(m_pos, boxSize, HIT_COLOR, false, false);
	}
	else
	{
		debug::DrawBox(m_pos, boxSize, NO_HIT_COLOR, false, false);
	}*/
}


/*
const Vector3 Manipulator::CalcPlanePos(int index)
{
	float d = Absf(
		Vec3Dot(PLANE_NORMAL[index], m_param.mouseToCenter) /
		Vec3Dot(PLANE_NORMAL[index], m_param.mouseDir));
	return m_param.mouseStart + m_param.mouseDir * d;
}

const float Manipulator::CalcMoveValue(int index)
{
	float dirX;
	float dirY;
	float move;
	float nearClip;

	nearClip = CAMERA->GetMain()->GetNear();
	dirX = m_param.moveValue.x;
	dirY = m_param.moveValue.y;
	dirX *= Signf(Vec3Dot(m_param.cameraRight, PLANE_NORMAL[index]));
	dirY *= Signf(Vec3Dot(m_param.cameraUp, PLANE_NORMAL[index]));
	move = dirX + dirY;
	move = (move / nearClip) * m_param.startScale;
	return move;
}
const Vector4 &Manipulator::HitColor(int index)
{
	return m_bHitGuide[index] ? HIT_COLOR : AXIS_COLOR[index];
}*/

#endif
// EOF