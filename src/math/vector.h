#pragma once


#include "types.h"
#include "common.h"
#include "../essentials/common.h"


namespace NMaxestFramework { namespace NMath
{
	// Vector2

	SVector2 VectorCustom(float x, float y);
	SVector2 VectorCopy(const SVector2& v);
	SVector2 Add(const SVector2& v1, const SVector2& v2);
	SVector2 Sub(const SVector2& v1, const SVector2& v2);
	SVector2 Mul(const SVector2& v1, const SVector2& v2);
	SVector2 Mul(const SVector2& v, float f);
	SVector2 Div(const SVector2& v, float f);
	SVector2 Div(float f, const SVector2& v);
	void AddIn(SVector2& v1, const SVector2& v2);
	void SubIn(SVector2& v1, const SVector2& v2);
	void MulIn(SVector2& v1, const SVector2& v2);
	void MulIn(SVector2& v, float f);
	void DivIn(SVector2& v, float f);
	bool Equal(const SVector2& v1, const SVector2& v2);
	float Dot(const SVector2& v1, const SVector2& v2);
	float LengthSquared(const SVector2& v);
	float Length(const SVector2& v);
	void SetLength(SVector2& v, float newLength);
	SVector2 Normalize(const SVector2& v);
	void NormalizeIn(SVector2& v);
	SVector2 Pow(const SVector2& v, float f);
	void PowIn(SVector2& v, float f);
	float Angle(const SVector2& v1, const SVector2& v2);
	float MaxComponent(const SVector2& v);
	SVector2 ProjAOnB(const SVector2& a, SVector2 b);
	SVector2 Reflect(const SVector2& input, const SVector2& normal);
	SVector2 Refract(const SVector2& input, const SVector2& normal, float eta);
	SVector2 Lerp(const SVector2& v1, const SVector2& v2, float t);
	SVector2 Slerp(const SVector2& v1, const SVector2& v2, float t);
	SVector2 MoveTowards(const SVector2& src, const SVector2& dst, float maxDistance);
	SVector2 Abs(const SVector2& v);
	SVector2 Clamp(const SVector2& v, const SVector2& min, const SVector2& max);
	SVector2 Average(const SVector2* points, int pointsCount);
	SVector2 operator + (const SVector2& v1, const SVector2& v2);
	SVector2 operator - (const SVector2& v1, const SVector2& v2);
	SVector2 operator - (const SVector2& v);
	SVector2 operator * (const SVector2& v1, const SVector2& v2);
	SVector2 operator * (const SVector2& v, float f);
	SVector2 operator * (float f, const SVector2& v);
	SVector2 operator / (const SVector2& v, float f);
	SVector2 operator / (float f, const SVector2& v);
	SVector2& operator += (SVector2& v1, const SVector2& v2);
	SVector2& operator -= (SVector2& v1, const SVector2& v2);
	SVector2& operator *= (SVector2& v1, const SVector2& v2);
	SVector2& operator *= (SVector2& v, float f);
	SVector2& operator /= (SVector2& v, float f);
	bool operator == (const SVector2& v1, const SVector2& v2);

	// SVector2ui16

	bool Equal(const SVector2ui16& v1, const SVector2ui16& v2);
	bool operator == (const SVector2ui16& v1, const SVector2ui16& v2);

	// SVector2i32

	bool Equal(const SVector2i32& v1, const SVector2i32& v2);
	bool operator == (const SVector2i32& v1, const SVector2i32& v2);

	// Vector3

