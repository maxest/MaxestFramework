#pragma once


#include "../../../src/math/main.h"


using namespace NMath;


namespace NRayTracer
{
	class CCamera
	{
	private:
		enum class EType { Perspective, Ortho };

	public:
		void UpdateView(const SVector3& eye, const SVector3& at, const SVector3& up)
		{
			position = eye;

			transformInverse = MatrixLookAtRH(eye, at, up);
			InvertIn(transformInverse);
		}

		void SetPerspective(float fovY, int width, int height, float nearPlaneDistance)
		{
			type = EType::Perspective;

			float aspect = (float)width / (float)height;
			SVector2 nearPlaneSize = PlaneSize(fovY, aspect, nearPlaneDistance);

			this->nearPlaneDistance = nearPlaneDistance;

			SVector2 p1 = VectorCustom(0.0f, -nearPlaneSize.y / 2.0f);
			SVector2 p2 = VectorCustom((float)height, nearPlaneSize.y / 2.0f);
			yCoeffs = SolveLineCoeffs(p1, p2);

			SVector2 p3 = VectorCustom(0.0f, -nearPlaneSize.x / 2.0f);
			SVector2 p4 = VectorCustom((float)width, nearPlaneSize.x / 2.0f);
			xCoeffs = SolveLineCoeffs(p3, p4);
		}

		void SetOrtho(int width, int height, float nearPlaneHeight)
		{
			type = EType::Ortho;

			float aspect = (float)width / (float)height;
			float nearPlaneWidth = aspect * nearPlaneHeight;

			SVector2 p1 = VectorCustom(0.0f, -nearPlaneHeight / 2.0f);
			SVector2 p2 = VectorCustom((float)height, nearPlaneHeight / 2.0f);
			yCoeffs = SolveLineCoeffs(p1, p2);

			SVector2 p3 = VectorCustom(0.0f, -nearPlaneWidth / 2.0f);
			SVector2 p4 = VectorCustom((float)width, nearPlaneWidth / 2.0f);
			xCoeffs = SolveLineCoeffs(p3, p4);
		}

		FORCE_INLINE void Ray(float x, float y, SVector3& rayStart, SVector3& rayDir) const
		{
			if (type == EType::Perspective)
			{
				rayStart = position;

				rayDir.x = xCoeffs.x*x + xCoeffs.y;
				rayDir.y = yCoeffs.x*y + yCoeffs.y;
				rayDir.z = -nearPlaneDistance;
				NormalizeIn(rayDir);
				rayDir = Transform(rayDir, transformInverse);
			}
			else if (type == EType::Ortho)
			{
				rayStart.x = xCoeffs.x*x + xCoeffs.y;
				rayStart.y = yCoeffs.x*y + yCoeffs.y;
				rayStart.z = 0.0f;
				rayStart = TransformPoint(rayStart, transformInverse);

				rayDir.x = 0.0f;
				rayDir.y = 0.0f;
				rayDir.z = -1.0f;
				rayDir = Transform(rayDir, transformInverse);
			}
		}

		FORCE_INLINE void RayDOF(float x, float y, float dofDX, float dofDY, float dofFocalPlaneDistance, SVector3& rayStart, SVector3& rayDir) const
		{
			if (type == EType::Perspective)
			{
				rayDir.x = xCoeffs.x*x + xCoeffs.y;
				rayDir.y = yCoeffs.x*y + yCoeffs.y;
				rayDir.z = -nearPlaneDistance;
				NormalizeIn(rayDir);

				SVector3 pointOnFocalPlane = (-dofFocalPlaneDistance / rayDir.z) * rayDir;
				SVector3 dofCameraPosition = VectorCustom(dofDX, dofDY, 0.0f);

				rayStart = TransformPoint(dofCameraPosition, transformInverse);

				rayDir = pointOnFocalPlane - dofCameraPosition;
				NormalizeIn(rayDir);
				rayDir = Transform(rayDir, transformInverse);
			}
			else if (type == EType::Ortho)
			{
				MF_ASSERT(false);
			}
		}

	private:
		EType type;

		SVector3 position;
		SMatrix transformInverse;

		float nearPlaneDistance;
		SVector2 yCoeffs;
		SVector2 xCoeffs;
	};
}
