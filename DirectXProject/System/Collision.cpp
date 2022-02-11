#include <System/Collision.h>

void CalcFactorPointRayDistance(DirectX::XMFLOAT3& pos, CollisionRay& ray, float& t)
{
	DirectX::XMVECTOR V = DirectX::XMLoadFloat3(&ray.direction);
	DirectX::XMVECTOR vP = DirectX::XMLoadFloat3(&pos);
	DirectX::XMVECTOR vStart = DirectX::XMLoadFloat3(&ray.pos);
	V = DirectX::XMVector3Normalize(V);
	DirectX::XMVECTOR vDot = DirectX::XMVector3Dot(V, DirectX::XMVectorSubtract(vP, vStart));
	DirectX::XMStoreFloat(&t, vDot);
}
void CalcFactor2RayDistance(CollisionRay& rayA, CollisionRay& rayB, float& t1, float& t2)
{
	/* http://marupeke296.com/COL_3D_No27_CapsuleCapsule.html
	* 2�̖��������̍ŒZ����������A'B'�x�N�g���́A�o���̒����ɐ����ȃx�N�g��
	* �@����A'B'���ꂼ��̈ʒu�����߂�
	*	A'	= A + t1V1 / B' = B + t2V2
	* �A��̎�����t2�����߂鎮�𗧂Ă�
	*	t2	= V2�E(A' - B)
	*		= V2�E(A + t1V1 - B)
	* �@���ς̌��� (a + b)�Ec = a�Ec + b�Ec�@����
	*		= V2�E(A - B) + V2�Et1V1
	* �@���ς̌��� ka�Eb = k(a�Eb)�@����
	*		= V2�E(A - B) + t1(V1�EV2)
	* �BV1��A'B'�͐����Ȃ��߁A���ς̎�����t1�����߂�
	*	0	= V1�E(A' - B')
	*		= V1�E((A + t1V1) - (B + t2V2))
	*		= V1�E(A + t1V1 - B - t2V2)
	* �@���ς̌��� (a + b)�Ec = a�Ec + b�Ec�@����
	*		= V1�E(A - B) + V1�Et1V1 - V1�Et2V2
	* �@���ς̌��� ka�Eb = k(a�Eb)�@����
	*		= V1�E(A - B) + t1(V1�EV1) - t2(V1�EV2)
	* �@V1�AV2�͐��K���x�N�g���̂��߁A�����x�N�g�����m�̓��ό��ʂ͂P�ƂȂ�
	*		= V1�E(A - B) + t1 - t2(V1�EV2)
	* �@t2�����߂鎮���
	*		= V1�E(A - B) + t1 - (V2�E(A - B) + t1(V1�EV2))(V1�EV2)
	* �@V1�EV2��DotV�ƒu��
	*		= V1�E(A - B) + t1 - (V2�E(A - B) + t1DotV) * DotV
	*		= V1�E(A - B) + t1 - ((V2�E(A - B)) * DotV + t1DotV^2)
	*		= V1�E(A - B) + t1 - (V2�E(A - B)) * DotV - t1DotV^2
	*		= V1�E(A - B) + t1 - t1DotV^2 - (V2�E(A - B)) * DotV
	*		= V1�E(A - B) + t1(1 - DotV^2) - (V2�E(A - B)) * DotV
	* �@t1�����߂鎮�ɕό`
	*	-t1(1 - DotV^2)	= V1�E(A - B) - (V2�E(A - B)) * DotV
	*	t1(1 - DotV^2)	= -V1�E(A - B) + (V2�E(A - B)) * DotV
	*	t1(1 - DotV^2)	= (V2�E(A - B)) * DotV - V1�E(A - B)
	*	t1 = ((V2�E(A - B)) * DotV - V1�E(A - B)) / (1 - DotV^2)
	*/

	// ���O�̌v�Z
	DirectX::XMVECTOR V1 = DirectX::XMLoadFloat3(&rayA.direction);
	DirectX::XMVECTOR V2 = DirectX::XMLoadFloat3(&rayB.direction);
	DirectX::XMVECTOR vA = DirectX::XMLoadFloat3(&rayA.pos);
	DirectX::XMVECTOR vB = DirectX::XMLoadFloat3(&rayB.pos);
	DirectX::XMVECTOR vAB = DirectX::XMVectorSubtract(vA, vB);
	V1 = DirectX::XMVector3Normalize(V1);
	V2 = DirectX::XMVector3Normalize(V2);
	DirectX::XMVECTOR vDotV = DirectX::XMVector3Dot(V1, V2);
	DirectX::XMVECTOR vDotV1AB = DirectX::XMVector3Dot(V1, vAB);
	DirectX::XMVECTOR vDotV2AB = DirectX::XMVector3Dot(V2, vAB);
	float dDotV, dV1AB, dV2AB;
	DirectX::XMStoreFloat(&dDotV, vDotV);
	DirectX::XMStoreFloat(&dV1AB, vDotV1AB);
	DirectX::XMStoreFloat(&dV2AB, vDotV2AB);

	// t1,t2�����߂�
	t1 = (dV2AB * dDotV - dV1AB) / (1.0f - dDotV * dDotV);
	t2 = dV2AB + t1 * dDotV;
}

