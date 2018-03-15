#pragma once


#include "vector.h"
#include "matrix.h"


namespace NMaxestFramework { namespace NMath
{
	class CCamera
	{
	public:
		CCamera()
		{
			horizontalAngle = 0.0f;
			verticalAngle = 0.0f;
			distanceFromEyeToAt = 1.0f;
		}

		void UpdateFixed(const SVector3& eye, const SVector3& at, const SVector3& up = VectorCustom(0.0f, 1.0f, 0.0f));
		void UpdateFree(const SVector3& eye, const SVector3& up = VectorCustom(0.0f, 1.0f, 0.0f));
		void UpdateFocused(const SVector3& at, const SVector3& up = VectorCustom(0.0f, 1.0f, 0.0f));

	public:
		float horizontalAngle, verticalAngle;
		float distanceFromEyeToAt;

	public: // readonly
		SVector3 eye, at, up;
		SVector3 forwardVector, rightVector, upVector;
	};

	//

	inline void NMath::CCamera::UpdateFixed(const SVector3& eye, const SVector3& at, const SVector3& up)
	{
		forwardVector = Normalize(at - eye);

		this->eye = eye;
		this->at = at;
		this->up = up;
	}


	inline void NMath::CCamera::UpdateFree(const SVector3& eye, const SVector3& up)
	{
		SMatrix transformMatrix = MatrixRotateX(verticalAngle) * MatrixRotateY(horizontalAngle);

		forwardVector = -cVector3EZ * transformMatrix;
		rightVector = cVector3EX * transformMatrix;
		upVector = Cross(rightVector, forwardVector);

		this->eye = eye;
		this->at = eye + forwardVector;
		this->up = up;
	}


	inline void NMath::CCamera::UpdateFocused(const SVector3& at, const SVector3& up)
	{
		SMatrix transformMatrix = MatrixRotateX(verticalAngle) * MatrixRotateY(horizontalAngle);

		forwardVector = -cVector3EZ * transformMatrix;
		rightVector = cVector3EX * transformMatrix;
		upVector = Cross(rightVector, forwardVector);

		this->eye = at - forwardVector*distanceFromEyeToAt;
		this->at = at;
		this->up = up;
	}
} }
