#include "ray_tracer.h"


bool NRayTracer::SceneIntersection_Primary(const SScene& scene, const SVector3& rayStart, const SVector3& rayDir, float maxDistance, SSceneIntersectionResult& sceneIntersectionResult)
{
	float closestDistance = cFloatMax;
	SVector3 tempIntersectionPoint;
	float tempDistance;

	for (uint i = 0; i < scene.triangles.size(); i++)
	{
		if (IntersectionRayTriangle(rayStart, rayDir, scene.triangles[i].p1, scene.triangles[i].p2, scene.triangles[i].p3, tempIntersectionPoint, tempDistance))
		{
			if (tempDistance < 0.0f)
				continue;
			if (tempDistance > closestDistance)
				continue;
			if (tempDistance > maxDistance)
				continue;
			// backface culling
			if (Dot(rayDir, scene.triangles[i].normal) > 0.0f)
				continue;

			sceneIntersectionResult.materialIndex = scene.triangles[i].materialIndex;
			sceneIntersectionResult.point = tempIntersectionPoint;
			sceneIntersectionResult.normal = scene.triangles[i].normal;
			sceneIntersectionResult.backside = false;
			sceneIntersectionResult.triangleIndex = i;

			closestDistance = tempDistance;
		}
	}

	for (uint i = 0; i < scene.spheres.size(); i++)
	{
		if (IntersectionRaySphere(rayStart, rayDir, scene.spheres[i].position, scene.spheres[i].radius, tempIntersectionPoint, tempDistance))
		{
			if (tempDistance < 0.0f)
				continue;
			if (tempDistance > closestDistance)
				continue;
			if (tempDistance > maxDistance)
				continue;

			sceneIntersectionResult.materialIndex = scene.spheres[i].materialIndex;
			sceneIntersectionResult.point = tempIntersectionPoint;
			sceneIntersectionResult.normal = 1.0f / scene.spheres[i].radius * (tempIntersectionPoint - scene.spheres[i].position);
			sceneIntersectionResult.backside = false;
			sceneIntersectionResult.triangleIndex = -1;

			// are we inside the sphere?
			if (Distance(rayStart, scene.spheres[i].position) < scene.spheres[i].radius)
			{
				sceneIntersectionResult.normal = -sceneIntersectionResult.normal;
				sceneIntersectionResult.backside = true;
			}

			closestDistance = tempDistance;
		}
	}

	return closestDistance != cFloatMax;
}


