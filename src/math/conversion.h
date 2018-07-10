#pragma once


#include "constants.h"
#include "common.h"
#include "matrix.h"
#include "quaternion.h"
#include "../essentials/coding.h"


namespace NMaxestFramework { namespace NMath
{
	float DegToRad(float degrees);
	float RadToDeg(float radians);

	SVector3 SphericalToCartesian(const SSpherical& spherical);
	SSpherical CartesianToSpherical(const SVector3& cartesian);

	SVector3 QuaternionToEulerAngles(const SQuaternion& q);
	SMatrix QuaternionToMatrix(const SQuaternion& q);

	//

	inline float DegToRad(float degrees)
	{
		return (degrees * (cPi / 180.0f));
	}

	inline float RadToDeg(float radians)
	{
		return (radians * (180.0f / cPi));
	}

	inline SVector3 SphericalToCartesian(const SSpherical& spherical)
	{
		SVector3 cartesian;

		cartesian.x = sinf(spherical.theta) * cosf(spherical.phi);
		cartesian.y = sinf(spherical.theta) * sinf(spherical.phi);
		cartesian.z = cosf(spherical.theta);

		return cartesian;
	}

	inline SSpherical CartesianToSSpherical(const SVector3& cartesian)
	{
		SSpherical spherical;

		spherical.theta = acosf(cartesian.z / Length(cartesian));
		spherical.phi = atan2f(cartesian.y, cartesian.x);

		return spherical;
	}

	inline SVector3 QuaternionToEulerAngles(const SQuaternion& q)
	{
		SVector3 temp;

		temp.x = ATan2(2.0f*(q.x*q.w + q.y*q.z), 1.0f - 2.0f*(q.x*q.x + q.y*q.y));
		temp.y = ASin(2.0f*(q.y*q.w - q.x*q.z));
		temp.z = ATan2(2.0f*(q.z*q.w + q.x*q.y), 1.0f - 2.0f*(q.y*q.y + q.z*q.z));

		return temp;
	}

	inline SMatrix QuaternionToMatrix(const SQuaternion& q)
	{
		return MatrixRotate(Axis(q), Angle(q));
	}
} }
