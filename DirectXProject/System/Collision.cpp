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
	* 2つの無限直線の最短距離を結ぶA'B'ベクトルは、双方の直線に垂直なベクトル
	* ①このA'B'それぞれの位置を求める
	*	A'	= A + t1V1 / B' = B + t2V2
	* ②上の式からt2を求める式を立てる
	*	t2	= V2・(A' - B)
	*		= V2・(A + t1V1 - B)
	* 　内積の公式 (a + b)・c = a・c + b・c　から
	*		= V2・(A - B) + V2・t1V1
	* 　内積の公式 ka・b = k(a・b)　から
	*		= V2・(A - B) + t1(V1・V2)
	* ③V1とA'B'は垂直なため、内積の式からt1を求める
	*	0	= V1・(A' - B')
	*		= V1・((A + t1V1) - (B + t2V2))
	*		= V1・(A + t1V1 - B - t2V2)
	* 　内積の公式 (a + b)・c = a・c + b・c　から
	*		= V1・(A - B) + V1・t1V1 - V1・t2V2
	* 　内積の公式 ka・b = k(a・b)　から
	*		= V1・(A - B) + t1(V1・V1) - t2(V1・V2)
	* 　V1、V2は正規化ベクトルのため、同じベクトル同士の内積結果は１となる
	*		= V1・(A - B) + t1 - t2(V1・V2)
	* 　t2を求める式より
	*		= V1・(A - B) + t1 - (V2・(A - B) + t1(V1・V2))(V1・V2)
	* 　V1・V2をDotVと置く
	*		= V1・(A - B) + t1 - (V2・(A - B) + t1DotV) * DotV
	*		= V1・(A - B) + t1 - ((V2・(A - B)) * DotV + t1DotV^2)
	*		= V1・(A - B) + t1 - (V2・(A - B)) * DotV - t1DotV^2
	*		= V1・(A - B) + t1 - t1DotV^2 - (V2・(A - B)) * DotV
	*		= V1・(A - B) + t1(1 - DotV^2) - (V2・(A - B)) * DotV
	* 　t1を求める式に変形
	*	-t1(1 - DotV^2)	= V1・(A - B) - (V2・(A - B)) * DotV
	*	t1(1 - DotV^2)	= -V1・(A - B) + (V2・(A - B)) * DotV
	*	t1(1 - DotV^2)	= (V2・(A - B)) * DotV - V1・(A - B)
	*	t1 = ((V2・(A - B)) * DotV - V1・(A - B)) / (1 - DotV^2)
	*/

	// 事前の計算
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

	// t1,t2を求める
	t1 = (dV2AB * dDotV - dV1AB) / (1.0f - dDotV * dDotV);
	t2 = dV2AB + t1 * dDotV;
}

