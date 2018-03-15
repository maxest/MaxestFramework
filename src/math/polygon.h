#pragma once


#include "types.h"
#include "vector.h"


namespace NMaxestFramework { namespace NMath
{
	SVector3 TriangleNormal(const SVector3& v1, const SVector3& v2, const SVector3& v3);
	void TriangleTangentBasis(
		const SVector3& v1, const SVector2& uv1,
		const SVector3& v2, const SVector2& uv2,
		const SVector3& v3, const SVector2& uv3,
		SVector3& tangent, SVector3& bitangent, SVector3& normal);
	bool IsPointInsideTriangle(
		const SVector3& point,
		const SVector3& v1,
		const SVector3& v2,
		const SVector3& v3);
	bool DoesTriangleOverlapTriangle(
		const SVector3& t1_v1,
		const SVector3& t1_v2,
		const SVector3& t1_v3,
		const SVector3& t2_v1,
		const SVector3& t2_v2,
		const SVector3& t2_v3);
	void TriangleBarycentricWeightsForPoint(
		const SVector3& point,
		const SVector3& v1,
		const SVector3& v2,
		const SVector3& v3,
		float& w1, float& w2, float& w3);

	bool DoesQuadOverlapQuad(
		const SVector3& q1_v1,
		const SVector3& q1_v2,
		const SVector3& q1_v3,
		const SVector3& q1_v4,
		const SVector3& q2_v1,
		const SVector3& q2_v2,
		const SVector3& q2_v3,
		const SVector3& q2_v4);

	//

	inline SVector3 TriangleNormal(const SVector3& v1, const SVector3& v2, const SVector3& v3)
	{
		return Normalize(Cross((v2 - v1), (v3 - v1)));
	}
} }
