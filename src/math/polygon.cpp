#include "polygon.h"
#include "plane.h"
#include "distance.h"


using namespace NMaxestFramework;


// algorithm comes from Fernando's and Kilgard's "The Cg Tutorial"
void NMath::TriangleTangentBasis(
	const SVector3& v1, const SVector2& uv1,
	const SVector3& v2, const SVector2& uv2,
	const SVector3& v3, const SVector2& uv3,
	SVector3& tangent, SVector3& bitangent, SVector3& normal)
{
	SVector3 delta1_xuv = VectorCustom(v2.x - v1.x, uv2.x - uv1.x, uv2.y - uv1.y);
	SVector3 delta2_xuv = VectorCustom(v3.x - v1.x, uv3.x - uv1.x, uv3.y - uv1.y);
	SVector3 cross_xuv = Cross(delta1_xuv, delta2_xuv);

	SVector3 delta1_yuv = VectorCustom(v2.y - v1.y, uv2.x - uv1.x, uv2.y - uv1.y);
	SVector3 delta2_yuv = VectorCustom(v3.y - v1.y, uv3.x - uv1.x, uv3.y - uv1.y);
	SVector3 cross_yuv = Cross(delta1_yuv, delta2_yuv);

	SVector3 delta1_zuv = VectorCustom(v2.z - v1.z, uv2.x - uv1.x, uv2.y - uv1.y);
	SVector3 delta2_zuv = VectorCustom(v3.z - v1.z, uv3.x - uv1.x, uv3.y - uv1.y);
	SVector3 cross_zuv = Cross(delta1_zuv, delta2_zuv);

	tangent.x = -cross_xuv.y / cross_xuv.x;
	tangent.y = -cross_yuv.y / cross_yuv.x;
	tangent.z = -cross_zuv.y / cross_zuv.x;

	bitangent.x = -cross_xuv.z / cross_xuv.x;
	bitangent.y = -cross_yuv.z / cross_yuv.x;
	bitangent.z = -cross_zuv.z / cross_zuv.x;

	normal = TriangleNormal(v1, v2, v3);
}


bool NMath::IsPointInsideTriangle(
	const SVector3& point,
	const SVector3& v1,
	const SVector3& v2,
	const SVector3& v3)
{
	SVector3 triangleNormal = TriangleNormal(v1, v2, v3);

	SPlane plane1 = PlaneFromPoints(v1, v2, v2 + triangleNormal);
	SPlane plane2 = PlaneFromPoints(v2, v3, v3 + triangleNormal);
	SPlane plane3 = PlaneFromPoints(v3, v1, v1 + triangleNormal);

	if (DistanceSigned(plane1, point) <= 0.0f &&
		DistanceSigned(plane2, point) <= 0.0f &&
		DistanceSigned(plane3, point) <= 0.0f)
	{
		return true;
	}
	else
	{
		return false;
	}
}


bool NMath::DoesTriangleOverlapTriangle(
	const SVector3& t1_v1,
	const SVector3& t1_v2,
	const SVector3& t1_v3,
	const SVector3& t2_v1,
	const SVector3& t2_v2,
	const SVector3& t2_v3)
{
	if (IsPointInsideTriangle(t1_v1, t2_v1, t2_v2, t2_v3) ||
		IsPointInsideTriangle(t1_v2, t2_v1, t2_v2, t2_v3) ||
		IsPointInsideTriangle(t1_v3, t2_v1, t2_v2, t2_v3) ||
		IsPointInsideTriangle(t2_v1, t1_v1, t1_v2, t1_v3) ||
		IsPointInsideTriangle(t2_v2, t1_v1, t1_v2, t1_v3) ||
		IsPointInsideTriangle(t2_v3, t1_v1, t1_v2, t1_v3))
	{
		return true;
	}
	else
	{
		return false;
	}
}


// algorithm comes from "3D Math Primer for Graphics and Game Development", chapter 12.
void NMath::TriangleBarycentricWeightsForPoint(
	const SVector3& point,
	const SVector3& v1,
	const SVector3& v2,
	const SVector3& v3,
	float& w1, float& w2, float& w3)
{
	SVector3 e1 = v3 - v2;
	SVector3 e2 = v1 - v3;
	SVector3 e3 = v2 - v1;

	SVector3 d1 = point - v1;
	SVector3 d2 = point - v2;
	SVector3 d3 = point - v3;

	SVector3 normal = TriangleNormal(v1, v2, v3);

	float a_t = 0.5f * Dot(Cross(e1, e2), normal);
	float a_t1 = 0.5f * Dot(Cross(e1, d3), normal);
	float a_t2 = 0.5f * Dot(Cross(e2, d1), normal);
	float a_t3 = 0.5f * Dot(Cross(e3, d2), normal);

	w1 = a_t1 / a_t;
	w2 = a_t2 / a_t;
	w3 = a_t3 / a_t;
}


bool NMath::DoesQuadOverlapQuad(
	const SVector3& q1_v1,
	const SVector3& q1_v2,
	const SVector3& q1_v3,
	const SVector3& q1_v4,
	const SVector3& q2_v1,
	const SVector3& q2_v2,
	const SVector3& q2_v3,
	const SVector3& q2_v4)
{
	if (DoesTriangleOverlapTriangle(q1_v1, q1_v2, q1_v3, q2_v1, q2_v2, q2_v3) ||
		DoesTriangleOverlapTriangle(q1_v1, q1_v2, q1_v3, q2_v1, q2_v3, q2_v4) ||
		DoesTriangleOverlapTriangle(q1_v1, q1_v3, q1_v4, q2_v1, q2_v2, q2_v3) ||
		DoesTriangleOverlapTriangle(q1_v1, q1_v3, q1_v4, q2_v1, q2_v3, q2_v4))
	{
		return true;
	}
	else
	{
		return false;
	}
}
