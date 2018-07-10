#pragma once


#include "../essentials/main.h"


namespace NMaxestFramework { namespace NMath
{
	float IntToNormalizedFloat_Inclusive(int i, int max_inclusive = 255);
	int NormalizedFloatToInt_Inclusive(float f, int max_inclusive = 255);
	float IntToNormalizedFloat(int i, int max_inclusive = 255);
	int NormalizedFloatToInt(float f, int max_inclusive = 255);

	uint32 Vector4ToUInt32(const SVector4& v);

	SVector3 RGBToLumaChroma(const SVector3& rgb);
	SVector3 LumaChromaToRGB(SVector3 lumaChroma);

	//

	inline float IntToNormalizedFloat_Inclusive(int i, int max_inclusive)
	{
		return (float)i / (float)max_inclusive;
	}

	inline int NormalizedFloatToInt_Inclusive(float f, int max_inclusive)
	{
		return (int)(f * max_inclusive);
	}

	inline float IntToNormalizedFloat(int i, int max_inclusive)
	{
		int n = max_inclusive + 1;
		float half = 0.5f / (float)n;
		return half + i*2.0f*half;
	}

	inline int NormalizedFloatToInt(float f, int max_inclusive)
	{
		int n = max_inclusive + 1;
		float f_unorm = f * n;
		return NEssentials::Min((int)f_unorm, max_inclusive);
	}

	inline uint32 Vector4ToUInt32(const SVector4& v)
	{
		return NEssentials::Encode_R8G8G8A8UInt_In_R32UInt(
			(uint8)(255.0f * Saturate(v.x)),
			(uint8)(255.0f * Saturate(v.y)),
			(uint8)(255.0f * Saturate(v.z)),
			(uint8)(255.0f * Saturate(v.w)));
	}

	inline SVector3 RGBToLumaChroma(const SVector3& rgb)
	{
		SMatrix matrix;

		matrix.m[0][0] = 0.299f;
		matrix.m[0][1] = 0.587f;
		matrix.m[0][2] = 0.114f;
		matrix.m[1][0] = -0.168736f;
		matrix.m[1][1] = -0.331264f;
		matrix.m[1][2] = 0.5f;
		matrix.m[2][0] = 0.5f;
		matrix.m[2][1] = -0.418688f;
		matrix.m[2][2] = -0.081312f;

		SVector3 result = matrix * rgb;
		result.y += 0.5f;
		result.z += 0.5f;

		return result;
	}


	inline SVector3 LumaChromaToRGB(SVector3 lumaChroma)
	{
		lumaChroma.y -= 0.5f;
		lumaChroma.z -= 0.5f;

		SMatrix matrix;

		matrix.m[0][0] = 1.0f;
		matrix.m[0][1] = 0.0f;
		matrix.m[0][2] = 1.402f;
		matrix.m[1][0] = 1.0f;
		matrix.m[1][1] = -0.344136f;
		matrix.m[1][2] = -0.714136f;
		matrix.m[2][0] = 1.0f;
		matrix.m[2][1] = 1.772f;
		matrix.m[2][2] = 0.0f;

		return matrix * lumaChroma;
	}
} }
