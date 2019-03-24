#include "scene.h"


using namespace NRayTracer;


void CScene::AddMesh(const NMesh::SMesh& mesh, const SMatrix& transform, int materialIndex)
{
	for (uint i = 0; i < mesh.chunks.size(); i++)
		MF_ASSERT(mesh.chunks[i].indices.size() != 0);

	STrianglePrimitive triangle;
	triangle.materialIndex = materialIndex;

	for (uint i = 0; i < mesh.chunks.size(); i++)
	{
		for (uint j = 0; j < mesh.chunks[i].indices.size() / 3; j++)
		{
			triangle.p1 = VectorCustom(
				mesh.chunks[i].vertices[mesh.chunks[i].indices[3 * j + 0]].position.x,
				mesh.chunks[i].vertices[mesh.chunks[i].indices[3 * j + 0]].position.y,
				mesh.chunks[i].vertices[mesh.chunks[i].indices[3 * j + 0]].position.z);
			triangle.p2 = VectorCustom(
				mesh.chunks[i].vertices[mesh.chunks[i].indices[3 * j + 1]].position.x,
				mesh.chunks[i].vertices[mesh.chunks[i].indices[3 * j + 1]].position.y,
				mesh.chunks[i].vertices[mesh.chunks[i].indices[3 * j + 1]].position.z);
			triangle.p3 = VectorCustom(
				mesh.chunks[i].vertices[mesh.chunks[i].indices[3 * j + 2]].position.x,
				mesh.chunks[i].vertices[mesh.chunks[i].indices[3 * j + 2]].position.y,
				mesh.chunks[i].vertices[mesh.chunks[i].indices[3 * j + 2]].position.z);

			triangle.p1 = TransformPoint(triangle.p1, transform);
			triangle.p2 = TransformPoint(triangle.p2, transform);
			triangle.p3 = TransformPoint(triangle.p3, transform);

			triangle.normal = TriangleNormal(triangle.p1, triangle.p2, triangle.p3);

			triangles.push_back(triangle);
		}
	}
}


bool CScene::IntersectionPrimary(const SVector3& rayStart, const SVector3& rayDir, float maxDistance, SSceneIntersectionResult& sceneIntersectionResult) const
{
	float closestDistance = cFloatMax;
	SVector3 tempIntersectionPoint;
	float tempDistance;

	for (uint i = 0; i < triangles.size(); i++)
	{
		if (IntersectionRayTriangle(rayStart, rayDir, triangles[i].p1, triangles[i].p2, triangles[i].p3, tempIntersectionPoint, tempDistance))
		{
			if (tempDistance < 0.0f)
				continue;
			if (tempDistance > closestDistance)
				continue;
			if (tempDistance > maxDistance)
				continue;
			// backface culling
			if (Dot(rayDir, triangles[i].normal) > 0.0f)
				continue;

			sceneIntersectionResult.materialIndex = triangles[i].materialIndex;
			sceneIntersectionResult.point = tempIntersectionPoint;
			sceneIntersectionResult.normal = triangles[i].normal;
			sceneIntersectionResult.backside = false;
			sceneIntersectionResult.triangleIndex = i;

			closestDistance = tempDistance;
		}
	}

	for (uint i = 0; i < spheres.size(); i++)
	{
		if (IntersectionRaySphere(rayStart, rayDir, spheres[i].position, spheres[i].radius, tempIntersectionPoint, tempDistance))
		{
			if (tempDistance < 0.0f)
				continue;
			if (tempDistance > closestDistance)
				continue;
			if (tempDistance > maxDistance)
				continue;

			sceneIntersectionResult.materialIndex = spheres[i].materialIndex;
			sceneIntersectionResult.point = tempIntersectionPoint;
			sceneIntersectionResult.normal = 1.0f / spheres[i].radius * (tempIntersectionPoint - spheres[i].position);
			sceneIntersectionResult.backside = false;
			sceneIntersectionResult.triangleIndex = -1;

			// are we inside the sphere?
			if (Distance(rayStart, spheres[i].position) < spheres[i].radius)
			{
				sceneIntersectionResult.normal = -sceneIntersectionResult.normal;
				sceneIntersectionResult.backside = true;
			}

			closestDistance = tempDistance;
		}
	}

	return closestDistance != cFloatMax;
}


bool CScene::IntersectionShadow(const SVector3& rayStart, const SVector3& rayDir, float maxDistance, int triangleIndex) const
{
	float closestDistance = cFloatMax;
	SVector3 tempIntersectionPoint;
	float tempDistance;

	for (uint i = 0; i < triangles.size(); i++)
	{
		if ((int)i == triangleIndex) // no self-occlusion please
			continue;

		if (IntersectionRayTriangle(rayStart, rayDir, triangles[i].p1, triangles[i].p2, triangles[i].p3, tempIntersectionPoint, tempDistance))
		{
			if (tempDistance < 0.0f)
				continue;
			if (tempDistance > closestDistance)
				continue;
			if (tempDistance > maxDistance)
				continue;
			// backface culling
			// If we comment this out, then front-facing and back-facing triangles can be aligned and
			// there won't be a problem with shadow acne.
			// However, to be consistent with other ray intersection functions it's here.
			if (Dot(rayDir, triangles[i].normal) > 0.0f)
				continue;

			closestDistance = tempDistance;
		}
	}

	for (uint i = 0; i < spheres.size(); i++)
	{
		// sphere uses offset instead of "triangleIndex" because sphere can occlude itself
		if (IntersectionRaySphere(rayStart + 0.001f*rayDir, rayDir, spheres[i].position, spheres[i].radius, tempIntersectionPoint, tempDistance))
		{
			if (tempDistance < 0.0f)
				continue;
			if (tempDistance > closestDistance)
				continue;
			if (tempDistance > maxDistance)
				continue;

			closestDistance = tempDistance;
		}
	}

	return closestDistance != cFloatMax;
}
