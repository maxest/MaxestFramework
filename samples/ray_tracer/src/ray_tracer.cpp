#include "ray_tracer.h"


using namespace NRayTracer;


void CRayTracer::Create(int width, int height, const CScene &scene)
{
	this->width = width;
	this->height = height;
	this->scene = &scene;
	sampler.Create(width, height, 8);

	outputData = new uint8[4 * width * height];
}


void CRayTracer::Destroy()
{
	MF_ASSERT(outputData != nullptr);
	delete[] outputData;
	outputData = nullptr;

	scene = nullptr;
}


uint8* CRayTracer::Render(CJobSystem* jobSystem, const CCamera& camera, bool dof, bool aa)
{
	const int jobsCount = 32;
	CRayTraceJob* jobs[jobsCount];
	for (int i = 0; i < jobsCount; i++)
		jobs[i] = new CRayTraceJob(outputData, width, i*height/jobsCount, (i+1)*height/jobsCount, *this, camera, dof, aa);

	if (jobSystem == nullptr)
	{
		for (int i = 0; i < jobsCount; i++)
			jobs[i]->Do();
	}
	else
	{
		CJobGroup jobGroup;
		for (int i = 0; i < jobsCount; i++)
			jobGroup.AddJob(jobs[i]);
		jobSystem->AddJobGroup(jobGroup);
		jobGroup.Wait();
	}

	for (int i = 0; i < jobsCount; i++)
		delete jobs[i];

	return outputData;
}


SVector3 CRayTracer::Radiance_Recursive(int samplesSetIndex, const SVector3& rayStart, const SVector3& rayDir, int depth) const
{
	SVector3 radiance = cVector3Zero;
	SSceneIntersectionResult sir;

	if (depth == MAX_DEPTH)
		return radiance;

	if (scene->IntersectionPrimary(rayStart, rayDir, cFloatMax, sir))
	{
		const CMaterial& material = scene->materials[sir.materialIndex];
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
			radiance += scene->ambientConst * material.diffuseBRDF->rho();

		// ambient occlusion
		if (scene->ambientOcclusionFactor > 0.0f)
		{
			SVector3 brdf = material.diffuseBRDF->f(cVector3Zero, cVector3Zero, cVector3Zero); // assume the surface to be Lambertian; for Lambertian the input params to f are not used

			float ambientOcclusion = 0.0f;
			int samplesCount = sampler.SamplesCount();
			for (int i = 0; i < samplesCount; i++)
			{
				const SVector3& wi_tangent = sampler.Get(samplesSetIndex, i);
				SVector3 wi = wi_tangent * tangentToWorld;

				if (!scene->IntersectionShadow(point + 0.001f*wi, wi, cFloatMax, triangleIndex))
				{
					float NdotL = Dot(wi, normal); // should never be zero because the samples should never be perfectly parallel to the surface
					float pdf = NdotL / cPi; // samples used are cosine-weighted

					ambientOcclusion += NdotL / pdf;
				}
			}
			ambientOcclusion /= (float)samplesCount;

			radiance += scene->ambientOcclusionFactor * brdf * ambientOcclusion;
		}

		// direct illumination
		{
			// dir lights
			for (uint i = 0; i < scene->dirLights.size(); i++)
			{
				const SDirLight& light = scene->dirLights[i];

				SVector3 wi = -light.dir;
				SVector3 wi_tangent = Normalize(wi * worldToTangent);

				if (!scene->IntersectionShadow(point, wi, cFloatMax, triangleIndex))
				{
					float NdotL = Saturate(Dot(wi, normal));

					radiance +=
						light.color *
						material.BRDF(wi_tangent, wo_tangent, normal_tangent) *
						NdotL;
				}
			}

			// point lights
			for (uint i = 0; i < scene->pointLights.size(); i++)
			{
				const SPointLight& light = scene->pointLights[i];

				SVector3 wi = light.position - point;
				float distanceToLight = Length(wi);
				wi = wi / distanceToLight;
				SVector3 wi_tangent = Normalize(wi * worldToTangent);

				if (!scene->IntersectionShadow(point, wi, 1.1f*distanceToLight, triangleIndex))
				{
					float NdotL = Saturate(Dot(wi, normal));

					radiance +=
						light.color * (1.0f / Sqr(distanceToLight)) *
						material.BRDF(wi_tangent, wo_tangent, normal_tangent) *
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
						Radiance_Recursive(samplesSetIndex, point + 0.001f*wi, wi, depth + 1);
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
					Radiance_Recursive(samplesSetIndex, point + 0.001f*wi, wi, depth + 1);
			}
		}
		/*
		// GI
		for (uint i = 0; i < scene->samples_hemisphere1[samplesSetIndex].size(); i++)
		{
			SVector3 wi = SphericalToCartesian(scene->samples_hemisphere1[samplesSetIndex][i]);
			wi = wi * tangentToWorld;

			radiance +=
				Radiance_Recursive(scene, samplesSetIndex, sir.point + 0.001f*wi, wi, depth + 1, maxDepth) /
				(float)scene->samples_hemisphere1[samplesSetIndex].size();
		}*/
	}

	return radiance;
}