	SVector3 VectorCustom(float x, float y, float z);
	SVector3 VectorCopy(const SVector3& v);
	SVector3 Add(const SVector3& v1, const SVector3& v2);
	SVector3 Sub(const SVector3& v1, const SVector3& v2);
	SVector3 Mul(const SVector3& v1, const SVector3& v2);
	SVector3 Mul(const SVector3& v, float f);
	SVector3 Div(const SVector3& v, float f);
	SVector3 Div(float f, const SVector3& v);
	void AddIn(SVector3& v1, const SVector3& v2);
	void SubIn(SVector3& v1, const SVector3& v2);
	void MulIn(SVector3& v1, const SVector3& v2);
	void MulIn(SVector3& v, float f);
	void DivIn(SVector3& v, float f);
	bool Equal(const SVector3& v1, const SVector3& v2);
	float Dot(const SVector3& v1, const SVector3& v2);
	SVector3 Cross(const SVector3& v1, const SVector3& v2);
	float LengthSquared(const SVector3& v);
	float Length(const SVector3& v);
	void SetLength(SVector3& v, float newLength);
	SVector3 Normalize(const SVector3& v);
	void NormalizeIn(SVector3& v);
	SVector3 Pow(const SVector3& v, float f);
	void PowIn(SVector3& v, float f);
	float Angle(const SVector3& v1, const SVector3& v2);
	float MaxComponent(const SVector3& v);
	SVector3 ProjAOnB(const SVector3& a, SVector3 b);
	SVector3 Reflect(const SVector3& input, const SVector3& normal);
	SVector3 Refract(const SVector3& input, const SVector3& normal, float eta);
	SVector3 Lerp(const SVector3& v1, const SVector3& v2, float t);
	SVector3d Lerp(const SVector3d& v1, const SVector3d& v2, double t);
	SVector3 Slerp(const SVector3& v1, const SVector3& v2, float t);
	SVector3 MoveTowards(const SVector3& src, const SVector3& dst, float maxDistance);
	SVector3 Abs(const SVector3& v);
	SVector3 Clamp(const SVector3& v, const SVector3& min, const SVector3& max);
	SVector3 Average(const SVector3* points, int pointsCount);
	SVector3 operator + (const SVector3& v1, const SVector3& v2);
	SVector3 operator - (const SVector3& v1, const SVector3& v2);
	SVector3 operator - (const SVector3& v);
	SVector3 operator * (const SVector3& v1, const SVector3& v2);
	SVector3 operator * (const SVector3& v, float f);
	SVector3 operator * (float f, const SVector3& v);
	SVector3 operator / (const SVector3& v, float f);
	SVector3 operator / (float f, const SVector3& v);
	SVector3& operator += (SVector3& v1, const SVector3& v2);
	SVector3& operator -= (SVector3& v1, const SVector3& v2);
	SVector3& operator *= (SVector3& v, float f);
	SVector3& operator *= (SVector3& v1, const SVector3& v2);
	SVector3& operator /= (SVector3& v, float f);
	bool operator == (const SVector2& v1, const SVector2& v2);

	// Vector4

	SVector4 VectorCustom(float x, float y, float z, float w);
	SVector4 VectorCopy(const SVector4& v);
	SVector4 Add(const SVector4& v1, const SVector4& v2);
	SVector4 Sub(const SVector4& v1, const SVector4& v2);
	SVector4 Mul(const SVector4& v1, const SVector4& v2);
	SVector4 Mul(const SVector4& v, float f);
	SVector4 Div(const SVector4& v, float f);
	SVector4 Div(float f, const SVector4& v);
	void AddIn(SVector4& v1, const SVector4& v2);
	void SubIn(SVector4& v1, const SVector4& v2);
	void MulIn(SVector4& v1, const SVector4& v2);
	void MulIn(SVector4& v, float f);
	void DivIn(SVector4& v, float f);
	bool Equal(const SVector4& v1, const SVector4& v2);
	SVector4 Pow(const SVector4& v, float f);
	void PowIn(SVector4& v, float f);
	float MaxComponent(const SVector4& v);
	SVector4 Abs(const SVector4& v);
	SVector4 Clamp(const SVector4& v, const SVector4& min, const SVector4& max);
	SVector4 DivideByW(const SVector4& v);
	void DivideByWIn(SVector4& v);
	SVector4 operator + (const SVector4& v1, const SVector4& v2);
	SVector4 operator - (const SVector4& v1, const SVector4& v2);
	SVector4 operator - (const SVector4& v);
	SVector4 operator * (const SVector4& v1, const SVector4& v2);
	SVector4 operator * (const SVector4& v, float f);
	SVector4 operator * (float f, const SVector4& v);
	SVector4 operator / (const SVector4& v, float f);
	SVector4 operator / (float f, const SVector4& v);
	SVector4& operator += (SVector4& v1, const SVector4& v2);
	SVector4& operator -= (SVector4& v1, const SVector4& v2);
	SVector4& operator *= (SVector4& v, float f);
	SVector4& operator *= (SVector4& v1, const SVector4& v2);
	SVector4& operator /= (SVector4& v, float f);
	bool operator == (const SVector2& v1, const SVector2& v2);