float CollisionCheckDistance(CollisionRay& rayA, CollisionRay& rayB, DirectX::XMFLOAT3* pPosA, DirectX::XMFLOAT3* pPosB)
{
	float t1, t2;
	CalcFactor2RayDistance(rayA, rayB, t1, t2);

	// 事前の計算
	DirectX::XMVECTOR V1 = DirectX::XMLoadFloat3(&rayA.direction);
	DirectX::XMVECTOR V2 = DirectX::XMLoadFloat3(&rayB.direction);
	DirectX::XMVECTOR vA = DirectX::XMLoadFloat3(&rayA.pos);
	DirectX::XMVECTOR vB = DirectX::XMLoadFloat3(&rayA.pos);
	V1 = DirectX::XMVector3Normalize(V1);
	V2 = DirectX::XMVector3Normalize(V2);
	
	// 最短距離を求める
	float distance;
	vA = DirectX::XMVectorAdd(vA, DirectX::XMVectorScale(V1, t1));
	vB = DirectX::XMVectorAdd(vB, DirectX::XMVectorScale(V2, t2));
	DirectX::XMStoreFloat(&distance, DirectX::XMVectorSubtract(vA, vB));
	
	// 最短距離座標を格納
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
	// 直線での最短距離係数を求める
	CollisionRay rayA, rayB;
	rayA.pos = segmentA.start;
	rayB.pos = segmentB.start;
	rayA.direction = DirectX::XMFLOAT3(segmentA.end.x - segmentA.start.x, segmentA.end.y - segmentA.start.y, segmentA.end.z - segmentA.start.z);
	rayB.direction = DirectX::XMFLOAT3(segmentB.end.x - segmentB.start.x, segmentB.end.y - segmentB.start.y, segmentB.end.z - segmentB.start.z);
	float t1, t2, t = -1.0f;
	CalcFactor2RayDistance(rayA, rayB, t1, t2);

	// 係数がレイの距離を示すので、この距離が線分中に占める割合に変換
	float length[2];
	DirectX::XMStoreFloat(&length[0], DirectX::XMVector3Length(DirectX::XMLoadFloat3(&rayA.direction)));
	DirectX::XMStoreFloat(&length[1], DirectX::XMVector3Length(DirectX::XMLoadFloat3(&rayB.direction)));
	t1 = t1 / length[0];
	t2 = t2 / length[1];

	// t1が範囲外ならクランプし、線分Aの端点が線分Bに収まってるか計算
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

	// t2が範囲外ならクランプし、線分Bの端点が線分Aに収まってるか計算
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

	// 上記の処理で収まらなかったので、クランプしたそれぞれの端点が最短距離
	if((t1 < 0.0f || 1.0f < t1 || t2 < 0.0f || 1.0f < t2) && (t < 0.0f || 1.0f < t))
	{
		t1 = t1 < 0.0f ? 0.0f : 1.0f;
		t2 = t2 < 0.0f ? 0.0f : 1.0f;
	}

	// 事前の計算
	DirectX::XMVECTOR V1 = DirectX::XMLoadFloat3(&rayA.direction);
	DirectX::XMVECTOR V2 = DirectX::XMLoadFloat3(&rayB.direction);
	DirectX::XMVECTOR vA = DirectX::XMLoadFloat3(&rayA.pos);
	DirectX::XMVECTOR vB = DirectX::XMLoadFloat3(&rayB.pos);

	// 最短距離を求める
	float distance;
	vA = DirectX::XMVectorAdd(vA, DirectX::XMVectorScale(V1, t1));
	vB = DirectX::XMVectorAdd(vB, DirectX::XMVectorScale(V2, t2));
	DirectX::XMStoreFloat(&distance, DirectX::XMVector3Length(DirectX::XMVectorSubtract(vA, vB)));

	// 最短距離座標を格納
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
* @brief 線分と平面の当たり判定
* @param[in] segment 線分
* @param[in] plane 平面
* @param[out] pHit 交点
* @return 衝突しているか
*/
bool CollisionCheckHit(CollisionSegment& segment, CollisionPlane& plane, DirectX::XMFLOAT3* pHit)
{
	DirectX::XMVECTOR vPlanePos = DirectX::XMLoadFloat3(&plane.pos);
	DirectX::XMVECTOR vTriNormal = DirectX::XMLoadFloat3(&plane.normal);
	DirectX::XMVECTOR vLineStart = DirectX::XMLoadFloat3(&segment.start);
	DirectX::XMVECTOR vLineEnd = DirectX::XMLoadFloat3(&segment.end);
	DirectX::XMVECTOR vLineVec = DirectX::XMVectorSubtract(vLineEnd, vLineStart);
	vTriNormal = DirectX::XMVector3Normalize(vTriNormal);
	// 線分の始点と終点、それぞれの面までの内積値(最短距離)
	DirectX::XMVECTOR vBorderDot[] =
	{
		DirectX::XMVector3Dot(vTriNormal, DirectX::XMVectorSubtract(vLineStart, vPlanePos)),
		DirectX::XMVector3Dot(vTriNormal, DirectX::XMVectorSubtract(vLineEnd, vPlanePos)),
	};
	// 内積値の比率から交差しているか判定
	float borderDot[2];
	DirectX::XMStoreFloat(&borderDot[0], vBorderDot[0]);
	DirectX::XMStoreFloat(&borderDot[1], vBorderDot[1]);
	if (borderDot[0] * borderDot[1] > 0.0f)
	{
		return false;
	}
	// 交点を計算
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
* @brief 線分と三角形の当たり判定
* @param[in] ray 線分
* @param[in] triangle 三角形
* @param[out] pHit 交点
* @return 衝突しているか
*/
bool CollisionCheckHit(CollisionSegment& segment, CollisionTriangle& triangle, DirectX::XMFLOAT3* pHit)
{
	CollisionPlane plane = { triangle.pos0, triangle.normal };
	DirectX::XMFLOAT3 hitPos;
	if (!CollisionCheckHit(segment, plane, &hitPos))
	{
		return false;
	}

	// 交点が三角形内に含まれるか
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