bool NRayTracer::SceneIntersection_Shadow(const SScene& scene, const SVector3& rayStart, const SVector3& rayDir, float maxDistance, int triangleIndex)
{
	float closestDistance = cFloatMax;
	SVector3 tempIntersectionPoint;
	float tempDistance;

	for (uint i = 0; i < scene.triangles.size(); i++)
	{
		if ((int)i == triangleIndex) // no self-occlusion please
			continue;

		if (IntersectionRayTriangle(rayStart, rayDir, scene.triangles[i].p1, scene.triangles[i].p2, scene.triangles[i].p3, tempIntersectionPoint, tempDistance))
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
			if (Dot(rayDir, scene.triangles[i].normal) > 0.0f)
				continue;

			closestDistance = tempDistance;
		}
	}

	for (uint i = 0; i < scene.spheres.size(); i++)
	{
		// sphere uses offset instead of "triangleIndex" because sphere can occlude itself
		if (IntersectionRaySphere(rayStart + 0.001f*rayDir, rayDir, scene.spheres[i].position, scene.spheres[i].radius, tempIntersectionPoint, tempDistance))
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


SVector3 NRayTracer::SceneRadiance_Recursive(const SScene& scene, int samplesSetIndex, const SVector3& rayStart, const SVector3& rayDir, int depth, int maxDepth)
{
	SVector3 radiance = cVector3Zero;
	SSceneIntersectionResult sir;

	if (depth == maxDepth)
		return radiance;

	if (SceneIntersection_Primary(scene, rayStart, rayDir, cFloatMax, sir))
	{
		const SMaterial& material = scene.materials[sir.materialIndex];
		SMatrix worldToTangent = WorldToTangent(sir.normal);
		SMatrix tangentToWorld = Transpose(worldToTangent);
		SVector3& point = sir.point;
		SVector3 wo = -Normalize(rayDir);
		SVector3 wo_tangent = wo * worldToTangent;
		SVector3& normal = sir.normal;
		SVector3 normal_tangent = sir.normal * worldToTangent;
		bool& backside = sir.backside;
		int32& triangleIndex = sir.triangleIndex;

		MF_ASSERT(material.diffuseBRDF != nullptr);
		MF_ASSERT(material.specularBRDF != nullptr);

		// constant ambient
		if (depth == 0) // this is a fake effect and seems to make sense to be applied only once (not recursively)
			radiance += scene.ambientConst * material.diffuseBRDF->rho();

		// ambient occlusion
		if (scene.ambientOcclusionFactor > 0.0f)
		{
			SVector3 brdf = material.diffuseBRDF->f(cVector3Zero, cVector3Zero, cVector3Zero); // assume the surface to be Lambertian; for Lambertian the input params to f are not used

			float ambientOcclusion = 0.0f;
			int samplesCount = (int)scene.samples_hemisphere1_cartesian[samplesSetIndex].size();
			for (int i = 0; i < samplesCount; i++)
			{
				SVector3 wi_tangent = DecodeHemisphericalCartesian(scene.samples_hemisphere1_cartesian[samplesSetIndex][i]);
				SVector3 wi = wi_tangent * tangentToWorld;

				if (!SceneIntersection_Shadow(scene, point + 0.001f*wi, wi, cFloatMax, triangleIndex))
				{
					float NdotL = Dot(wi, normal); // should never be zero because the samples should never be perfectly parallel to the surface
					float pdf = NdotL / cPi; // samples used are cosine-weighted

					ambientOcclusion += NdotL / pdf;
				}
			}
			ambientOcclusion /= (float)samplesCount;

			radiance += scene.ambientOcclusionFactor * brdf * ambientOcclusion;
		}

		// direct illumination
		{
			// dir lights
			for (uint i = 0; i < scene.dirLights.size(); i++)
			{
				const SDirLight& light = scene.dirLights[i];

				SVector3 wi = -light.dir;
				SVector3 wi_tangent = Normalize(wi * worldToTangent);

				if (!SceneIntersection_Shadow(scene, point, wi, cFloatMax, triangleIndex))
				{
					float NdotL = Saturate(Dot(wi, normal));

					radiance +=
						light.color *
						(material.diffuseBRDF->f(wi_tangent, wo_tangent, normal_tangent) +
						material.specularBRDF->f(wi_tangent, wo_tangent, normal_tangent)) *
						NdotL;
				}
			}

			// point lights
			for (uint i = 0; i < scene.pointLights.size(); i++)
			{
				const SPointLight& light = scene.pointLights[i];

				SVector3 wi = light.position - point;
				float distanceToLight = Length(wi);
				wi = wi / distanceToLight;
				SVector3 wi_tangent = Normalize(wi * worldToTangent);

				if (!SceneIntersection_Shadow(scene, point, wi, 1.1f*distanceToLight, triangleIndex))
				{
					float NdotL = Saturate(Dot(wi, normal));

					radiance +=
						light.color * (1.0f / Sqr(distanceToLight)) *
						(material.diffuseBRDF->f(wi_tangent, wo_tangent, normal_tangent) +
						material.specularBRDF->f(wi_tangent, wo_tangent, normal_tangent)) *
						NdotL;
				}
			}
		}

		// indirect illumination
		{
			float transmittance = material.transmittance;
			float reflectivity = material.reflectivity;

			if (transmittance > 0.0f)
			{
				float eta = material.transmittanceEta;
				float NdotV = Dot(wo, normal);

				if (backside)
					eta = 1.0f / eta;

				float NdotT = 1.0f - (1.0f - Sqr(NdotV)) / Sqr(eta);

				if (NdotT >= 0.0f)
				{
					NdotT = Sqrt(NdotT);
					SVector3 wi = -wo/eta - (NdotT - NdotV/eta) * normal;

					radiance +=
						transmittance /
						Sqr(eta) *
						SceneRadiance_Recursive(scene, samplesSetIndex, point + 0.001f*wi, wi, depth + 1, maxDepth);
				}
				else // total internal reflection
				{
					reflectivity += transmittance;
				}
			}

			if (reflectivity > 0.0f)
			{
				SVector3 wi = Reflect(-wo, normal);

				radiance +=
					reflectivity *
					SceneRadiance_Recursive(scene, samplesSetIndex, point + 0.001f*wi, wi, depth + 1, maxDepth);
			}
		}
		/*
		// GI
		for (uint i = 0; i < scene.samples_hemisphere1[samplesSetIndex].size(); i++)
		{
			SVector3 wi = SphericalToCartesian(scene.samples_hemisphere1[samplesSetIndex][i]);
			wi = wi * tangentToWorld;

			radiance +=
				SceneRadiance_Recursive(scene, samplesSetIndex, sir.point + 0.001f*wi, wi, depth + 1, maxDepth) /
				(float)scene.samples_hemisphere1[samplesSetIndex].size();
		}*/
	}

	return radiance;
}


void NRayTracer::SceneAddMesh(SScene& scene, const NMesh::SMesh& mesh, const SMatrix& transform, int materialIndex)
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

			scene.triangles.push_back(triangle);
		}
	}
}
