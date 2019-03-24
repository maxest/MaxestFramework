#pragma once


#include "../../../src/math/main.h"


using namespace NMath;


namespace NRayTracer
{
	struct SCamera
	{
		SVector3 position;
		SMatrix transformInverse;

		float nearPlaneDistance;
		SVector2 yCoeffs;
		SVector2 xCoeffs;
	};

	//

	inline void UpdateView(SCamera& camera, const SVector3& position, const SVector3& target, const SVector3& up)
	{
		camera.position = position;

		camera.transformInverse = MatrixLookAtRH(position, target, up);
		InvertIn(camera.transformInverse);
	}

	inline void UpdateOrtho(SCamera& camera, int width, int height, float nearPlaneHeight)
	{
		float aspect = (float)width / (float)height;
		float nearPlaneWidth = aspect * nearPlaneHeight;

		SVector2 p1 = VectorCustom(0.0f, -nearPlaneHeight / 2.0f);
		SVector2 p2 = VectorCustom((float)height, nearPlaneHeight / 2.0f);
		camera.yCoeffs = SolveLineCoeffs(p1, p2);

		SVector2 p3 = VectorCustom(0.0f, -nearPlaneWidth / 2.0f);
		SVector2 p4 = VectorCustom((float)width, nearPlaneWidth / 2.0f);
		camera.xCoeffs = SolveLineCoeffs(p3, p4);
	}

	inline void UpdatePerspective(SCamera& camera, int width, int height, float fov, float nearPlaneDistance)
	{
		float aspect = (float)width / (float)height;
		SVector2 nearPlaneSize = PlaneSize(fov, aspect, nearPlaneDistance);

		camera.nearPlaneDistance = nearPlaneDistance;

		SVector2 p1 = VectorCustom(0.0f, -nearPlaneSize.y / 2.0f);
		SVector2 p2 = VectorCustom((float)height, nearPlaneSize.y / 2.0f);
		camera.yCoeffs = SolveLineCoeffs(p1, p2);

		SVector2 p3 = VectorCustom(0.0f, -nearPlaneSize.x / 2.0f);
		SVector2 p4 = VectorCustom((float)width, nearPlaneSize.x / 2.0f);
		camera.xCoeffs = SolveLineCoeffs(p3, p4);
	}

	inline void RayOrtho(const SCamera& camera, float x, float y, SVector3& rayStart, SVector3& rayDir)
	{
		rayStart.x = camera.xCoeffs.x*x + camera.xCoeffs.y;
		rayStart.y = camera.yCoeffs.x*y + camera.yCoeffs.y;
		rayStart.z = 0.0f;
		rayStart = TransformPoint(rayStart, camera.transformInverse);

		rayDir.x = 0.0f;
		rayDir.y = 0.0f;
		rayDir.z = -1.0f;
		rayDir = Transform(rayDir, camera.transformInverse);
	}

	inline void RayPerspective(const SCamera& camera, float x, float y, SVector3& rayStart, SVector3& rayDir)
	{
		rayStart = camera.position;

		rayDir.x = camera.xCoeffs.x*x + camera.xCoeffs.y;
		rayDir.y = camera.yCoeffs.x*y + camera.yCoeffs.y;
		rayDir.z = -camera.nearPlaneDistance;
		NormalizeIn(rayDir);
		rayDir = Transform(rayDir, camera.transformInverse);
	}

	inline void RayPerspectiveDOF(const SCamera& camera, float x, float y, float dofDX, float dofDY, float dofFocalPlaneDistance, SVector3& rayStart, SVector3& rayDir)
	{
		rayDir.x = camera.xCoeffs.x*x + camera.xCoeffs.y;
		rayDir.y = camera.yCoeffs.x*y + camera.yCoeffs.y;
		rayDir.z = -camera.nearPlaneDistance;
		NormalizeIn(rayDir);

		SVector3 pointOnFocalPlane = (-dofFocalPlaneDistance / rayDir.z) * rayDir;
		SVector3 dofCameraPosition = VectorCustom(dofDX, dofDY, 0.0f);

		rayStart = TransformPoint(dofCameraPosition, camera.transformInverse);

		rayDir = pointOnFocalPlane - dofCameraPosition;
		NormalizeIn(rayDir);
		rayDir = Transform(rayDir, camera.transformInverse);
	}
}
