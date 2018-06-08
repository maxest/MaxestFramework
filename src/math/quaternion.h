#pragma once


#include "types.h"
#include "common.h"
#include "vector.h"


namespace NMaxestFramework { namespace NMath
{
	SQuaternion SQuaternionCustom(float x, float y, float z, float w);
	SQuaternion SQuaternionCopy(const SQuaternion& q);

	SQuaternion Add(const SQuaternion& q1, const SQuaternion& q2);
	SQuaternion Mul(const SQuaternion& q1, const SQuaternion& q2);
	SQuaternion Mul(const SQuaternion& q, float f);
	void AddIn(SQuaternion& q1, const SQuaternion& q2);
	void MulIn(SQuaternion& q1, const SQuaternion& q2);
	void MulIn(SQuaternion& q, float f);
	SQuaternion Conjugate(const SQuaternion& q);
	float Dot(const SQuaternion& q1, const SQuaternion& q2);
	float LengthSquared(const SQuaternion& q);
	float Length(const SQuaternion& q);
	SQuaternion Normalize(const SQuaternion& q);
	void NormalizeIn(SQuaternion& q);
	SQuaternion Pow(const SQuaternion& q, float p); // it's simply a multiplication of angle by p
	void PowIn(SQuaternion& q, float p);
	float Angle(const SQuaternion& q);
	SVector3 Axis(const SQuaternion& q);
	SQuaternion Slerp(const SQuaternion& q1, const SQuaternion& q2, float t);
	SQuaternion operator + (const SQuaternion& q1, const SQuaternion& q2);
	SQuaternion operator * (const SQuaternion& q1, const SQuaternion& q2);
	SQuaternion operator * (const SQuaternion& q, float f);
	SQuaternion operator * (float f, const SQuaternion& q);
	void operator += (SQuaternion& q1, const SQuaternion& q2);
	void operator *= (SQuaternion& q1, const SQuaternion& q2);
	void operator *= (SQuaternion& q, float f);

	void SetIdentity(SQuaternion& q);
	void SetRotate(SQuaternion& q, float x, float y, float z, float angle);
	void SetRotate(SQuaternion& q, const SVector3& axis, float angle);
	void SetRotateX(SQuaternion& q, float angle);
	void SetRotateY(SQuaternion& q, float angle);
	void SetRotateZ(SQuaternion& q, float angle);

	SQuaternion SQuaternionIdentity();
	SQuaternion SQuaternionRotate(float x, float y, float z, float angle);
	SQuaternion SQuaternionRotate(const SVector3& axis, float angle);
	SQuaternion SQuaternionRotateX(float angle);
	SQuaternion SQuaternionRotateY(float angle);
	SQuaternion SQuaternionRotateZ(float angle);

	//

	const SQuaternion cQuaternionZero = SQuaternionCustom(0.0f, 0.0f, 0.0f, 0.0f);
	const SQuaternion cQuaternionIdentity = SQuaternionIdentity();

	//

	inline SQuaternion SQuaternionCustom(float x, float y, float z, float w)
	{
		SQuaternion temp;

		temp.x = x;
		temp.y = y;
		temp.z = z;
		temp.w = w;

		return temp;
	}

	inline SQuaternion SQuaternionCopy(const SQuaternion& q)
	{
		return SQuaternionCustom(q.x, q.y, q.z, q.w);
	}

	inline SQuaternion Add(const SQuaternion& q1, const SQuaternion& q2)
	{
		SQuaternion temp;

		temp.x = q1.x + q2.x;
		temp.y = q1.y + q2.y;
		temp.z = q1.z + q2.z;
		temp.w = q1.w + q2.w;

		return temp;
	}

	inline SQuaternion Mul(const SQuaternion& q1, const SQuaternion& q2)
	{
		SQuaternion temp;

		temp.x = q1.w*q2.x + q1.x*q2.w + q1.z*q2.y - q1.y*q2.z;
		temp.y = q1.w*q2.y + q1.y*q2.w + q1.x*q2.z - q1.z*q2.x;
		temp.z = q1.w*q2.z + q1.z*q2.w + q1.y*q2.x - q1.x*q2.y;
		temp.w = q1.w*q2.w - q1.x*q2.x - q1.y*q2.y - q1.z*q2.z;

		return temp;
	}
	
	inline SQuaternion Mul(const SQuaternion& q, float f)
	{
		SQuaternion temp;

		temp.x = f * q.x;
		temp.y = f * q.y;
		temp.z = f * q.z;
		temp.w = f * q.w;

		return temp;
	}

	inline void MulIn(SQuaternion& q1, const SQuaternion& q2)
	{
		q1 = Mul(q1, q2);
	}
	
	inline void MulIn(SQuaternion& q, float f)
	{
		q = Mul(q, f);
	}

	inline void AddIn(SQuaternion& q1, const SQuaternion& q2)
	{
		q1 = Add(q1, q2);
	}

	inline SQuaternion Conjugate(const SQuaternion& q)
	{
		SQuaternion temp;

		temp.x = -q.x;
		temp.y = -q.y;
		temp.z = -q.z;
		temp.w = q.w;

		return temp;
	}

	inline float Dot(const SQuaternion& q1, const SQuaternion& q2)
	{
		return q1.x*q2.x + q1.y*q2.y + q1.z*q2.z + q1.w*q2.w;
	}

	inline float LengthSquared(const SQuaternion& q)
	{
		return Dot(q, q);
	}

	inline float Length(const SQuaternion& q)
	{
		return Sqrt(LengthSquared(q));
	}

