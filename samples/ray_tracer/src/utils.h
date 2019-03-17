#pragma once


#include "types.h"

#include "../../../src/essentials/common.h"
#include "../../../src/math/main.h"


using namespace NMaxestFramework;
using namespace NEssentials;
using namespace NMath;


namespace NRayTracer
{
	inline float CosTheta(const SVector3& v)
	{
		return v.z;
	}

	inline float SinThetaSqr(const SVector3& v)
	{
		return Max(0.0f, 1.0f - Sqr(CosTheta(v)));
	}

	inline float SinTheta(const SVector3& v)
	{
		return Sqrt(SinThetaSqr(v));
	}

	inline float CosPhi(const SVector3& v)
	{
		float sinTheta = SinTheta(v);
		if (sinTheta == 0.0f)
			return 1.0f;
		return Clamp(v.x / sinTheta, -1.0f, 1.0f);
	}

	inline float SinPhi(const SVector3& v)
	{
		float sinTheta = SinTheta(v);
		if (sinTheta == 0.0f)
			return 1.0f;
		return Clamp(v.y / sinTheta, -1.0f, 1.0f);
	}

	inline SMatrix WorldToTangent(const SVector3& normal)
	{
		SVector3 random = Normalize(VectorCustom(1.0f, 1.0f, 1.0f));
		SVector3 tangent = Normalize(Cross(normal, random));
		SVector3 bitangent = Normalize(Cross(normal, tangent));

		SMatrix worldToTangent;
		worldToTangent.m[0][0] = tangent.x;
		worldToTangent.m[1][0] = tangent.y;
		worldToTangent.m[2][0] = tangent.z;
		worldToTangent.m[3][0] = 0.0f;
		worldToTangent.m[0][1] = bitangent.x;
		worldToTangent.m[1][1] = bitangent.y;
		worldToTangent.m[2][1] = bitangent.z;
		worldToTangent.m[3][1] = 0.0f;
		worldToTangent.m[0][2] = normal.x;
		worldToTangent.m[1][2] = normal.y;
		worldToTangent.m[2][2] = normal.z;
		worldToTangent.m[3][2] = 0.0f;
		worldToTangent.m[0][3] = 0.0f;
		worldToTangent.m[1][3] = 0.0f;
		worldToTangent.m[2][3] = 0.0f;
		worldToTangent.m[3][3] = 1.0f;

		return worldToTangent;
	}
}
