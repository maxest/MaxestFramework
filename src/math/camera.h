#pragma once


#include "vector.h"
#include "matrix.h"


namespace NMaxestFramework { namespace NMath
{
	SMatrix MatrixPerspectiveFovRH_Zoom(EZRange zRange, float fovY, float aspectRatio, float zNear, float zFar, float zoom, const SVector2 startPos_ndc, const SVector2& targetPos_ndc, float targetFOV);

	//

	inline SMatrix MatrixPerspectiveFovRH_Zoom(EZRange zRange, float fovY, float aspectRatio, float zNear, float zFar, float zoom, const SVector2 startPos_ndc, const SVector2& targetPos_ndc, float targetFOV)
	{
		struct SUtils
		{
			SMatrix MatrixPerspectiveRH_Z(float zNear, float zFar)
			{
				return MatrixCustom(
					zNear, 0.0f, 0.0f, 0.0f,
					0.0f, zNear, 0.0f, 0.0f,
					0.0f, 0.0f, zNear + zFar, -1.0f,
					0.0f, 0.0f, zNear * zFar, 0.0f);
			}

			SMatrix MatrixPerspectiveRH_XY(EZRange zRange, float left, float right, float bottom, float top, float zNear, float zFar)
			{
				// taken from http://maxest.gct-game.net/content/vainmoinen/bachelor_thesis.pdf
				if (zRange == EZRange::MinusOneToPlusOne)
					return NMath::MatrixCustom(
					2.0f / (right - left), 0.0f, 0.0f, 0.0f,
					0.0f, 2.0f / (top - bottom), 0.0f, 0.0f,
					0.0f, 0.0f, -2.0f / (zFar - zNear), 0.0f,
					-(right + left) / (right - left), -(top + bottom) / (top - bottom), -(zNear + zFar) / (zFar - zNear), 1.0f);
				else
					return MatrixCustom(
						2.0f / (right - left), 0.0f, 0.0f, 0.0f,
						0.0f, 2.0f / (top - bottom), 0.0f, 0.0f,
						0.0f, 0.0f, -1.0f / (zFar - zNear), 0.0f,
						-(right + left) / (right - left), -(top + bottom) / (top - bottom), -zNear / (zFar - zNear), 1.0f);
			}

			SMatrix MatrixPerspectiveRH_XY(EZRange zRange, SVector2 nearPlaneSize, float zNear, float zFar)
			{
				return MatrixPerspectiveRH_XY(zRange, -0.5f*nearPlaneSize.x, 0.5f*nearPlaneSize.x, -0.5f*nearPlaneSize.y, 0.5f*nearPlaneSize.y, zNear, zFar);
			}

			SVector2 NearPlaneSize(float fovY, float aspectRatio, float zNear)
			{
				fovY /= 2.0f;
				float h = zNear * Tan(fovY);
				return 2.0f * VectorCustom(aspectRatio*h, h);
			}
		} utils;
		
		SVector2 nearPlaneSize_startFOV = utils.NearPlaneSize(fovY, aspectRatio, zNear);
	
		float fovDivider = SolveLineCoeffs(
			VectorCustom(0.0f, 1.0f),
			VectorCustom(1.0f, fovY / targetFOV),
			zoom);

		SVector2 posLerp_ndc = Lerp(targetPos_ndc, startPos_ndc, zoom);

		SMatrix temp = utils.MatrixPerspectiveRH_Z(zNear, zFar);
		temp *= MatrixScale(1.0f / (0.5f * nearPlaneSize_startFOV.x), 1.0f / (0.5f * nearPlaneSize_startFOV.y), 1.0f); // normalize near plane size to [0, 1]
		temp *= MatrixTranslate(-targetPos_ndc.x, -targetPos_ndc.y, 0.0f);
		temp *= utils.MatrixPerspectiveRH_XY(zRange, nearPlaneSize_startFOV / fovDivider, zNear, zFar);
		temp *= MatrixScale(0.5f * nearPlaneSize_startFOV.x, 0.5f * nearPlaneSize_startFOV.y, 1.0f); // de-normalize from [0, 1]
		temp *= MatrixTranslate(posLerp_ndc.x, posLerp_ndc.y, 0.0f);

		return temp;
	}

	//

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

	inline void CCamera::UpdateFixed(const SVector3& eye, const SVector3& at, const SVector3& up)
	{
		forwardVector = Normalize(at - eye);

		this->eye = eye;
		this->at = at;
		this->up = up;
	}


	inline void CCamera::UpdateFree(const SVector3& eye, const SVector3& up)
	{
		SMatrix transformMatrix = MatrixRotateX(verticalAngle) * MatrixRotateY(horizontalAngle);

		forwardVector = -cVector3EZ * transformMatrix;
		rightVector = cVector3EX * transformMatrix;
		upVector = Cross(rightVector, forwardVector);

		this->eye = eye;
		this->at = eye + forwardVector;
		this->up = up;
	}


	inline void CCamera::UpdateFocused(const SVector3& at, const SVector3& up)
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
