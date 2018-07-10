#include "matrix.h"
#include "types.h"
#include "constants.h"
#include "vector.h"


using namespace NMaxestFramework;


NMath::SMatrix NMath::MatrixCustom(
	float m00, float m01, float m02, float m03,
	float m10, float m11, float m12, float m13,
	float m20, float m21, float m22, float m23,
	float m30, float m31, float m32, float m33)
{
	SMatrix temp;

	temp.m[0][0] = m00;
	temp.m[0][1] = m01;
	temp.m[0][2] = m02;
	temp.m[0][3] = m03;

	temp.m[1][0] = m10;
	temp.m[1][1] = m11;
	temp.m[1][2] = m12;
	temp.m[1][3] = m13;

	temp.m[2][0] = m20;
	temp.m[2][1] = m21;
	temp.m[2][2] = m22;
	temp.m[2][3] = m23;

	temp.m[3][0] = m30;
	temp.m[3][1] = m31;
	temp.m[3][2] = m32;
	temp.m[3][3] = m33;

	return temp;
}


NMath::SMatrix NMath::MatrixCopy(const NMath::SMatrix& m)
{
	return MatrixCustom(
		m.m[0][0], m.m[0][1], m.m[0][2], m.m[0][3],
		m.m[1][0], m.m[1][1], m.m[1][2], m.m[1][3],
		m.m[2][0], m.m[2][1], m.m[2][2], m.m[2][3],
		m.m[3][0], m.m[3][1], m.m[3][2], m.m[3][3]);
}


NMath::SMatrix NMath::Add(const SMatrix& m1, const SMatrix& m2)
{
	SMatrix temp;

	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			temp.m[i][j] = m1.m[i][j] + m2.m[i][j];
		}
	}

	return temp;
}


NMath::SMatrix NMath::Sub(const SMatrix& m1, const SMatrix& m2)
{
	SMatrix temp;

	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			temp.m[i][j] = m1.m[i][j] - m2.m[i][j];
		}
	}

	return temp;
}


NMath::SMatrix NMath::Mul(const SMatrix& m1, const SMatrix& m2)
{
	SMatrix temp;

	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			temp.m[i][j] =
				m1.m[i][0] * m2.m[0][j] +
				m1.m[i][1] * m2.m[1][j] +
				m1.m[i][2] * m2.m[2][j] +
				m1.m[i][3] * m2.m[3][j];
		}
	}

	return temp;
}


NMath::SMatrix NMath::Transpose(const SMatrix& m)
{
	SMatrix temp = m;
	TransposeIn(temp);
	return temp;
}


NMath::SMatrix NMath::Invert(const SMatrix& m)
{
	SMatrix temp = m;
	InvertIn(temp);
	return temp;
}


NMath::SMatrix NMath::Orthogonalize3x3(const SMatrix& m)
{
	SMatrix temp = m;
	Orthogonalize3x3In(temp);
	return temp;
}


void NMath::MulIn(SMatrix& m1, const SMatrix& m2)
{
	m1 = Mul(m1, m2);
}


void NMath::TransposeIn(SMatrix& m)
{
	SMatrix temp = m;

	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			m.m[i][j] = temp.m[j][i];
		}
	}
}


