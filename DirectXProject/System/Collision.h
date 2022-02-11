#ifndef __COLLISION_H__
#define __COLLISION_H__

#include <DirectXMath.h>

using CollisionPoint = DirectX::XMFLOAT3;
struct CollisionRay // ÉåÉC
{
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT3 direction;
};
struct CollisionSegment // ê¸ï™
{
	DirectX::XMFLOAT3 start;
	DirectX::XMFLOAT3 end;
};
struct CollisionTriangle // éOäpå`
{
	DirectX::XMFLOAT3 pos0;
	DirectX::XMFLOAT3 pos1;
	DirectX::XMFLOAT3 pos2;
	DirectX::XMFLOAT3 normal;
};
struct CollisionPlane // ïΩñ 
{
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT3 normal;
};

// ç≈íZãóó£
float CollisionCheckDistance(CollisionPoint& point, CollisionSegment& segment, DirectX::XMFLOAT3* pPos);
float CollisionCheckDistance(CollisionRay& rayA, CollisionRay& rayB, DirectX::XMFLOAT3 *pPosA, DirectX::XMFLOAT3 *pPosB);
float CollisionCheckDistance(CollisionSegment& segmentA, CollisionSegment& segmentB, DirectX::XMFLOAT3* pPosA, DirectX::XMFLOAT3* pPosB);
// è’ìÀîªíË
bool CollisionCheckHit(CollisionSegment& segment, CollisionPlane& plane, DirectX::XMFLOAT3* pHit);
bool CollisionCheckHit(CollisionSegment& segment, CollisionTriangle& triangle, DirectX::XMFLOAT3* pHit);

#endif