#include "ray_tracer.h"


using namespace NRayTracer;


void CRayTracer::Create(int width, int height, const CScene &scene)
{
	ambientConst = 0.0f;
	ambientOcclusionFactor = 0.0f;

	this->width = width;
	this->height = height;
	this->scene = &scene;
	sampler64.Create(width, height, 8);

	outputData = new uint8[4 * width * height];
}


void CRayTracer::Destroy()
{
	MF_ASSERT(outputData != nullptr);
	delete[] outputData;
	outputData = nullptr;

	scene = nullptr;
}


uint8* CRayTracer::Render(CJobSystem* jobSystem, const NRayTracer::CCamera& camera)
{
	class CRayTraceJob : public CJob
	{
	public:
		CRayTraceJob(uint8* data, int width, int heightMin, int heightMax, const CRayTracer& rayTracer, const NRayTracer::CCamera& camera)
		{
			this->data = data;
			this->width = width;
			this->heightMin = heightMin;
			this->heightMax = heightMax;
			this->rayTracer = &rayTracer;
			this->camera = &camera;
		}

		int Do()
		{
			int samplesCountX = 1;
			int samplesCountY = 1;
			bool dof = false;

			float sampleOffsetX = 1.0f / (float)samplesCountX;
			float sampleOffsetY = 1.0f / (float)samplesCountY;
			float oneOverSamplesCount = 1.0f / (float)(samplesCountX * samplesCountY);

			for (int y = heightMin; y < heightMax; y++)
			{
				for (int x = 0; x < width; x++)
				{
					SVector3 radiance = cVector3Zero;

					float dofDX = RandomFloat(-0.16f, 0.16f);
					float dofDY = RandomFloat(-0.16f, 0.16f);
					for (int j = 0; j < samplesCountY; j++)
					{
						for (int i = 0; i < samplesCountX; i++)
						{
							SVector3 rayStart, rayDir;

							if (dof)
								camera->RayDOF((float)x + i*sampleOffsetX, (float)y + j*sampleOffsetY, dofDX, dofDY, 8.0f, rayStart, rayDir);
							else
								camera->Ray((float)x + i*sampleOffsetX, (float)y + j*sampleOffsetY, rayStart, rayDir);

							radiance += rayTracer->Radiance_Recursive(y*width + x, rayStart, rayDir, 0);
						}
					}

					radiance *= oneOverSamplesCount;

					SetPixel(x, y, radiance);
				}
			}

			return 0;
		}

	private:
		FORCE_INLINE void SetPixel(int x, int y, SVector3& radiance)
		{
			radiance = Clamp(radiance, cVector3Zero, cVector3One);
			//PowIn(radiance, 1.0f/2.2f);

			int index = Idx(x, y, width);
			data[4 * index + 0] = (uint8)(255.0f * radiance.z);
			data[4 * index + 1] = (uint8)(255.0f * radiance.y);
			data[4 * index + 2] = (uint8)(255.0f * radiance.x);
			data[4 * index + 3] = 255;
		}

	private:
		uint8* data;
		int width;
		int heightMin, heightMax;
		const CRayTracer* rayTracer;
		const NRayTracer::CCamera* camera;
	};

	const int jobsCount = 32;
	CRayTraceJob* jobs[jobsCount];
	for (int i = 0; i < jobsCount; i++)
		jobs[i] = new CRayTraceJob(outputData, width, i*height/jobsCount, (i+1)*height/jobsCount, *this, camera);

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

	if (depth == maxRecursionDepth)
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

		// constant ambient
		if (depth == 0) // this is a fake effect and seems to make sense to be applied only once (not recursively)
			radiance += ambientConst * material.LambertianRHO();

		// ambient occlusion
		if (ambientOcclusionFactor > 0.0f)
		{
			float ambientOcclusion = 0.0f;

			uint samplesCount = sampler64.SamplesCount();
			for (uint i = 0; i < samplesCount; i++)
			{
				const SVector3& wi_tangent = sampler64.Get(samplesSetIndex, i);
				SVector3 wi = wi_tangent * tangentToWorld;

				if (!scene->IntersectionShadow(point + 0.001f*wi, wi, cFloatMax, triangleIndex))
				{
					float NdotL = Dot(wi, normal); // should never be zero because the samples should never be perfectly parallel to the surface
					float pdf = NdotL / cPi; // samples used are cosine-weighted

					ambientOcclusion += NdotL / pdf;
				}
			}
			ambientOcclusion /= (float)samplesCount;

			radiance += ambientOcclusionFactor * ambientOcclusion * material.LambertianBRDF(); // assume the surface to be Lambertian
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

				if (!scene->IntersectionShadow(point, wi, distanceToLight, triangleIndex))
				{
					float NdotL = Saturate(Dot(wi, normal));

					radiance +=
						light.color * (1.0f / distanceToLight) *
						material.BRDF(wi_tangent, wo_tangent, normal_tangent) *
						NdotL;
				}
			}
		}

		// GI
		if (globalIllumination)
		{
			for (uint i = 0; i < material.brdfs.size(); i++)
			{
				// TODO
				if (i > 0) // only let 0'th which is assumed (temporarily) to be Lambertian
					break;

				SVector3 brdfRadiance = cVector3Zero;
				uint samplesCount = material.brdfs[i]->SamplesCount();

				for (uint j = 0; j < samplesCount; j++)
				{
					SVector3 wi;
					float NdotWi;					
					float pdf;

					SVector3 brdf = material.brdfs[i]->f_sample(samplesSetIndex, j, tangentToWorld, wo, normal, wi, NdotWi, pdf);

					brdfRadiance += brdf * Radiance_Recursive(samplesSetIndex, sir.point + 0.001f*wi, wi, depth + 1) * NdotWi / pdf;
				}

				brdfRadiance /= (float)samplesCount;
				radiance += brdfRadiance;
			}
		}
		
		// transmittance and reflectivity
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
	}

	return radiance;
}