void NMath::InvertIn(SMatrix& m)
{
	float determinant =
		+m.m[0][0] * Determinant(
			m.m[1][1], m.m[1][2], m.m[1][3],
			m.m[2][1], m.m[2][2], m.m[2][3],
			m.m[3][1], m.m[3][2], m.m[3][3])
		-m.m[0][1] * Determinant(
			m.m[1][0], m.m[1][2], m.m[1][3],
			m.m[2][0], m.m[2][2], m.m[2][3],
			m.m[3][0], m.m[3][2], m.m[3][3])
		+m.m[0][2] * Determinant(
			m.m[1][0], m.m[1][1], m.m[1][3],
			m.m[2][0], m.m[2][1], m.m[2][3],
			m.m[3][0], m.m[3][1], m.m[3][3])
		-m.m[0][3] * Determinant(
			m.m[1][0], m.m[1][1], m.m[1][2],
			m.m[2][0], m.m[2][1], m.m[2][2],
			m.m[3][0], m.m[3][1], m.m[3][2]);

	float adj[4][4];

	adj[0][0] = +Determinant(
		m.m[1][1], m.m[1][2], m.m[1][3],
		m.m[2][1], m.m[2][2], m.m[2][3],
		m.m[3][1], m.m[3][2], m.m[3][3]);
	adj[0][1] = -Determinant(
		m.m[1][0], m.m[1][2], m.m[1][3],
		m.m[2][0], m.m[2][2], m.m[2][3],
		m.m[3][0], m.m[3][2], m.m[3][3]);
	adj[0][2] = +Determinant(
		m.m[1][0], m.m[1][1], m.m[1][3],
		m.m[2][0], m.m[2][1], m.m[2][3],
		m.m[3][0], m.m[3][1], m.m[3][3]);
	adj[0][3] = -Determinant(
		m.m[1][0], m.m[1][1], m.m[1][2],
		m.m[2][0], m.m[2][1], m.m[2][2],
		m.m[3][0], m.m[3][1], m.m[3][2]);

	adj[1][0] = -Determinant(
		m.m[0][1], m.m[0][2], m.m[0][3],
		m.m[2][1], m.m[2][2], m.m[2][3],
		m.m[3][1], m.m[3][2], m.m[3][3]);
	adj[1][1] = +Determinant(
		m.m[0][0], m.m[0][2], m.m[0][3],
		m.m[2][0], m.m[2][2], m.m[2][3],
		m.m[3][0], m.m[3][2], m.m[3][3]);
	adj[1][2] = -Determinant(
		m.m[0][0], m.m[0][1], m.m[0][3],
		m.m[2][0], m.m[2][1], m.m[2][3],
		m.m[3][0], m.m[3][1], m.m[3][3]);
	adj[1][3] = +Determinant(
		m.m[0][0], m.m[0][1], m.m[0][2],
		m.m[2][0], m.m[2][1], m.m[2][2],
		m.m[3][0], m.m[3][1], m.m[3][2]);

	adj[2][0] = +Determinant(
		m.m[0][1], m.m[0][2], m.m[0][3],
		m.m[1][1], m.m[1][2], m.m[1][3],
		m.m[3][1], m.m[3][2], m.m[3][3]);
	adj[2][1] = -Determinant(
		m.m[0][0], m.m[0][2], m.m[0][3],
		m.m[1][0], m.m[1][2], m.m[1][3],
		m.m[3][0], m.m[3][2], m.m[3][3]);
	adj[2][2] = +Determinant(
		m.m[0][0], m.m[0][1], m.m[0][3],
		m.m[1][0], m.m[1][1], m.m[1][3],
		m.m[3][0], m.m[3][1], m.m[3][3]);
	adj[2][3] = -Determinant(
		m.m[0][0], m.m[0][1], m.m[0][2],
		m.m[1][0], m.m[1][1], m.m[1][2],
		m.m[3][0], m.m[3][1], m.m[3][2]);

	adj[3][0] = -Determinant(
		m.m[0][1], m.m[0][2], m.m[0][3],
		m.m[1][1], m.m[1][2], m.m[1][3],
		m.m[2][1], m.m[2][2], m.m[2][3]);
	adj[3][1] = +Determinant(
		m.m[0][0], m.m[0][2], m.m[0][3],
		m.m[1][0], m.m[1][2], m.m[1][3],
		m.m[2][0], m.m[2][2], m.m[2][3]);
	adj[3][2] = -Determinant(
		m.m[0][0], m.m[0][1], m.m[0][3],
		m.m[1][0], m.m[1][1], m.m[1][3],
		m.m[2][0], m.m[2][1], m.m[2][3]);
	adj[3][3] = +Determinant(
		m.m[0][0], m.m[0][1], m.m[0][2],
		m.m[1][0], m.m[1][1], m.m[1][2],
		m.m[2][0], m.m[2][1], m.m[2][2]);

	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			m.m[i][j] = (1.0f / determinant) * adj[j][i];
		}
	}
}