	//

	const SVector2 cVector2Zero = VectorCustom(0.0f, 0.0f);
	const SVector3 cVector3Zero = VectorCustom(0.0f, 0.0f, 0.0f);
	const SVector4 cVector4Zero = VectorCustom(0.0f, 0.0f, 0.0f, 0.0f);
	const SVector2 cVector2One = VectorCustom(1.0f, 1.0f);
	const SVector3 cVector3One = VectorCustom(1.0f, 1.0f, 1.0f);
	const SVector4 cVector4One = VectorCustom(1.0f, 1.0f, 1.0f, 1.0f);

	const SVector2 cVector2Min = VectorCustom(cFloatMin, cFloatMin);
	const SVector3 cVector3Min = VectorCustom(cFloatMin, cFloatMin, cFloatMin);
	const SVector4 cVector4Min = VectorCustom(cFloatMin, cFloatMin, cFloatMin, cFloatMin);
	const SVector2 cVector2Max = VectorCustom(cFloatMax, cFloatMax);
	const SVector3 cVector3Max = VectorCustom(cFloatMax, cFloatMax, cFloatMax);
	const SVector4 cVector4Max = VectorCustom(cFloatMax, cFloatMax, cFloatMax, cFloatMax);

	const SVector2 cVector2EX = VectorCustom(1.0f, 0.0f);
	const SVector2 cVector2EY = VectorCustom(0.0f, 1.0f);
	const SVector3 cVector3EX = VectorCustom(1.0f, 0.0f, 0.0f);
	const SVector3 cVector3EY = VectorCustom(0.0f, 1.0f, 0.0f);
	const SVector3 cVector3EZ = VectorCustom(0.0f, 0.0f, 1.0f);

	// Vector2

	inline SVector2 VectorCustom(float x, float y)
	{
		SVector2 temp;

		temp.x = x;
		temp.y = y;

		return temp;
	}

	inline SVector2 VectorCopy(const SVector2& v)
	{
		return VectorCustom(v.x, v.y);
	}

	inline SVector2 Add(const SVector2& v1, const SVector2& v2)
	{
		return VectorCustom(v1.x + v2.x, v1.y + v2.y);
	}

	inline SVector2 Sub(const SVector2& v1, const SVector2& v2)
	{
		return VectorCustom(v1.x - v2.x, v1.y - v2.y);
	}

	inline SVector2 Mul(const SVector2& v1, const SVector2& v2)
	{
		return VectorCustom(v1.x*v2.x, v1.y*v2.y);
	}

	inline SVector2 Mul(const SVector2& v, float f)
	{
		return VectorCustom(v.x*f, v.y*f);
	}

	inline SVector2 Div(const SVector2& v, float f)
	{
		f = 1.0f / f;
		return VectorCustom(v.x*f, v.y*f);
	}

	inline SVector2 Div(float f, const SVector2& v)
	{
		return VectorCustom(f/v.x, f/v.y);
	}

	inline void AddIn(SVector2& v1, const SVector2& v2)
	{
		v1.x += v2.x;
		v1.y += v2.y;
	}

	inline void SubIn(SVector2& v1, const SVector2& v2)
	{
		v1.x -= v2.x;
		v1.y -= v2.y;
	}

	inline void MulIn(SVector2& v1, const SVector2& v2)
	{
		v1.x *= v2.x;
		v1.y *= v2.y;
	}

	inline void MulIn(SVector2& v, float f)
	{
		v.x *= f;
		v.y *= f;
	}

	inline void DivIn(SVector2& v, float f)
	{
		f = 1.0f / f;
		v.x *= f;
		v.y *= f;
	}

