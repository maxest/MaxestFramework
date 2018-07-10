#pragma once


#include "types.h"
#include "vector.h"


namespace NMaxestFramework { namespace NMath
{
	float Distance(const SVector2& v1, const SVector2& v2);
	float Distance(const SVector3& v1, const SVector3& v2);
	float DistanceSquared(const SVector2& v1, const SVector2& v2);
	float DistanceSquared(const SVector3& v1, const SVector3& v2);
	double DistanceSquared(const SVector3d& v1, const SVector3d& v2);
	float DistanceSigned(const SPlane& plane, const SVector3& point);
	float Distance_RectWarped(const SVector2& v1, const SVector2& v2, float rectSize);

	//

	inline float Distance(const SVector2& v1, const SVector2& v2)
	{
		return Length(Sub(v1, v2));
	}

	inline float Distance(const SVector3& v1, const SVector3& v2)
	{
		return Length(Sub(v1, v2));
	}

	inline float DistanceSquared(const SVector2& v1, const SVector2& v2)
	{
		return LengthSquared(Sub(v1, v2));
	}

	inline float DistanceSquared(const SVector3& v1, const SVector3& v2)
	{
		return LengthSquared(Sub(v1, v2));
	}

	inline double DistanceSquared(const SVector3d& v1, const SVector3d& v2)
	{
		double dx = v1.x - v2.x;
		double dy = v1.y - v2.y;
		double dz = v1.z - v2.z;

		dx *= dx;
		dy *= dy;
		dz *= dz;

		return dx + dy + dz;
	}

	inline float DistanceSigned(const SPlane& plane, const SVector3& point)
	{
		return plane.a*point.x + plane.b*point.y + plane.c*point.z + plane.d;
	}

	inline float Distance_RectWarped(const SVector2& v1, const SVector2& v2, float rectSize)
	{
		float dx = NEssentials::Abs(v2.x - v1.x);
		float dy = NEssentials::Abs(v2.y - v1.y);

		if (dx > rectSize / 2.0f)
			dx = rectSize - dx;
		if (dy > rectSize / 2.0f)
			dy = rectSize - dy;

		return Sqrt(dx*dx + dy*dy);
	}
} }