void NMath::Orthogonalize3x3In(SMatrix& m)
{
	SVector3 v1 = VectorCustom(m.m[0][0], m.m[0][1], m.m[0][2]);
	SVector3 v2 = VectorCustom(m.m[1][0], m.m[1][1], m.m[1][2]);
	SVector3 v3 = VectorCustom(m.m[2][0], m.m[2][1], m.m[2][2]);

	SVector3 u1 = v1;
	SVector3 u2 = v2 - (Dot(v2, u1)/Dot(u1, u1))*u1;
	SVector3 u3 = v3 - (Dot(v3, u1)/Dot(u1, u1))*u1 - (Dot(v3, u2)/Dot(u2, u2))*u2;

	m.m[0][0] = u1.x;
	m.m[0][1] = u1.y;
	m.m[0][2] = u1.z;

	m.m[1][0] = u2.x;
	m.m[1][1] = u2.y;
	m.m[1][2] = u2.z;

	m.m[2][0] = u3.x;
	m.m[2][1] = u3.y;
	m.m[2][2] = u3.z;
}


bool NMath::Equal(const SMatrix& m1, const SMatrix& m2)
{
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			if (NEssentials::Abs(m1.m[i][j] - m2.m[i][j]) >= cEpsilon4)
				return false;
		}
	}

	return true;
}


NMath::SMatrix NMath::operator + (const SMatrix& m1, const SMatrix& m2)
{
	return Add(m1, m2);
}


NMath::SMatrix NMath::operator - (const SMatrix& m1, const SMatrix& m2)
{
	return Sub(m1, m2);
}


NMath::SMatrix NMath::operator * (const SMatrix& m1, const SMatrix& m2)
{
	return Mul(m1, m2);
}


bool NMath::operator == (const SMatrix& m1, const SMatrix& m2)
{
	return Equal(m1, m2);
}


bool NMath::operator != (const SMatrix& m1, const SMatrix& m2)
{
	return !Equal(m1, m2);
}


void NMath::SetZeros(SMatrix& m)
{
	m.m[0][0] = 0.0f;
	m.m[0][1] = 0.0f;
	m.m[0][2] = 0.0f;
	m.m[0][3] = 0.0f;

	m.m[1][0] = 0.0f;
	m.m[1][1] = 0.0f;
	m.m[1][2] = 0.0f;
	m.m[1][3] = 0.0f;

	m.m[2][0] = 0.0f;
	m.m[2][1] = 0.0f;
	m.m[2][2] = 0.0f;
	m.m[2][3] = 0.0f;

	m.m[3][0] = 0.0f;
	m.m[3][1] = 0.0f;
	m.m[3][2] = 0.0f;
	m.m[3][3] = 0.0f;
}


void NMath::SetIdentity(SMatrix& m)
{
	m.m[0][0] = 1.0f;
	m.m[0][1] = 0.0f;
	m.m[0][2] = 0.0f;
	m.m[0][3] = 0.0f;

	m.m[1][0] = 0.0f;
	m.m[1][1] = 1.0f;
	m.m[1][2] = 0.0f;
	m.m[1][3] = 0.0f;

	m.m[2][0] = 0.0f;
	m.m[2][1] = 0.0f;
	m.m[2][2] = 1.0f;
	m.m[2][3] = 0.0f;

	m.m[3][0] = 0.0f;
	m.m[3][1] = 0.0f;
	m.m[3][2] = 0.0f;
	m.m[3][3] = 1.0f;
}


void NMath::SetTranslate(SMatrix& m, float x, float y, float z)
{
	m.m[0][0] = 1.0f;
	m.m[0][1] = 0.0f;
	m.m[0][2] = 0.0f;
	m.m[0][3] = 0.0f;

	m.m[1][0] = 0.0f;
	m.m[1][1] = 1.0f;
	m.m[1][2] = 0.0f;
	m.m[1][3] = 0.0f;

	m.m[2][0] = 0.0f;
	m.m[2][1] = 0.0f;
	m.m[2][2] = 1.0f;
	m.m[2][3] = 0.0f;

	m.m[3][0] = x;
	m.m[3][1] = y;
	m.m[3][2] = z;
	m.m[3][3] = 1.0f;
}