	inline SQuaternion Normalize(const SQuaternion& q)
	{
		SQuaternion temp = SQuaternionCopy(q);
		NormalizeIn(temp);
		return temp;
	}

	inline void NormalizeIn(SQuaternion& q)
	{
		float oneOverLength = 1.0f / Length(q);

		q.x *= oneOverLength;
		q.y *= oneOverLength;
		q.z *= oneOverLength;
		q.w *= oneOverLength;
	}

	inline SQuaternion Pow(const SQuaternion& q, float p)
	{
		float angleOver2 = ACos(q.w);
		float newAngleOver2 = angleOver2 * p;
		float ratio = Sin(newAngleOver2) / Sin(angleOver2);

		SQuaternion temp;

		temp.x = q.x * ratio;
		temp.y = q.y * ratio;
		temp.z = q.z * ratio;
		temp.w = Cos(newAngleOver2);

		return temp;
	}

	inline void PowIn(SQuaternion& q, float p)
	{
		q = Pow(q, p);
	}

	inline float Angle(const SQuaternion& q)
	{
		return 2.0f * ACos_Clamped(q.w);
	}

	inline SVector3 Axis(const SQuaternion& q)
	{
		float squaredSinAngleOver2 = 1.0f - q.w*q.w;

		if (squaredSinAngleOver2 <= 0.0f)
		{
			return cVector3EX;
		}
		else
		{
			float oneOverSinAngleOver2 = 1.0f / (Sqrt(squaredSinAngleOver2));
			return VectorCustom(oneOverSinAngleOver2*q.x, oneOverSinAngleOver2*q.y, oneOverSinAngleOver2*q.z);
		}
	}

	inline SQuaternion Slerp(const SQuaternion& q1, const SQuaternion& q2, float t)
	{
		// taken from UE4

		float dot = Dot(q1, q2);
		float dot_shortest = (dot >= 0.0f ? dot : -dot);

		float scale1, scale2;

		if (dot_shortest < 0.9999f)
		{	
			float omega = ACos(dot_shortest);
			float oneOverSin = 1.0f / Sin(dot_shortest);
			scale1 = Sin((1.0f - t) * omega) * oneOverSin;
			scale2 = Sin(t * omega) * oneOverSin;
		}
		else
		{
			// linear interpolation
			scale1 = 1.0f - t;
			scale2 = t;	
		}

		scale2 = (dot >= 0.0f ? scale2 : -scale2);

		SQuaternion result;
		
		result.x = scale1*q1.x + scale2*q2.x;
		result.y = scale1*q1.y + scale2*q2.y;
		result.z = scale1*q1.z + scale2*q2.z;
		result.w = scale1*q1.w + scale2*q2.w;

		return result;
	}

	inline SQuaternion operator + (const SQuaternion& q1, const SQuaternion& q2)
	{
		return Add(q1, q2);
	}

	inline SQuaternion operator * (const SQuaternion& q1, const SQuaternion& q2)
	{
		return Mul(q1, q2);
	}
	
	inline SQuaternion operator * (const SQuaternion& q, float f)
	{
		return Mul(q, f);
	}

	inline SQuaternion operator * (float f, const SQuaternion& q)
	{
		return Mul(q, f);
	}

	inline void operator += (SQuaternion& q1, const SQuaternion& q2)
	{
		AddIn(q1, q2);
	}

	inline void operator *= (SQuaternion& q1, const SQuaternion& q2)
	{
		MulIn(q1, q2);
	}

	inline void operator *= (SQuaternion& q, float f)
	{
		MulIn(q, f);
	}

	inline void SetIdentity(SQuaternion& q)
	{
		q.x = 0.0f;
		q.y = 0.0f;
		q.z = 0.0f;
		q.w = 1.0f;
	}

	inline void SetRotate(SQuaternion& q, float x, float y, float z, float angle)
	{
		angle = 0.5f * angle;
		float c = Cos(angle);
		float s = Sin(angle);

		q.x = x * s;
		q.y = y * s;
		q.z = z * s;
		q.w = c;
	}

	inline void SetRotate(SQuaternion& q, const SVector3& axis, float angle)
	{
		SetRotate(q, axis.x, axis.y, axis.z, angle);
	}

	inline void SetRotateX(SQuaternion& q, float angle)
	{
		SetRotate(q, 1.0f, 0.0f, 0.0f, angle);
	}

	inline void SetRotateY(SQuaternion& q, float angle)
	{
		SetRotate(q, 0.0f, 1.0f, 0.0f, angle);
	}


	inline void SetRotateZ(SQuaternion& q, float angle)
	{
		SetRotate(q, 0.0f, 0.0f, 1.0f, angle);
	}

	inline SQuaternion SQuaternionIdentity()
	{
		SQuaternion temp;
		SetIdentity(temp);
		return temp;
	}

	inline SQuaternion SQuaternionRotate(float x, float y, float z, float angle)
	{
		SQuaternion temp;
		SetRotate(temp, x, y, z, angle);
		return temp;
	}

	inline SQuaternion SQuaternionRotate(const SVector3& axis, float angle)
	{
		SQuaternion temp;
		SetRotate(temp, axis, angle);
		return temp;
	}

	inline SQuaternion SQuaternionRotateX(float angle)
	{
		SQuaternion temp;
		SetRotateX(temp, angle);
		return temp;
	}

	inline SQuaternion SQuaternionRotateY(float angle)
	{
		SQuaternion temp;
		SetRotateY(temp, angle);
		return temp;
	}

	inline SQuaternion SQuaternionRotateZ(float angle)
	{
		SQuaternion temp;
		SetRotateZ(temp, angle);
		return temp;
	}
} }