	inline bool Equal(const SVector2& v1, const SVector2& v2)
	{
		if (NEssentials::Abs(v1.x - v2.x) < cEpsilon4 &&
			NEssentials::Abs(v1.y - v2.y) < cEpsilon4)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	inline float Dot(const SVector2& v1, const SVector2& v2)
	{
		return v1.x*v2.x + v1.y*v2.y;
	}

	inline float LengthSquared(const SVector2& v)
	{
		return Dot(v, v);
	}

	inline float Length(const SVector2& v)
	{
		return Sqrt(LengthSquared(v));
	}

	inline void SetLength(SVector2& v, float newLength)
	{
		float scale = newLength / Length(v);

		v.x *= scale;
		v.y *= scale;
	}

	inline SVector2 Normalize(const SVector2& v)
	{
		SVector2 temp = VectorCopy(v);
		NormalizeIn(temp);
		return temp;
	}

	inline void NormalizeIn(SVector2& v)
	{
		float oneOverLength = 1.0f / Length(v);

		v.x *= oneOverLength;
		v.y *= oneOverLength;
	}

	inline SVector2 Pow(const SVector2& v, float f)
	{
		return VectorCustom(Pow(v.x, f), Pow(v.y, f));
	}

	inline void PowIn(SVector2& v, float f)
	{
		v.x = Pow(v.x, f);
		v.y = Pow(v.y, f);
	}

	inline float Angle(const SVector2& v1, const SVector2& v2)
	{
		return ACos(Dot(v1, v2));
	}

	inline float MaxComponent(const SVector2& v)
	{
		return NEssentials::Max(v.x, v.y);
	}

	inline SVector2 ProjAOnB(const SVector2& a, SVector2 b)
	{
		NormalizeIn(b);
		return Dot(a, b) * b;
	}

	inline SVector2 Reflect(const SVector2& input, const SVector2& normal)
	{
		return input - 2.0f * Dot(input, normal) * normal;
	}

	inline SVector2 Refract(const SVector2& input, const SVector2& normal, float eta)
	{
		float IdotN = Dot(input, normal);
		float k = 1.0f - Sqr(eta) * (1.0f - Sqr(IdotN));
		if (k < 0.0f)
			return cVector2Zero;
		else
			return eta * input - (eta * IdotN + Sqrt(k)) * normal;
	}

	inline SVector2 Lerp(const SVector2& v1, const SVector2& v2, float t)
	{
		return (1.0f - t)*v1 + t*v2;
	}

	inline SVector2 Slerp(const SVector2& v1, const SVector2& v2, float t)
	{
		float omega = Angle(v1, v2);

		float s1 = Sin((1.0f - t)*omega);
		float s2 = Sin(t*omega);
		float s3 = Sin(omega);

		return v1*(s1/s3) + v2*(s2/s3);
	}

	inline SVector2 MoveTowards(const SVector2& src, const SVector2& dst, float maxDistance)
	{
		SVector2 direction = dst - src;
		float dist = Length(direction);
		if (dist < cEpsilon4)
			return src;

		direction /= dist;
		float distToTravel = NEssentials::Min(dist, maxDistance);

		return src + distToTravel*direction;
	}

	inline SVector2 Abs(const SVector2& v)
	{
		return VectorCustom(NEssentials::Abs(v.x), NEssentials::Abs(v.y));
	}

	inline SVector2 Clamp(const SVector2& v, const SVector2& min, const SVector2& max)
	{
		return VectorCustom(Clamp(v.x, min.x, max.x), Clamp(v.y, min.y, max.y));
	}

	inline SVector2 Average(const SVector2* points, int pointsCount)
	{
		SVector2 avg = points[0];

		for (int i = 1; i < pointsCount; i++)
			avg += points[i];

		return avg / (float)pointsCount;
	}

	inline SVector2 operator + (const SVector2& v1, const SVector2& v2)
	{
		return Add(v1, v2);
	}

	inline SVector2 operator - (const SVector2& v1, const SVector2& v2)
	{
		return Sub(v1, v2);
	}

	inline SVector2 operator - (const SVector2& v)
	{
		return VectorCustom(-v.x, -v.y);
	}

	inline SVector2 operator * (const SVector2& v1, const SVector2& v2)
	{
		return Mul(v1, v2);
	}

	inline SVector2 operator * (const SVector2& v, float f)
	{
		return Mul(v, f);
	}

	inline SVector2 operator * (float f, const SVector2& v)
	{
		return Mul(v, f);
	}

	inline SVector2 operator / (const SVector2& v, float f)
	{
		return Div(v, f);
	}

	inline SVector2 operator / (float f, const SVector2& v)
	{
		return Div(f, v);
	}

	inline SVector2& operator += (SVector2& v1, const SVector2& v2)
	{
		AddIn(v1, v2);
		return v1;
	}

	inline SVector2& operator -= (SVector2& v1, const SVector2& v2)
	{
		SubIn(v1, v2);
		return v1;
	}

	inline SVector2& operator *= (SVector2& v, float f)
	{
		MulIn(v, f);
		return v;
	}

	inline SVector2& operator *= (SVector2& v1, const SVector2& v2)
	{
		MulIn(v1, v2);
		return v1;
	}

	inline SVector2& operator /= (SVector2& v, float f)
	{
		DivIn(v, f);
		return v;
	}

	inline bool operator == (const SVector2& v1, const SVector2& v2)
	{
		return Equal(v1, v2);
	}

	// SVector2ui16

	inline bool Equal(const SVector2ui16& v1, const SVector2ui16& v2)
	{
		return (v1.x == v2.x && v1.y == v2.y);
	}

	inline bool operator == (const SVector2ui16& v1, const SVector2ui16& v2)
	{
		return Equal(v1, v2);
	}

	// SVector2i32

	inline bool Equal(const SVector2i32& v1, const SVector2i32& v2)
	{
		return (v1.x == v2.x && v1.y == v2.y);
	}

	inline bool operator == (const SVector2i32& v1, const SVector2i32& v2)
	{
		return Equal(v1, v2);
	}

	// Vector3

	inline SVector3 VectorCustom(float x, float y, float z)
	{
		SVector3 temp;

		temp.x = x;
		temp.y = y;
		temp.z = z;

		return temp;
	}

	inline SVector3 VectorCopy(const SVector3& v)
	{
		return VectorCustom(v.x, v.y, v.z);
	}

	inline SVector3 Add(const SVector3& v1, const SVector3& v2)
	{
		return VectorCustom(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z);
	}

	inline SVector3 Sub(const SVector3& v1, const SVector3& v2)
	{
		return VectorCustom(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z);
	}

	inline SVector3 Mul(const SVector3& v1, const SVector3& v2)
	{
		return VectorCustom(v1.x*v2.x, v1.y*v2.y, v1.z*v2.z);
	}

	inline SVector3 Mul(const SVector3& v, float f)
	{
		return VectorCustom(v.x*f, v.y*f, v.z*f);
	}

	inline SVector3 Div(const SVector3& v, float f)
	{
		f = 1.0f / f;
		return VectorCustom(v.x*f, v.y*f, v.z*f);
	}

	inline SVector3 Div(float f, const SVector3& v)
	{
		return VectorCustom(f/v.x, f/v.y, f/v.z);
	}

	inline void AddIn(SVector3& v1, const SVector3& v2)
	{
		v1.x += v2.x;
		v1.y += v2.y;
		v1.z += v2.z;
	}

	inline void SubIn(SVector3& v1, const SVector3& v2)
	{
		v1.x -= v2.x;
		v1.y -= v2.y;
		v1.z -= v2.z;
	}

	inline void MulIn(SVector3& v, float f)
	{
		v.x *= f;
		v.y *= f;
		v.z *= f;
	}

	inline void MulIn(SVector3& v1, const SVector3& v2)
	{
		v1.x *= v2.x;
		v1.y *= v2.y;
		v1.z *= v2.z;
	}

	inline void DivIn(SVector3& v, float f)
	{
		f = 1.0f / f;
		v.x *= f;
		v.y *= f;
		v.z *= f;
	}

	inline bool Equal(const SVector3& v1, const SVector3& v2)
	{
		if (NEssentials::Abs(v1.x - v2.x) < cEpsilon4 &&
			NEssentials::Abs(v1.y - v2.y) < cEpsilon4 &&
			NEssentials::Abs(v1.z - v2.z) < cEpsilon4)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	inline float Dot(const SVector3& v1, const SVector3& v2)
	{
		return v1.x*v2.x + v1.y*v2.y + v1.z*v2.z;
	}

	inline SVector3 Cross(const SVector3& v1, const SVector3& v2)
	{
		SVector3 temp;

		temp.x = v1.y*v2.z - v1.z*v2.y;
		temp.y = v1.z*v2.x - v1.x*v2.z;
		temp.z = v1.x*v2.y - v1.y*v2.x;

		return temp;
	}

	inline float LengthSquared(const SVector3& v)
	{
		return Dot(v, v);
	}

	inline float Length(const SVector3& v)
	{
		return Sqrt(LengthSquared(v));
	}

	inline void SetLength(SVector3& v, float newLength)
	{
		float scale = newLength / Length(v);

		v.x *= scale;
		v.y *= scale;
		v.z *= scale;
	}

	inline SVector3 Normalize(const SVector3& v)
	{
		SVector3 temp = VectorCopy(v);
		NormalizeIn(temp);
		return temp;
	}

	inline void NormalizeIn(SVector3& v)
	{
		float oneOverLength = 1.0f / Length(v);

		v.x *= oneOverLength;
		v.y *= oneOverLength;
		v.z *= oneOverLength;
	}

	inline SVector3 Pow(const SVector3& v, float f)
	{
		return VectorCustom(Pow(v.x, f), Pow(v.y, f), Pow(v.z, f));
	}

	inline void PowIn(SVector3& v, float f)
	{
		v.x = Pow(v.x, f);
		v.y = Pow(v.y, f);
		v.z = Pow(v.z, f);
	}

	inline float Angle(const SVector3& v1, const SVector3& v2)
	{
		return ACos(Dot(v1, v2));
	}

	inline float MaxComponent(const SVector3& v)
	{
		return NEssentials::Max(NEssentials::Max(v.x, v.y), v.z);
	}

	inline SVector3 ProjAOnB(const SVector3& a, SVector3 b)
	{
		NormalizeIn(b);
		return Dot(a, b) * b;
	}

	inline SVector3 Reflect(const SVector3& input, const SVector3& normal)
	{
		return input - 2.0f * Dot(input, normal) * normal;
	}

	inline SVector3 Refract(const SVector3& input, const SVector3& normal, float eta)
	{
		float IdotN = Dot(input, normal);
		float k = 1.0f - Sqr(eta) * (1.0f - Sqr(IdotN));
		if (k < 0.0f)
			return cVector3Zero;
		else
			return eta * input - (eta * IdotN + Sqrt(k)) * normal;
	}

	inline SVector3 Lerp(const SVector3& v1, const SVector3& v2, float t)
	{
		return (1.0f - t)*v1 + t*v2;
	}

	inline SVector3d Lerp(const SVector3d& v1, const SVector3d& v2, double t)
	{
		SVector3d temp;

		temp.x = (1.0 - t)*v1.x + t*v2.x;
		temp.y = (1.0 - t)*v1.y + t*v2.y;
		temp.z = (1.0 - t)*v1.z + t*v2.z;

		return temp;
	}

	inline SVector3 Slerp(const SVector3& v1, const SVector3& v2, float t)
	{
		float omega = Angle(v1, v2);

		float s1 = Sin((1.0f - t) * omega);
		float s2 = Sin(t * omega);
		float s3 = Sin(omega);

		return v1*(s1/s3) + v2*(s2/s3);
	}

	inline SVector3 MoveTowards(const SVector3& src, const SVector3& dst, float maxDistance)
	{
		SVector3 direction = dst - src;
		float dist = Length(direction);
		if (dist < cEpsilon4)
			return src;

		direction /= dist;
		float distToTravel = NEssentials::Min(dist, maxDistance);

		return src + distToTravel*direction;
	}

	inline SVector3 Abs(const SVector3& v)
	{
		return VectorCustom(NEssentials::Abs(v.x), NEssentials::Abs(v.y), NEssentials::Abs(v.z));
	}

	inline SVector3 Clamp(const SVector3& v, const SVector3& min, const SVector3& max)
	{
		return VectorCustom(Clamp(v.x, min.x, max.x), Clamp(v.y, min.y, max.y), Clamp(v.z, min.z, max.z));
	}

	inline SVector3 Average(const SVector3* points, int pointsCount)
	{
		SVector3 avg = points[0];

		for (int i = 1; i < pointsCount; i++)
			avg += points[i];

		return avg / (float)pointsCount;
	}

	inline SVector3 operator + (const SVector3& v1, const SVector3& v2)
	{
		return Add(v1, v2);
	}

	inline SVector3 operator - (const SVector3& v1, const SVector3& v2)
	{
		return Sub(v1, v2);
	}

	inline SVector3 operator - (const SVector3& v)
	{
		return VectorCustom(-v.x, -v.y, -v.z);
	}

	inline SVector3 operator * (const SVector3& v1, const SVector3& v2)
	{
		return Mul(v1, v2);
	}

	inline SVector3 operator * (const SVector3& v, float f)
	{
		return Mul(v, f);
	}

	inline SVector3 operator * (float f, const SVector3& v)
	{
		return Mul(v, f);
	}

	inline SVector3 operator / (const SVector3& v, float f)
	{
		return Div(v, f);
	}

	inline SVector3 operator / (float f, const SVector3& v)
	{
		return Div(f, v);
	}

	inline SVector3& operator += (SVector3& v1, const SVector3& v2)
	{
		AddIn(v1, v2);
		return v1;
	}

	inline SVector3& operator -= (SVector3& v1, const SVector3& v2)
	{
		SubIn(v1, v2);
		return v1;
	}

	inline SVector3& operator *= (SVector3& v1, const SVector3& v2)
	{
		MulIn(v1, v2);
		return v1;
	}

	inline SVector3& operator *= (SVector3& v, float f)
	{
		MulIn(v, f);
		return v;
	}

	inline SVector3& operator /= (SVector3& v, float f)
	{
		DivIn(v, f);
		return v;
	}

	inline bool operator == (const SVector3& v1, const SVector3& v2)
	{
		return Equal(v1, v2);
	}

	// Vector4

	inline SVector4 VectorCustom(float x, float y, float z, float w)
	{
		SVector4 temp;

		temp.x = x;
		temp.y = y;
		temp.z = z;
		temp.w = w;

		return temp;
	}

	inline SVector4 VectorCopy(const SVector4& v)
	{
		return VectorCustom(v.x, v.y, v.z, v.w);
	}

	inline SVector4 Add(const SVector4& v1, const SVector4& v2)
	{
		return VectorCustom(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z, v1.w + v2.w);
	}

	inline SVector4 Sub(const SVector4& v1, const SVector4& v2)
	{
		return VectorCustom(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z, v1.w - v2.w);
	}

	inline SVector4 Mul(const SVector4& v1, const SVector4& v2)
	{
		return VectorCustom(v1.x*v2.x, v1.y*v2.y, v1.z*v2.z, v1.w*v2.w);
	}

	inline SVector4 Mul(const SVector4& v, float f)
	{
		return VectorCustom(v.x*f, v.y*f, v.z*f, v.w*f);
	}

	inline SVector4 Div(const SVector4& v, float f)
	{
		f = 1.0f / f;
		return VectorCustom(v.x*f, v.y*f, v.z*f, v.w*f);
	}

	inline SVector4 Div(float f, const SVector4& v)
	{
		return VectorCustom(f/v.x, f/v.y, f/v.z, f/v.w);
	}

	inline void AddIn(SVector4& v1, const SVector4& v2)
	{
		v1.x += v2.x;
		v1.y += v2.y;
		v1.z += v2.z;
		v1.w += v2.w;
	}

	inline void SubIn(SVector4& v1, const SVector4& v2)
	{
		v1.x -= v2.x;
		v1.y -= v2.y;
		v1.z -= v2.z;
		v1.w -= v2.w;
	}

	inline void MulIn(SVector4& v1, const SVector4& v2)
	{
		v1.x *= v2.x;
		v1.y *= v2.y;
		v1.z *= v2.z;
		v1.w *= v2.w;
	}

	inline void MulIn(SVector4& v, float f)
	{
		v.x *= f;
		v.y *= f;
		v.z *= f;
		v.w *= f;
	}

	inline void DivIn(SVector4& v, float f)
	{
		f = 1.0f / f;
		v.x *= f;
		v.y *= f;
		v.z *= f;
		v.w *= f;
	}

	inline bool Equal(const SVector4& v1, const SVector4& v2)
	{
		if (NEssentials::Abs(v1.x - v2.x) < cEpsilon4 &&
			NEssentials::Abs(v1.y - v2.y) < cEpsilon4 &&
			NEssentials::Abs(v1.z - v2.z) < cEpsilon4 &&
			NEssentials::Abs(v1.w - v2.w) < cEpsilon4)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	inline SVector4 Pow(const SVector4& v, float f)
	{
		return VectorCustom(Pow(v.x, f), Pow(v.y, f), Pow(v.z, f), Pow(v.w, f));
	}

	inline void PowIn(SVector4& v, float f)
	{
		v.x = Pow(v.x, f);
		v.y = Pow(v.y, f);
		v.z = Pow(v.z, f);
		v.w = Pow(v.w, f);
	}

	inline float MaxComponent(const SVector4& v)
	{
		return NEssentials::Max(NEssentials::Max(v.x, v.y), NEssentials::Max(v.z, v.w));
	}

	inline SVector4 Abs(const SVector4& v)
	{
		return VectorCustom(NEssentials::Abs(v.x), NEssentials::Abs(v.y), NEssentials::Abs(v.z), NEssentials::Abs(v.w));
	}

	inline SVector4 Clamp(const SVector4& v, const SVector4& min, const SVector4& max)
	{
		return VectorCustom(Clamp(v.x, min.x, max.x), Clamp(v.y, min.y, max.y), Clamp(v.z, min.z, max.z), Clamp(v.w, min.w, max.w));
	}

	inline SVector4 DivideByW(const SVector4& v)
	{
		float oneOverW = 1.0f / v.w;
		return VectorCustom(v.x*oneOverW, v.y*oneOverW, v.z*oneOverW, 1.0f);
	}

	inline void DivideByWIn(SVector4& v)
	{
		float oneOverW = 1.0f / v.w;

		v.x *= oneOverW;
		v.y *= oneOverW;
		v.z *= oneOverW;
		v.w = 1.0f;
	}

	inline SVector4 operator + (const SVector4& v1, const SVector4& v2)
	{
		return Add(v1, v2);
	}

	inline SVector4 operator - (const SVector4& v1, const SVector4& v2)
	{
		return Sub(v1, v2);
	}

	inline SVector4 operator - (const SVector4& v)
	{
		return VectorCustom(-v.x, -v.y, -v.z, -v.w);
	}

	inline SVector4 operator * (const SVector4& v1, const SVector4& v2)
	{
		return Mul(v1, v2);
	}

	inline SVector4 operator * (const SVector4& v, float f)
	{
		return Mul(v, f);
	}

	inline SVector4 operator * (float f, const SVector4& v)
	{
		return Mul(v, f);
	}

	inline SVector4 operator / (const SVector4& v, float f)
	{
		return Div(v, f);
	}

	inline SVector4 operator / (float f, const SVector4& v)
	{
		return Div(f, v);
	}

	inline SVector4& operator += (SVector4& v1, const SVector4& v2)
	{
		AddIn(v1, v2);
		return v1;
	}

	inline SVector4& operator -= (SVector4& v1, const SVector4& v2)
	{
		SubIn(v1, v2);
		return v1;
	}

	inline SVector4& operator *= (SVector4& v1, const SVector4& v2)
	{
		MulIn(v1, v2);
		return v1;
	}

	inline SVector4& operator *= (SVector4& v, float f)
	{
		MulIn(v, f);
		return v;
	}

	inline SVector4& operator /= (SVector4& v, float f)
	{
		DivIn(v, f);
		return v;
	}

	inline bool operator == (const SVector4& v1, const SVector4& v2)
	{
		return Equal(v1, v2);
	}
} }