void NMath::SetTranslate(SMatrix& m, const SVector3& v)
{
	SetTranslate(m, v.x, v.y, v.z);
}


void NMath::SetRotate(SMatrix& m, float x, float y, float z, float angle)
{
	float s = sinf(angle);
	float c = cosf(angle);

	m.m[0][0] = c + x*x*(1-c);
	m.m[0][1] = x*y*(1-c) + z*s;
	m.m[0][2] = x*z*(1-c) - y*s;
	m.m[0][3] = 0.0f;

	m.m[1][0] = x*y*(1-c) - z*s;
	m.m[1][1] = c + y*y*(1-c);
	m.m[1][2] = y*z*(1-c) + x*s;
	m.m[1][3] = 0.0f;

	m.m[2][0] = x*z*(1-c) + y*s;
	m.m[2][1] = y*z*(1-c) - x*s;
	m.m[2][2] = c + z*z*(1-c);
	m.m[2][3] = 0.0f;

	m.m[3][0] = 0.0f;
	m.m[3][1] = 0.0f;
	m.m[3][2] = 0.0f;
	m.m[3][3] = 1.0f;
}


void NMath::SetRotate(SMatrix& m, const SVector3& axis, float angle)
{
	SetRotate(m, axis.x, axis.y, axis.z, angle);
}


void NMath::SetRotateX(SMatrix& m, float angle)
{
	m.m[0][0] = 1.0f;
	m.m[0][1] = 0.0f;
	m.m[0][2] = 0.0f;
	m.m[0][3] = 0.0f;

	m.m[1][0] = 0.0f;
	m.m[1][1] = cosf(angle);
	m.m[1][2] = sin(angle);
	m.m[1][3] = 0.0f;

	m.m[2][0] = 0.0f;
	m.m[2][1] = -sin(angle);
	m.m[2][2] = cos(angle);
	m.m[2][3] = 0.0f;

	m.m[3][0] = 0.0f;
	m.m[3][1] = 0.0f;
	m.m[3][2] = 0.0f;
	m.m[3][3] = 1.0f;
}


void NMath::SetRotateY(SMatrix& m, float angle)
{
	m.m[0][0] = cos(angle);
	m.m[0][1] = 0.0f;
	m.m[0][2] = -sin(angle);
	m.m[0][3] = 0.0f;

	m.m[1][0] = 0.0f;
	m.m[1][1] = 1.0f;
	m.m[1][2] = 0.0f;
	m.m[1][3] = 0.0f;

	m.m[2][0] = sin(angle);
	m.m[2][1] = 0.0f;
	m.m[2][2] = cos(angle);
	m.m[2][3] = 0.0f;

	m.m[3][0] = 0.0f;
	m.m[3][1] = 0.0f;
	m.m[3][2] = 0.0f;
	m.m[3][3] = 1.0f;
}


void NMath::SetRotateZ(SMatrix& m, float angle)
{
	m.m[0][0] = cos(angle);
	m.m[0][1] = sin(angle);
	m.m[0][2] = 0.0f;
	m.m[0][3] = 0.0f;

	m.m[1][0] = -sin(angle);
	m.m[1][1] = cos(angle);
	m.m[1][2] = 0.0f;
	m.m[1][3] = 0.0f;

	m.m[2][0] = 0.0f;
	m.m[2][1] = 0.0f;
	m.m[2][2] = 1.0f;
	m.m[2][3] = 0.0f;

	m.m[3][0] = 0.0f;
	m.m[3][1] = 0.0f;
	m.m[3][2] = 0.0f;
	m.m[3][3] = 1.0f;
}


