#pragma once


#include "types.h"
#include "vector.h"
#include "quaternion.h"


namespace NMaxestFramework { namespace NMath
{
	SAABB AABB();
	void AABBAddVertex(SAABB& aabb, const SVector3& v);
	SAABB AABBFromVertices(const SVector3* vertices, int verticesCount);
	void AABBExtractCorners(const SAABB& aabb, SVector3 vertices[8]);
	SAABB AABBMerge(const SAABB& aabb1, const SAABB& aabb2);

	//

	inline SAABB AABB()
	{
		SAABB aabb;
		aabb.min = cVector3Max;
		aabb.max = cVector3Min;
		return aabb;
	}

	inline void AABBAddVertex(SAABB& aabb, const SVector3& v)
	{
		aabb.min.x = NEssentials::Min(aabb.min.x, v.x);
		aabb.min.y = NEssentials::Min(aabb.min.y, v.y);
		aabb.min.z = NEssentials::Min(aabb.min.z, v.z);

		aabb.max.x = NEssentials::Max(aabb.max.x, v.x);
		aabb.max.y = NEssentials::Max(aabb.max.y, v.y);
		aabb.max.z = NEssentials::Max(aabb.max.z, v.z);
	}

	inline SAABB AABBFromVertices(const SVector3* vertices, int verticesCount)
	{
		SAABB aabb = AABB();

		for (int i = 0; i < verticesCount; i++)
			AABBAddVertex(aabb, vertices[i]);

		return aabb;
	}

	inline void AABBExtractCorners(const SAABB& aabb, SVector3 vertices[8])
	{
		vertices[0] = VectorCustom(aabb.min.x, aabb.min.y, aabb.min.z);
		vertices[1] = VectorCustom(aabb.max.x, aabb.min.y, aabb.min.z);
		vertices[2] = VectorCustom(aabb.max.x, aabb.max.y, aabb.min.z);
		vertices[3] = VectorCustom(aabb.min.x, aabb.max.y, aabb.min.z);

		vertices[4] = VectorCustom(aabb.min.x, aabb.min.y, aabb.max.z);
		vertices[5] = VectorCustom(aabb.max.x, aabb.min.y, aabb.max.z);
		vertices[6] = VectorCustom(aabb.max.x, aabb.max.y, aabb.max.z);
		vertices[7] = VectorCustom(aabb.min.x, aabb.max.y, aabb.max.z);
	}

	inline SAABB AABBMerge(const SAABB& aabb1, const SAABB& aabb2)
	{
		SAABB aabb;

		aabb.min.x = NEssentials::Min(aabb1.min.x, aabb2.min.x);
		aabb.min.y = NEssentials::Min(aabb1.min.y, aabb2.min.y);
		aabb.min.z = NEssentials::Min(aabb1.min.z, aabb2.min.z);

		aabb.max.x = NEssentials::Max(aabb1.max.x, aabb2.max.x);
		aabb.max.y = NEssentials::Max(aabb1.max.y, aabb2.max.y);
		aabb.max.z = NEssentials::Max(aabb1.max.z, aabb2.max.z);

		return aabb;
	}
} }
