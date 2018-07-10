#pragma once


#include "types.h"
#include "common.h"


namespace NMaxestFramework { namespace NMath
{
	SPlane PlaneFromPointAndNormal(const SVector3& point, const SVector3& normal);
	SPlane PlaneFromPoints(const SVector3& point1, const SVector3& point2, const SVector3& point3);
	SVector2 PlaneSize(float distance, float fovY, float aspect);
	void NormalizeIn(SPlane& plane);

	//

	inline SPlane PlaneFromPointAndNormal(const SVector3& point, const SVector3& normal)
	{
		SPlane temp;

		temp.a = normal.x;
		temp.b = normal.y;
		temp.c = normal.z;
		temp.d = -Dot(point, normal);

		return temp;
	}

	inline SPlane PlaneFromPoints(const SVector3& point1, const SVector3& point2, const SVector3& point3)
	{
		SVector3 v1 = point2 - point1;
		SVector3 v2 = point3 - point1;
		SVector3 normal = Normalize(Cross(v1, v2));

		SPlane temp;

		temp.a = normal.x;
		temp.b = normal.y;
		temp.c = normal.z;
		temp.d = -Dot(point1, normal);

		return temp;
	}

	inline SVector2 PlaneSize(float distance, float fovY, float aspect)
	{
		SVector2 size;

		size.y = 2.0f * distance * Tan(0.5f * fovY);
		size.x = aspect * size.y;

		return size;
	}

	inline void NormalizeIn(SPlane& plane)
	{
		float length = Sqrt(plane.a*plane.a + plane.b*plane.b + plane.c*plane.c);

		plane.a /= length;
		plane.b /= length;
		plane.c /= length;
	}
} }