void NMath::SetScale(SMatrix& m, float x, float y, float z)
{
	m.m[0][0] = x;
	m.m[0][1] = 0.0f;
	m.m[0][2] = 0.0f;
	m.m[0][3] = 0.0f;

	m.m[1][0] = 0.0f;
	m.m[1][1] = y;
	m.m[1][2] = 0.0f;
	m.m[1][3] = 0.0f;

	m.m[2][0] = 0.0f;
	m.m[2][1] = 0.0f;
	m.m[2][2] = z;
	m.m[2][3] = 0.0f;

	m.m[3][0] = 0.0f;
	m.m[3][1] = 0.0f;
	m.m[3][2] = 0.0f;
	m.m[3][3] = 1.0f;
}


void NMath::SetScale(SMatrix& m, const SVector3& s)
{
	SetScale(m, s.x, s.y, s.z);
}


void NMath::SetScale(SMatrix & m, float s)
{
	SetScale(m, s, s, s);
}


void NMath::SetReflect(SMatrix& m, const SPlane& plane)
{
	m.m[0][0] = -2.0f*plane.a*plane.a + 1.0f;
	m.m[0][1] = -2.0f*plane.b*plane.a;
	m.m[0][2] = -2.0f*plane.c*plane.a;
	m.m[0][3] = 0.0f;

	m.m[1][0] = -2.0f*plane.a*plane.b;
	m.m[1][1] = -2.0f*plane.b*plane.b + 1.0f;
	m.m[1][2] = -2.0f*plane.c*plane.b;
	m.m[1][3] = 0.0f;

	m.m[2][0] = -2.0f*plane.a*plane.c;
	m.m[2][1] = -2.0f*plane.b*plane.c;
	m.m[2][2] = -2.0f*plane.c*plane.c + 1.0f;
	m.m[2][3] = 0.0f;

	m.m[3][0] = -2.0f*plane.a*plane.d;
	m.m[3][1] = -2.0f*plane.b*plane.d;
	m.m[3][2] = -2.0f*plane.c*plane.d;
	m.m[3][3] = 1.0f;
}


void NMath::SetLookAtLH(SMatrix& m, const SVector3& eye, const SVector3& at, const SVector3& up)
{
	SVector3 zAxis = Normalize(at - eye);
	SVector3 xAxis = Normalize(Cross(up, zAxis));
	SVector3 yAxis = Cross(zAxis, xAxis);

	m.m[0][0] = xAxis.x;
	m.m[0][1] = yAxis.x;
	m.m[0][2] = zAxis.x;
	m.m[0][3] = 0.0f;

	m.m[1][0] = xAxis.y;
	m.m[1][1] = yAxis.y;
	m.m[1][2] = zAxis.y;
	m.m[1][3] = 0.0f;

	m.m[2][0] = xAxis.z;
	m.m[2][1] = yAxis.z;
	m.m[2][2] = zAxis.z;
	m.m[2][3] = 0.0f;

	m.m[3][0] = -Dot(xAxis, eye);
	m.m[3][1] = -Dot(yAxis, eye);
	m.m[3][2] = -Dot(zAxis, eye);
	m.m[3][3] = 1.0f;
}


void NMath::SetLookAtRH(SMatrix& m, const SVector3& eye, const SVector3& at, const SVector3& up)
{
	SVector3 zAxis = Normalize(eye - at);
	SVector3 xAxis = Normalize(Cross(up, zAxis));
	SVector3 yAxis = Cross(zAxis, xAxis);

	m.m[0][0] = xAxis.x;
	m.m[0][1] = yAxis.x;
	m.m[0][2] = zAxis.x;
	m.m[0][3] = 0.0f;

	m.m[1][0] = xAxis.y;
	m.m[1][1] = yAxis.y;
	m.m[1][2] = zAxis.y;
	m.m[1][3] = 0.0f;

	m.m[2][0] = xAxis.z;
	m.m[2][1] = yAxis.z;
	m.m[2][2] = zAxis.z;
	m.m[2][3] = 0.0f;

	m.m[3][0] = -Dot(xAxis, eye);
	m.m[3][1] = -Dot(yAxis, eye);
	m.m[3][2] = -Dot(zAxis, eye);
	m.m[3][3] = 1.0f;
}


