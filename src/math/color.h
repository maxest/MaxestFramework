#pragma once


#include "../essentials/main.h"


namespace NMaxestFramework { namespace NMath
{
	float IntToNormalizedFloat_Inclusive(int i, int max_inclusive = 255);
	int NormalizedFloatToInt_Inclusive(float f, int max_inclusive = 255);
	float IntToNormalizedFloat(int i, int max_inclusive = 255);
	int NormalizedFloatToInt(float f, int max_inclusive = 255);

	uint32 Vector4ToUInt32(const SVector4& v);

	SVector3 RGBToLumaChroma(SVector3 value);
	SVector3 LumaChromaToRGB(SVector3 value);
	SVector3 RGBToLumaChroma_BT601(SVector3 value);
	SVector3 LumaChromaToRGB_BT601(SVector3 value);
	SVector3 RGBToLumaChroma_BT709(SVector3 value);
	SVector3 LumaChromaToRGB_BT709(SVector3 value);
	SVector3 RGBToLumaChroma_BT2020(SVector3 value);
	SVector3 LumaChromaToRGB_BT2020(SVector3 value);

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
			(uint8)(255.0f * NEssentials::Saturate(v.x)),
			(uint8)(255.0f * NEssentials::Saturate(v.y)),
			(uint8)(255.0f * NEssentials::Saturate(v.z)),
			(uint8)(255.0f * NEssentials::Saturate(v.w)));
	}

	inline SVector3 RGBToLumaChroma(SVector3 value)
	{
		SMatrix transform;
		transform.m[0][0] = 0.299f;
		transform.m[0][1] = 0.587f;
		transform.m[0][2] = 0.114f;
		transform.m[1][0] = -0.168736f;
		transform.m[1][1] = -0.331264f;
		transform.m[1][2] = 0.5f;
		transform.m[2][0] = 0.5f;
		transform.m[2][1] = -0.418688f;
		transform.m[2][2] = -0.081312f;

		value = transform * value;
		value.y += 0.5f;
		value.z += 0.5f;

		return value;
	}

	inline SVector3 LumaChromaToRGB(SVector3 value)
	{
		SMatrix transform;
		transform.m[0][0] = 1.0f;
		transform.m[0][1] = 0.0f;
		transform.m[0][2] = 1.402f;
		transform.m[1][0] = 1.0f;
		transform.m[1][1] = -0.344136f;
		transform.m[1][2] = -0.714136f;
		transform.m[2][0] = 1.0f;
		transform.m[2][1] = 1.772f;
		transform.m[2][2] = 0.0f;

		value.y -= 0.5f;
		value.z -= 0.5f;

		return transform * value;
	}

	inline SVector3 RGBToLumaChroma_BT601(SVector3 value)
	{
		SMatrix transform;
		transform.m[0][0] = 0.256788254f;
		transform.m[0][1] = 0.504129469f;
		transform.m[0][2] = 0.0979058892f;
		transform.m[1][0] = -0.144914359f;
		transform.m[1][1] = -0.284497440f;
		transform.m[1][2] = 0.429411799f;
		transform.m[2][0] = 0.429411799f;
		transform.m[2][1] = -0.359578788f;
		transform.m[2][2] = -0.0698330179f;

		value = transform * value;
		value.x += 16.0f / 255.0f;
		value.y += 128.0f / 255.0f;
		value.z += 128.0f / 255.0f;

		return value;
	}

	inline SVector3 LumaChromaToRGB_BT601(SVector3 value)
	{
		SMatrix transform;
		transform.m[0][0] = 1.16438353f;
		transform.m[0][1] = 0.0f;
		transform.m[0][2] = 1.63246572f;
		transform.m[1][0] = 1.16438353f;
		transform.m[1][1] = -0.400706619f;
		transform.m[1][2] = -0.831528485f;
		transform.m[2][0] = 1.16438353f;
		transform.m[2][1] = 2.06328773f;
		transform.m[2][2] = 0.0f;

		value.x -= 16.0f / 255.0f;
		value.y -= 128.0f / 255.0f;
		value.z -= 128.0f / 255.0f;

		return transform * value;
	}

	inline SVector3 RGBToLumaChroma_BT709(SVector3 value)
	{
		SMatrix transform;
		transform.m[0][0] = 0.182585895f;
		transform.m[0][1] = 0.614230633f;
		transform.m[0][2] = 0.0620070547f;
		transform.m[1][0] = -0.0983972251f;
		transform.m[1][1] = -0.331014544f;
		transform.m[1][2] = 0.429411799f;
		transform.m[2][0] = 0.429411799f;
		transform.m[2][1] = -0.390037209f;
		transform.m[2][2] = -0.0393745564f;

		value = transform * value;
		value.x += 16.0f / 255.0f;
		value.y += 128.0f / 255.0f;
		value.z += 128.0f / 255.0f;

		return value;
	}

	inline SVector3 LumaChromaToRGB_BT709(SVector3 value)
	{
		SMatrix transform;
		transform.m[0][0] = 1.16438353f;
		transform.m[0][1] = 0.0f;
		transform.m[0][2] = 1.83367121f;
		transform.m[1][0] = 1.16438353f;
		transform.m[1][1] = -0.218117282f;
		transform.m[1][2] = -0.545076191f;
		transform.m[2][0] = 1.16438353f;
		transform.m[2][1] = 2.16063023f;
		transform.m[2][2] = 0.0f;

		value.x -= 16.0f / 255.0f;
		value.y -= 128.0f / 255.0f;
		value.z -= 128.0f / 255.0f;

		return transform * value;
	}

	inline SVector3 RGBToLumaChroma_BT2020(SVector3 value)
	{
		SMatrix transform;
		transform.m[0][0] = 0.224735081f;
		transform.m[0][1] = 0.580016613f;
		transform.m[0][2] = 0.0507300682f;
		transform.m[1][0] = -0.119450986f;
		transform.m[1][1] = -0.308289915f;
		transform.m[1][2] = 0.427740902f;
		transform.m[2][0] = 0.427740872f;
		transform.m[2][1] = -0.393338263f;
		transform.m[2][2] = -0.0344025977f;

		value = transform * value;
		value.x += 16.0f / 255.0f;
		value.y += 128.0f / 255.0f;
		value.z += 128.0f / 255.0f;

		return value;
	}

	inline SVector3 LumaChromaToRGB_BT2020(SVector3 value)
	{
		SMatrix transform;
		transform.m[0][0] = 1.16893196f;
		transform.m[0][1] = 0.0f;
		transform.m[0][2] = 1.72370708f;
		transform.m[1][0] = 1.16893196f;
		transform.m[1][1] = -0.192351416f;
		transform.m[1][2] = -0.667873025f;
		transform.m[2][0] = 1.16893196f;
		transform.m[2][1] = 2.19922853f;
		transform.m[2][2] = 0.0f;

		value.x -= 16.0f / 255.0f;
		value.y -= 128.0f / 255.0f;
		value.z -= 128.0f / 255.0f;

		return transform * value;
	}
} }
