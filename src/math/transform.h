#pragma once


#include "types.h"
#include "vector.h"
#include "quaternion.h"


namespace NMaxestFramework { namespace NMath
{
	SVector2 Transform(const SVector2& v, const SMatrix& m);
	SVector3 Transform(const SVector3& v, const SMatrix& m);
	SVector4 Transform(const SVector4& v, const SMatrix& m);
	SVector2 Transform(const SMatrix& m, const SVector2& v);
	SVector3 Transform(const SMatrix& m, const SVector3& v);
	SVector4 Transform(const SMatrix& m, const SVector4& v);
	SVector2 operator * (const SVector2& v, const SMatrix& m);
	SVector3 operator * (const SVector3& v, const SMatrix& m);
	SVector4 operator * (const SVector4& v, const SMatrix& m);
	SVector2 operator * (const SMatrix& m, const SVector2& v);
	SVector3 operator * (const SMatrix& m, const SVector3& v);
	SVector4 operator * (const SMatrix& m, const SVector4& v);

	SVector3 TransformPoint(const SVector3& p, const SQuaternion& q);
	SVector3 TransformPoint(const SVector3& p, const SMatrix& m);
	SVector3 TransformVector(const SVector3& v, const SMatrix& m);
	SVector3 TransformNormal(const SVector3& v, SMatrix m);
	SPlane TransformPlane(const SPlane& plane, SMatrix transform);
	SAABB TransformAABB(const SAABB& aabb, const SMatrix& transform);

	//

	inline SVector2 Transform(const SVector2& v, const SMatrix& m)
	{
		SVector2 temp;

		temp.x = v.x*m.m[0][0] + v.y*m.m[1][0];
		temp.y = v.x*m.m[0][1] + v.y*m.m[1][1];

		return temp;
	}

	inline SVector3 Transform(const SVector3& v, const SMatrix& m)
	{
		SVector3 temp;

		temp.x = v.x*m.m[0][0] + v.y*m.m[1][0] + v.z*m.m[2][0];
		temp.y = v.x*m.m[0][1] + v.y*m.m[1][1] + v.z*m.m[2][1];
		temp.z = v.x*m.m[0][2] + v.y*m.m[1][2] + v.z*m.m[2][2];

		return temp;
	}

	inline SVector4 Transform(const SVector4& v, const SMatrix& m)
	{
		SVector4 temp;

		temp.x = v.x*m.m[0][0] + v.y*m.m[1][0] + v.z*m.m[2][0] + v.w*m.m[3][0];
		temp.y = v.x*m.m[0][1] + v.y*m.m[1][1] + v.z*m.m[2][1] + v.w*m.m[3][1];
		temp.z = v.x*m.m[0][2] + v.y*m.m[1][2] + v.z*m.m[2][2] + v.w*m.m[3][2];
		temp.w = v.x*m.m[0][3] + v.y*m.m[1][3] + v.z*m.m[2][3] + v.w*m.m[3][3];

		return temp;
	}

	inline SVector2 Transform(const SMatrix& m, const SVector2& v)
	{
		SVector2 temp;

		temp.x = m.m[0][0]*v.x + m.m[0][1]*v.y;
		temp.y = m.m[1][0]*v.x + m.m[1][1]*v.y;

		return temp;
	}

	inline SVector3 Transform(const SMatrix& m, const SVector3& v)
	{
		SVector3 temp;

		temp.x = m.m[0][0]*v.x + m.m[0][1]*v.y + m.m[0][2]*v.z;
		temp.y = m.m[1][0]*v.x + m.m[1][1]*v.y + m.m[1][2]*v.z;
		temp.z = m.m[2][0]*v.x + m.m[2][1]*v.y + m.m[2][2]*v.z;

		return temp;
	}

	inline SVector4 Transform(const SMatrix& m, const SVector4& v)
	{
		SVector4 temp;

		temp.x = m.m[0][0]*v.x + m.m[0][1]*v.y + m.m[0][2]*v.z + m.m[0][3]*v.w;
		temp.y = m.m[1][0]*v.x + m.m[1][1]*v.y + m.m[1][2]*v.z + m.m[1][3]*v.w;
		temp.z = m.m[2][0]*v.x + m.m[2][1]*v.y + m.m[2][2]*v.z + m.m[2][3]*v.w;
		temp.w = m.m[3][0]*v.x + m.m[3][1]*v.y + m.m[3][2]*v.z + m.m[3][3]*v.w;

		return temp;
	}

	inline SVector2 operator * (const SVector2& v, const SMatrix& m)
	{
		return Transform(v, m);
	}

	inline SVector3 operator * (const SVector3& v, const SMatrix& m)
	{
		return Transform(v, m);
	}

	inline SVector4 operator * (const SVector4& v, const SMatrix& m)
	{
		return Transform(v, m);
	}

	inline SVector2 operator * (const SMatrix& m, const SVector2& v)
	{
		return Transform(m, v);
	}

	inline SVector3 operator * (const SMatrix& m, const SVector3& v)
	{
		return Transform(m, v);
	}

	inline SVector4 operator * (const SMatrix& m, const SVector4& v)
	{
		return Transform(m, v);
	}

	inline SVector3 TransformPoint(const SVector3& p, const SQuaternion& q)
	{
		SQuaternion temp = (Conjugate(q)) * (SQuaternionCustom(p.x, p.y, p.z, 0.0f)) * q;
		return VectorCustom(temp.x, temp.y, temp.z);
	}

	inline SVector3 TransformPoint(const SVector3& p, const SMatrix& m)
	{
		SVector4 temp = VectorCustom(p.x, p.y, p.z, 1.0f);
		temp = Transform(temp, m);
		DivideByWIn(temp);
		return VectorCustom(temp.x, temp.y, temp.z);
	}

	inline SVector3 TransformVector(const SVector3& v, const SMatrix& m)
	{
		return Transform(v, m);
	}

	inline SVector3 TransformNormal(const SVector3& v, SMatrix m)
	{
		Invert(m);
		Transpose(m);
		return v * m;
	}

	inline SPlane TransformPlane(const SPlane& p, SMatrix m)
	{
		SVector4 v = VectorCustom(p.a, p.b, p.c, p.d);
		Invert(m);
		Transpose(m);
		v = v * m;

		SPlane newP;
		newP.a = v.x;
		newP.b = v.y;
		newP.c = v.z;
		newP.d = v.w;
		NormalizeIn(newP);

		return newP;
	}

	inline SAABB TransformAABB(const SAABB& aabb, const SMatrix& transform)
	{
		NMath::SVector3 vertices[8];
		AABBExtractCorners(aabb, vertices);

		for (int i = 0; i < 8; i++)
			vertices[i] = TransformPoint(vertices[i], transform);

		return AABBFromVertices(vertices, 8);
	}
} }