void NMath::SetPerspectiveFovLH(SMatrix& m, EZRange zRange, float fovY, float aspectRatio, float zNear, float zFar)
{
	float yScale = 1.0f / Tan(fovY / 2.0f);
	float xScale = yScale / aspectRatio;

	m.m[0][0] = xScale;
	m.m[0][1] = 0.0f;
	m.m[0][2] = 0.0f;
	m.m[0][3] = 0.0f;

	m.m[1][0] = 0.0f;
	m.m[1][1] = yScale;
	m.m[1][2] = 0.0f;
	m.m[1][3] = 0.0f;

	m.m[2][0] = 0.0f;
	m.m[2][1] = 0.0f;
	if (zRange == EZRange::MinusOneToPlusOne)
		m.m[2][2] = (zNear + zFar) / (zFar - zNear);
	else if (zRange == EZRange::ZeroToOne)
		m.m[2][2] = zFar / (zFar - zNear);
	m.m[2][3] = 1.0f;

	m.m[3][0] = 0.0f;
	m.m[3][1] = 0.0f;
	if (zRange == EZRange::MinusOneToPlusOne)
		m.m[3][2] = -2.0f * zNear * zFar / (zFar - zNear);
	else if (zRange == EZRange::ZeroToOne)
		m.m[3][2] = -zNear * zFar / (zFar - zNear);
	m.m[3][3] = 0.0f;
}


void NMath::SetPerspectiveFovRH(SMatrix& m, EZRange zRange, float fovY, float aspectRatio, float zNear, float zFar)
{
	float yScale = 1.0f / Tan(fovY / 2.0f);
	float xScale = yScale / aspectRatio;

	m.m[0][0] = xScale;
	m.m[0][1] = 0.0f;
	m.m[0][2] = 0.0f;
	m.m[0][3] = 0.0f;

	m.m[1][0] = 0.0f;
	m.m[1][1] = yScale;
	m.m[1][2] = 0.0f;
	m.m[1][3] = 0.0f;

	m.m[2][0] = 0.0f;
	m.m[2][1] = 0.0f;
	if (zRange == EZRange::MinusOneToPlusOne)
		m.m[2][2] = (zNear + zFar) / (zNear - zFar);
	else if (zRange == EZRange::ZeroToOne)
		m.m[2][2] = zFar / (zNear - zFar);
	m.m[2][3] = -1.0f;

	m.m[3][0] = 0.0f;
	m.m[3][1] = 0.0f;
	if (zRange == EZRange::MinusOneToPlusOne)
		m.m[3][2] = 2.0f * zNear * zFar / (zNear - zFar);
	else if (zRange == EZRange::ZeroToOne)
		m.m[3][2] = zNear * zFar / (zNear - zFar);
	m.m[3][3] = 0.0f;
}


void NMath::SetOrthoOffCenterLH(SMatrix& m, EZRange zRange, float left, float right, float bottom, float top, float zNear, float zFar)
{
	m.m[0][0] = 2.0f / (right - left);
	m.m[0][1] = 0.0f;
	m.m[0][2] = 0.0f;
	m.m[0][3] = 0.0f;

	m.m[1][0] = 0.0f;
	m.m[1][1] = 2.0f / (top - bottom);
	m.m[1][2] = 0.0f;
	m.m[1][3] = 0.0f;

	m.m[2][0] = 0.0f;
	m.m[2][1] = 0.0f;
	if (zRange == EZRange::MinusOneToPlusOne)
		m.m[2][2] = -2.0f / (zNear - zFar);
	else if (zRange == EZRange::ZeroToOne)
		m.m[2][2] = 1.0f / (zFar - zNear);
	m.m[2][3] = 0.0f;

	m.m[3][0] = (1.0f + right) / (1.0f - right);
	m.m[3][1] = (top + bottom) / (bottom - top);
	if (zRange == EZRange::MinusOneToPlusOne)
		m.m[3][2] = (zNear + zFar) / (zNear - zFar);
	else if (zRange == EZRange::ZeroToOne)
		m.m[3][2] = -zNear / (zFar - zNear);
	m.m[3][3] = 1.0f;
}