float CollisionCheckDistance(CollisionRay& rayA, CollisionRay& rayB, DirectX::XMFLOAT3* pPosA, DirectX::XMFLOAT3* pPosB)
{
	float t1, t2;
	CalcFactor2RayDistance(rayA, rayB, t1, t2);

	// ���O�̌v�Z
	DirectX::XMVECTOR V1 = DirectX::XMLoadFloat3(&rayA.direction);
	DirectX::XMVECTOR V2 = DirectX::XMLoadFloat3(&rayB.direction);
	DirectX::XMVECTOR vA = DirectX::XMLoadFloat3(&rayA.pos);
	DirectX::XMVECTOR vB = DirectX::XMLoadFloat3(&rayA.pos);
	V1 = DirectX::XMVector3Normalize(V1);
	V2 = DirectX::XMVector3Normalize(V2);
	
	// �ŒZ���������߂�
	float distance;
	vA = DirectX::XMVectorAdd(vA, DirectX::XMVectorScale(V1, t1));
	vB = DirectX::XMVectorAdd(vB, DirectX::XMVectorScale(V2, t2));
	DirectX::XMStoreFloat(&distance, DirectX::XMVectorSubtract(vA, vB));
	
	// �ŒZ�������W���i�[
	if (pPosA)
	{
		DirectX::XMStoreFloat3(pPosA, vA);
	}
	if (pPosB)
	{
		DirectX::XMStoreFloat3(pPosB, vB);
	}
	
	return distance;
}
float CollisionCheckDistance(CollisionSegment& segmentA, CollisionSegment& segmentB, DirectX::XMFLOAT3* pPosA, DirectX::XMFLOAT3* pPosB)
{
	// �����ł̍ŒZ�����W�������߂�
	CollisionRay rayA, rayB;
	rayA.pos = segmentA.start;
	rayB.pos = segmentB.start;
	rayA.direction = DirectX::XMFLOAT3(segmentA.end.x - segmentA.start.x, segmentA.end.y - segmentA.start.y, segmentA.end.z - segmentA.start.z);
	rayB.direction = DirectX::XMFLOAT3(segmentB.end.x - segmentB.start.x, segmentB.end.y - segmentB.start.y, segmentB.end.z - segmentB.start.z);
	float t1, t2, t = -1.0f;
	CalcFactor2RayDistance(rayA, rayB, t1, t2);

	// �W�������C�̋����������̂ŁA���̋������������ɐ�߂銄���ɕϊ�
	float length[2];
	DirectX::XMStoreFloat(&length[0], DirectX::XMVector3Length(DirectX::XMLoadFloat3(&rayA.direction)));
	DirectX::XMStoreFloat(&length[1], DirectX::XMVector3Length(DirectX::XMLoadFloat3(&rayB.direction)));
	t1 = t1 / length[0];
	t2 = t2 / length[1];

	// t1���͈͊O�Ȃ�N�����v���A����A�̒[�_������B�Ɏ��܂��Ă邩�v�Z
	if (t1 < 0.0f)
	{
		CalcFactorPointRayDistance(segmentA.start, rayB, t);
	}
	else if(1.0f < t1)
	{
		CalcFactorPointRayDistance(segmentA.end, rayB, t);
	}
	if (0.0f <= t && t <= 1.0f)
	{
		t1 = t1 < 0.0f ? 0.0f : 1.0f;
		t2 = t;
	}

	// t2���͈͊O�Ȃ�N�����v���A����B�̒[�_������A�Ɏ��܂��Ă邩�v�Z
	if (t2 < 0.0f)
	{
		CalcFactorPointRayDistance(segmentB.start, rayA, t);
	}
	else if (1.0f < t2)
	{
		CalcFactorPointRayDistance(segmentB.end, rayA, t);
	}
	if (0.0f <= t && t <= 1.0f)
	{
		t2 = t2 < 0.0f ? 0.0f : 1.0f;
		t1 = t;
	}

	// ��L�̏����Ŏ��܂�Ȃ������̂ŁA�N�����v�������ꂼ��̒[�_���ŒZ����
	if((t1 < 0.0f || 1.0f < t1 || t2 < 0.0f || 1.0f < t2) && (t < 0.0f || 1.0f < t))
	{
		t1 = t1 < 0.0f ? 0.0f : 1.0f;
		t2 = t2 < 0.0f ? 0.0f : 1.0f;
	}

	// ���O�̌v�Z
	DirectX::XMVECTOR V1 = DirectX::XMLoadFloat3(&rayA.direction);
	DirectX::XMVECTOR V2 = DirectX::XMLoadFloat3(&rayB.direction);
	DirectX::XMVECTOR vA = DirectX::XMLoadFloat3(&rayA.pos);
	DirectX::XMVECTOR vB = DirectX::XMLoadFloat3(&rayB.pos);

	// �ŒZ���������߂�
	float distance;
	vA = DirectX::XMVectorAdd(vA, DirectX::XMVectorScale(V1, t1));
	vB = DirectX::XMVectorAdd(vB, DirectX::XMVectorScale(V2, t2));
	DirectX::XMStoreFloat(&distance, DirectX::XMVector3Length(DirectX::XMVectorSubtract(vA, vB)));

	// �ŒZ�������W���i�[
	if (pPosA)
	{
		DirectX::XMStoreFloat3(pPosA, vA);
	}
	if (pPosB)
	{
		DirectX::XMStoreFloat3(pPosB, vB);
	}
	
	return distance;
}

