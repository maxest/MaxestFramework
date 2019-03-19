#pragma once


#include "../../../src/essentials/stl.h"
#include "../../../src/essentials/string.h"
#include "../../../src/essentials/coding.h"
#include "../../../src/math/main.h"


using namespace NMaxestFramework;
using namespace NMath;


//#define USE_SAMPLES_HALF_PRECISION // remember to remove cached samples file


namespace NRayTracer
{
	struct SPrimitive
	{
		int32 materialIndex;
	};
	struct STrianglePrimitive: public SPrimitive
	{
		SVector3 p1;
		SVector3 p2;
		SVector3 p3;
		SVector3 normal;
	};
	struct SSpherePrimitive: public SPrimitive
	{
		SVector3 position;
		float radius;
	};

	struct SLight
	{
		SVector3 color;
	};
	struct SDirLight: public SLight
	{
		SVector3 dir;
	};
	struct SPointLight: public SLight
	{
		SVector3 position;
	};

	struct SSceneIntersectionResult
	{
		int32 materialIndex;
		SVector3 point;
		SVector3 normal;
		bool backside;
		int32 triangleIndex;
	};

	struct SHemisphericalCartesian
	{
	#ifndef USE_SAMPLES_HALF_PRECISION
		float x, y;

		void Set(const SVector3& v)
		{
			x = v.x;
			y = v.y;
		}

		SVector3 Get() const
		{
			SVector3 temp;

			temp.x = x;
			temp.y = y;
			temp.z = Sqrt(1.0f - (x*x + y*y));

			return temp;
		}
	#else
		uint16 x, y;

		void Set(const SVector3& v)
		{
			x = NEssentials::FloatToHalf(v.x);
			y = NEssentials::FloatToHalf(v.y);
		}

		SVector3 Get() const
		{
			SVector3 temp;

			temp.x = NEssentials::HalfToFloat(x);
			temp.y = NEssentials::HalfToFloat(y);
			temp.z = Sqrt(1.0f - (temp.x*temp.x + temp.y*temp.y));

			return temp;
		}
	#endif
	};
}