void NMath::SetOrthoOffCenterRH(SMatrix& m, EZRange zRange, float left, float right, float bottom, float top, float zNear, float zFar)
{
	m.m[0][0] = 2.0f / (right - left);
	m.m[0][1] = 0.0f;
	m.m[0][2] = 0.0f;
	m.m[0][3] = 0.0f;

	m.m[1][0] = 0.0f;
	m.m[1][1] = 2.0f / (top - bottom);
	m.m[1][2] = 0.0f;
	m.m[1][3] = 0.0f;

	m.m[2][0] = 0.0f;
	m.m[2][1] = 0.0f;
	if (zRange == EZRange::MinusOneToPlusOne)
		m.m[2][2] = 2.0f / (zNear - zFar);
	else if (zRange == EZRange::ZeroToOne)
		m.m[2][2] = 1.0f / (zNear - zFar);
	m.m[2][3] = 0.0f;

	m.m[3][0] = (1.0f + right) / (1.0f - right);
	m.m[3][1] = (top + bottom) / (bottom - top);
	if (zRange == EZRange::MinusOneToPlusOne)
		m.m[3][2] = (zNear + zFar) / (zNear - zFar);
	else if (zRange == EZRange::ZeroToOne)
		m.m[3][2] = zNear / (zNear - zFar);
	m.m[3][3] = 1.0f;
}


void NMath::SetOrthoLH(SMatrix& m, EZRange zRange, float width, float height, float zNear, float zFar)
{
	m.m[0][0] = 2.0f / width;
	m.m[0][1] = 0.0f;
	m.m[0][2] = 0.0f;
	m.m[0][3] = 0.0f;

	m.m[1][0] = 0.0f;
	m.m[1][1] = 2.0f / height;
	m.m[1][2] = 0.0f;
	m.m[1][3] = 0.0f;

	m.m[2][0] = 0.0f;
	m.m[2][1] = 0.0f;
	if (zRange == EZRange::MinusOneToPlusOne)
		m.m[2][2] = -2.0f / (zNear - zFar);
	else if (zRange == EZRange::ZeroToOne)
		m.m[2][2] = 1.0f / (zFar - zNear);
	m.m[2][3] = 0.0f;

	m.m[3][0] = 0.0f;
	m.m[3][1] = 0.0f;
	if (zRange == EZRange::MinusOneToPlusOne)
		m.m[3][2] = (zNear + zFar) / (zNear - zFar);
	else if (zRange == EZRange::ZeroToOne)
		m.m[3][2] = -zNear / (zFar - zNear);
	m.m[3][3] = 1.0f;
}


void NMath::SetOrthoRH(SMatrix& m, EZRange zRange, float width, float height, float zNear, float zFar)
{
	m.m[0][0] = 2.0f / width;
	m.m[0][1] = 0.0f;
	m.m[0][2] = 0.0f;
	m.m[0][3] = 0.0f;

	m.m[1][0] = 0.0f;
	m.m[1][1] = 2.0f / height;
	m.m[1][2] = 0.0f;
	m.m[1][3] = 0.0f;

	m.m[2][0] = 0.0f;
	m.m[2][1] = 0.0f;
	if (zRange == EZRange::MinusOneToPlusOne)
		m.m[2][2] = 2.0f / (zNear - zFar);
	else if (zRange == EZRange::ZeroToOne)
		m.m[2][2] = 1.0f / (zNear - zFar);
	m.m[2][3] = 0.0f;

	m.m[3][0] = 0.0f;
	m.m[3][1] = 0.0f;
	if (zRange == EZRange::MinusOneToPlusOne)
		m.m[3][2] = (zNear + zFar) / (zNear - zFar);
	else if (zRange == EZRange::ZeroToOne)
		m.m[3][2] = zNear / (zNear - zFar);
	m.m[3][3] = 1.0f;
}


NMath::SMatrix NMath::MatrixZeros()
{
	SMatrix temp;
	SetZeros(temp);
	return temp;
}