/*
* @brief �����ƕ��ʂ̓����蔻��
* @param[in] segment ����
* @param[in] plane ����
* @param[out] pHit ��_
* @return �Փ˂��Ă��邩
*/
bool CollisionCheckHit(CollisionSegment& segment, CollisionPlane& plane, DirectX::XMFLOAT3* pHit)
{
	DirectX::XMVECTOR vPlanePos = DirectX::XMLoadFloat3(&plane.pos);
	DirectX::XMVECTOR vTriNormal = DirectX::XMLoadFloat3(&plane.normal);
	DirectX::XMVECTOR vLineStart = DirectX::XMLoadFloat3(&segment.start);
	DirectX::XMVECTOR vLineEnd = DirectX::XMLoadFloat3(&segment.end);
	DirectX::XMVECTOR vLineVec = DirectX::XMVectorSubtract(vLineEnd, vLineStart);
	vTriNormal = DirectX::XMVector3Normalize(vTriNormal);
	// �����̎n�_�ƏI�_�A���ꂼ��̖ʂ܂ł̓��ϒl(�ŒZ����)
	DirectX::XMVECTOR vBorderDot[] =
	{
		DirectX::XMVector3Dot(vTriNormal, DirectX::XMVectorSubtract(vLineStart, vPlanePos)),
		DirectX::XMVector3Dot(vTriNormal, DirectX::XMVectorSubtract(vLineEnd, vPlanePos)),
	};
	// ���ϒl�̔䗦����������Ă��邩����
	float borderDot[2];
	DirectX::XMStoreFloat(&borderDot[0], vBorderDot[0]);
	DirectX::XMStoreFloat(&borderDot[1], vBorderDot[1]);
	if (borderDot[0] * borderDot[1] > 0.0f)
	{
		return false;
	}
	// ��_���v�Z
	if (pHit)
	{
		float rate = borderDot[0] / (borderDot[0] - borderDot[1]);
		vLineVec = DirectX::XMVectorScale(vLineVec, rate);
		DirectX::XMVECTOR vHitPos = DirectX::XMVectorAdd(vLineStart, vLineVec);
		DirectX::XMStoreFloat3(pHit, vHitPos);
	}
	return true;
}

/*
* @brief �����ƎO�p�`�̓����蔻��
* @param[in] ray ����
* @param[in] triangle �O�p�`
* @param[out] pHit ��_
* @return �Փ˂��Ă��邩
*/
bool CollisionCheckHit(CollisionSegment& segment, CollisionTriangle& triangle, DirectX::XMFLOAT3* pHit)
{
	CollisionPlane plane = { triangle.pos0, triangle.normal };
	DirectX::XMFLOAT3 hitPos;
	if (!CollisionCheckHit(segment, plane, &hitPos))
	{
		return false;
	}

	// ��_���O�p�`���Ɋ܂܂�邩
	DirectX::XMVECTOR vTriPos[] =
	{
		DirectX::XMLoadFloat3(&triangle.pos0),
		DirectX::XMLoadFloat3(&triangle.pos1),
		DirectX::XMLoadFloat3(&triangle.pos2),
	};
	DirectX::XMVECTOR vTriEdge[] =
	{
		DirectX::XMVectorSubtract(vTriPos[1], vTriPos[0]),
		DirectX::XMVectorSubtract(vTriPos[2], vTriPos[1]),
		DirectX::XMVectorSubtract(vTriPos[0], vTriPos[2]),
	};
	DirectX::XMVECTOR vHitPos = DirectX::XMLoadFloat3(&hitPos);
	DirectX::XMVECTOR vCross[3];
	for (int i = 0; i < 3; ++i)
	{
		DirectX::XMVECTOR vToHit = DirectX::XMVectorSubtract(vHitPos, vTriPos[i]);
		vToHit = DirectX::XMVector3Normalize(vToHit);
		vTriEdge[i] = DirectX::XMVector3Normalize(vTriEdge[i]);
		vCross[i] = DirectX::XMVector3Cross(vToHit, vTriEdge[i]);
	}
	float dot;
	DirectX::XMVECTOR vDot = DirectX::XMVector3Dot(vCross[1], vCross[0]);
	DirectX::XMStoreFloat(&dot, vDot);
	if (dot < 0.0f) { return false; }
	vDot = DirectX::XMVector3Dot(vCross[2], vCross[0]);
	DirectX::XMStoreFloat(&dot, vDot);
	if (dot < 0.0f) { return false; }

	if (pHit) { *pHit = hitPos; }
	return true;
}