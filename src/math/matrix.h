#pragma once


#include "types.h"


namespace NMaxestFramework { namespace NMath
{
	SMatrix MatrixCustom(
		float m00, float m01, float m02, float m03,
		float m10, float m11, float m12, float m13,
		float m20, float m21, float m22, float m23,
		float m30, float m31, float m32, float m33);
	SMatrix MatrixCopy(const SMatrix & m);

	SMatrix Add(const SMatrix & m1, const SMatrix & m2);
	SMatrix Sub(const SMatrix & m1, const SMatrix & m2);
	SMatrix Mul(const SMatrix & m1, const SMatrix & m2);
	SMatrix Transpose(const SMatrix & m);
	SMatrix Invert(const SMatrix & m);
	SMatrix Orthogonalize3x3(const SMatrix & m);
	void MulIn(SMatrix & m1, const SMatrix & m2);
	void TransposeIn(SMatrix & m);
	void InvertIn(SMatrix & m);
	void Orthogonalize3x3In(SMatrix & m);
	bool Equal(const SMatrix & m1, const SMatrix & m2);
	SMatrix operator * (const SMatrix & m1, const SMatrix & m2);
	SMatrix operator + (const SMatrix & m1, const SMatrix & m2);
	SMatrix operator - (const SMatrix & m1, const SMatrix & m2);
	bool operator == (const SMatrix & m1, const SMatrix & m2);
	bool operator != (const SMatrix & m1, const SMatrix & m2);

	void SetZeros(SMatrix & m);
	void SetIdentity(SMatrix & m);
	void SetTranslate(SMatrix & m, float x, float y, float z);
	void SetTranslate(SMatrix & m, const SVector3& v);
	void SetRotate(SMatrix & m, float x, float y, float z, float angle);
	void SetRotate(SMatrix & m, const SVector3& axis, float angle);
	void SetRotateX(SMatrix & m, float angle);
	void SetRotateY(SMatrix & m, float angle);
	void SetRotateZ(SMatrix & m, float angle);
	void SetScale(SMatrix & m, float x, float y, float z);
	void SetScale(SMatrix & m, const SVector3& s);
	void SetScale(SMatrix & m, float s);
	void SetReflect(SMatrix & m, const SPlane& plane);
	void SetLookAtLH(SMatrix & m, const SVector3& eye, const SVector3& at, const SVector3& up);
	void SetLookAtRH(SMatrix & m, const SVector3& eye, const SVector3& at, const SVector3& up);
	void SetPerspectiveFovLH(SMatrix & m, EZRange zRange, float fovY, float aspectRatio, float zNear, float zFar);
	void SetPerspectiveFovRH(SMatrix & m, EZRange zRange, float fovY, float aspectRatio, float zNear, float zFar);
	void SetOrthoOffCenterLH(SMatrix & m, EZRange zRange, float left, float right, float bottom, float top, float zNear, float zFar);
	void SetOrthoOffCenterRH(SMatrix & m, EZRange zRange, float left, float right, float bottom, float top, float zNear, float zFar);
	void SetOrthoLH(SMatrix & m, EZRange zRange, float width, float height, float zNear, float zFar);
	void SetOrthoRH(SMatrix & m, EZRange zRange, float width, float height, float zNear, float zFar);

	SMatrix MatrixZeros();
	SMatrix MatrixIdentity();
	SMatrix MatrixTranslate(float x, float y, float z);
	SMatrix MatrixTranslate(const SVector3& v);
	SMatrix MatrixRotate(float x, float y, float z, float angle);
	SMatrix MatrixRotate(const SVector3& axis, float angle);
	SMatrix MatrixRotateX(float angle);
	SMatrix MatrixRotateY(float angle);
	SMatrix MatrixRotateZ(float angle);
	SMatrix MatrixScale(float x, float y, float z);
	SMatrix MatrixScale(const SVector3& s);
	SMatrix MatrixScale(float s);
	SMatrix MatrixReflect(const SPlane& plane);
	SMatrix MatrixLookAtLH(const SVector3& eye, const SVector3& at, const SVector3& up);
	SMatrix MatrixLookAtRH(const SVector3& eye, const SVector3& at, const SVector3& up);
	SMatrix MatrixPerspectiveFovLH(EZRange zRange, float fovY, float aspectRatio, float zNear, float zFar);
	SMatrix MatrixPerspectiveFovRH(EZRange zRange, float fovY, float aspectRatio, float zNear, float zFar);
	SMatrix MatrixOrthoOffCenterLH(EZRange zRange, float left, float right, float bottom, float top, float zNear, float zFar);
	SMatrix MatrixOrthoOffCenterRH(EZRange zRange, float left, float right, float bottom, float top, float zNear, float zFar);
	SMatrix MatrixOrthoLH(EZRange zRange, float width, float height, float zNear, float zFar);
	SMatrix MatrixOrthoRH(EZRange zRange, float width, float height, float zNear, float zFar);

	float Determinant(
		float _00, float _01, float _02,
		float _10, float _11, float _12,
		float _20, float _21, float _22);

	//

	const SMatrix cMatrixIdentity = MatrixIdentity();

	//

	inline float Determinant(
		float _00, float _01, float _02,
		float _10, float _11, float _12,
		float _20, float _21, float _22)
	{
		return ( (_00*_11*_22 + _10*_21*_02 + _20*_01*_12) - (_02*_11*_20 + _12*_21*_00 + _22*_01*_10) );
	}
} }