NMath::SMatrix NMath::MatrixIdentity()
{
	SMatrix temp;
	SetIdentity(temp);
	return temp;
}


NMath::SMatrix NMath::MatrixTranslate(float x, float y, float z)
{
	SMatrix temp;
	SetTranslate(temp, x, y, z);
	return temp;
}


NMath::SMatrix NMath::MatrixTranslate(const SVector3& v)
{
	return MatrixTranslate(v.x, v.y, v.z);
}


NMath::SMatrix NMath::MatrixRotate(float x, float y, float z, float angle)
{
	SMatrix temp;
	SetRotate(temp, x, y, z, angle);
	return temp;
}


NMath::SMatrix NMath::MatrixRotate(const SVector3& axis, float angle)
{
	return MatrixRotate(axis.x, axis.y, axis.z, angle);
}


NMath::SMatrix NMath::MatrixRotateX(float angle)
{
	SMatrix temp;
	SetRotateX(temp, angle);
	return temp;
}


NMath::SMatrix NMath::MatrixRotateY(float angle)
{
	SMatrix temp;
	SetRotateY(temp, angle);
	return temp;
}


NMath::SMatrix NMath::MatrixRotateZ(float angle)
{
	SMatrix temp;
	SetRotateZ(temp, angle);
	return temp;
}


NMath::SMatrix NMath::MatrixScale(float x, float y, float z)
{
	SMatrix temp;
	SetScale(temp, x, y, z);
	return temp;
}


NMath::SMatrix NMath::MatrixScale(const SVector3& s)
{
	return MatrixScale(s.x, s.y, s.z);
}


NMath::SMatrix NMath::MatrixScale(float s)
{
	SMatrix temp;
	SetScale(temp, s, s, s);
	return temp;
}


NMath::SMatrix NMath::MatrixReflect(const SPlane& plane)
{
	SMatrix temp;
	SetReflect(temp, plane);
	return temp;
}


NMath::SMatrix NMath::MatrixLookAtLH(const SVector3& eye, const SVector3& at, const SVector3& up)
{
	SMatrix temp;
	SetLookAtLH(temp, eye, at, up);
	return temp;
}


NMath::SMatrix NMath::MatrixLookAtRH(const SVector3& eye, const SVector3& at, const SVector3& up)
{
	SMatrix temp;
	SetLookAtRH(temp, eye, at, up);
	return temp;
}


NMath::SMatrix NMath::MatrixPerspectiveFovLH(EZRange zRange, float fovY, float aspectRatio, float zNear, float zFar)
{
	SMatrix temp;
	SetPerspectiveFovLH(temp, zRange, fovY, aspectRatio, zNear, zFar);
	return temp;
}


NMath::SMatrix NMath::MatrixPerspectiveFovRH(EZRange zRange, float fovY, float aspectRatio, float zNear, float zFar)
{
	SMatrix temp;
	SetPerspectiveFovRH(temp, zRange, fovY, aspectRatio, zNear, zFar);
	return temp;
}


NMath::SMatrix NMath::MatrixOrthoOffCenterLH(EZRange zRange, float left, float right, float bottom, float top, float zNear, float zFar)
{
	SMatrix temp;
	SetOrthoOffCenterLH(temp, zRange, left, right, bottom, top, zNear, zFar);
	return temp;
}


NMath::SMatrix NMath::MatrixOrthoOffCenterRH(EZRange zRange, float left, float right, float bottom, float top, float zNear, float zFar)
{
	SMatrix temp;
	SetOrthoOffCenterRH(temp, zRange, left, right, bottom, top, zNear, zFar);
	return temp;
}


NMath::SMatrix NMath::MatrixOrthoLH(EZRange zRange, float width, float height, float zNear, float zFar)
{
	SMatrix temp;
	SetOrthoLH(temp, zRange, width, height, zNear, zFar);
	return temp;
}


NMath::SMatrix NMath::MatrixOrthoRH(EZRange zRange, float width, float height, float zNear, float zFar)
{
	SMatrix temp;
	SetOrthoRH(temp, zRange, width, height, zNear, zFar);
	return temp;
}
