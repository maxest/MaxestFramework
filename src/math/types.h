#pragma once

#include "../essentials/types.h"


namespace NMaxestFramework { namespace NMath
{
	struct SVector2
	{
		float x, y;
	};

	struct SVector2d
	{
		double x, y;
	};

	struct SVector2ui
	{
		uint32 x, y;
	};

	struct SVector2ui16
	{
		uint8 x, y;
	};

	struct SVector2i32
	{
		int16 x, y;
	};

	struct SVector2i
	{
		int x, y;
	};

	struct SVector3
	{
		float x, y, z;
	};

	struct SVector3d
	{
		double x, y, z;
	};

	struct SVector3ui16
	{
		uint16 data;

		void Set(uint8 x, uint8 y, uint8 z)
		{
			data = z;
			data |= (y << 5);
			data |= (x << 11);
		}

		void Get(uint8& x, uint8& y, uint8& z)
		{
			z = (data & 31);
			y = ((data >> 5) & 63);
			x = ((data >> 11) & 31);
		}
	};

	struct SVector3ui24
	{
		uint8 x, y, z;
	};

	struct SVector4
	{
		float x, y, z, w;
	};

	struct SVector4d
	{
		double x, y, z, w;
	};

	struct SVector4ui32
	{
		uint8 x, y, z, w;
	};

	struct SVector4i32
	{
		int8 x, y, z, w;
	};

	struct SVector4i
	{
		int x, y, z, w;
	};

	struct SQuaternion
	{
		float x, y, z, w;
	};

	struct SPlane
	{
		float a, b, c, d;
	};

	struct SMatrix
	{
		float m[4][4];
	};

	struct SSpherical
	{
		float theta, phi; // theta in [0..Pi], phi in [0..2*Pi]
	};

	struct SAABB
	{
		SVector3 min, max;
	};

	enum class EZRange { ZeroToOne, MinusOneToPlusOne };
} }
