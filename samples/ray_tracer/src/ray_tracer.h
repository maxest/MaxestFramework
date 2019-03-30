#pragma once


#include "types.h"
#include "brdf.h"
#include "camera.h"
#include "sampler.h"
#include "scene.h"

#include "../../../src/math/main.h"
#include "../../../src/common/jobs.h"
#include "../../../src/mesh/types.h"


using namespace NMaxestFramework;
using namespace NMath;
using namespace NMesh;
using namespace NCommon;


#define MAX_DEPTH	3


namespace NRayTracer
{
	class CRayTracer
	{
	public:
		void Create(int width, int height, const CScene& scene);
		SVector3 Radiance_Recursive(int samplesSetIndex, const SVector3& rayStart, const SVector3& rayDir, int depth) const;

	private:
		const CScene* scene;
		CSamplerHemispherical sampler;
	};

	//

	class CRayTraceJob: public CJob
	{
	public:
		CRayTraceJob(uint8* data, int width, int heightMin, int heightMax, const CRayTracer& rayTracer, const CCamera& camera, bool dof, bool aa)
		{
			this->data = data;
			this->width = width;
			this->heightMin = heightMin;
			this->heightMax = heightMax;
			this->rayTracer = &rayTracer;
			this->camera = &camera;
			this->dof = dof;
			this->aa = aa;
		}

		void Pixel(int x, int y, SVector3& radiance)
		{
			radiance = Clamp(radiance, cVector3Zero, cVector3One);
		//	PowIn(radiance, 1.0f/2.2f);

			int index = Idx(x, y, width);
			data[4 * index + 0] = (uint8)(255.0f * radiance.z);
			data[4 * index + 1] = (uint8)(255.0f * radiance.y);
			data[4 * index + 2] = (uint8)(255.0f * radiance.x);
			data[4 * index + 3] = 255;
		}

		int Standard()
		{
			for (int y = heightMin; y < heightMax; y++)
			{
				for (int x = 0; x < width; x++)
				{
					SVector3 radiance = cVector3Zero;

					SVector3 rayStart, rayDir;
					camera->Ray((float)x, (float)y, rayStart, rayDir);
					radiance += rayTracer->Radiance_Recursive(y*width + x, rayStart, rayDir, 0);

					Pixel(x, y, radiance);
				}
			}

			return 0;
		}

		int DOF()
		{
			for (int y = heightMin; y < heightMax; y++)
			{
				for (int x = 0; x < width; x++)
				{
					SVector3 radiance = cVector3Zero;

					SVector3 rayStart, rayDir;
					camera->RayDOF((float)x, (float)y, RandomFloat(-0.16f, 0.16f), RandomFloat(-0.16f, 0.16f), 8.0f, rayStart, rayDir);
					radiance += rayTracer->Radiance_Recursive(y*width + x, rayStart, rayDir, 0) / 1.0f;

					Pixel(x, y, radiance);
				}
			}

			return 0;
		}

		int AA(int samplesCountX, int samplesCountY)
		{
			float sampleOffsetX = 1.0f / (float)samplesCountX;
			float sampleOffsetY = 1.0f / (float)samplesCountY;
			float oneOverSamplesCount = 1.0f / (float)(samplesCountX * samplesCountY);

			for (int y = heightMin; y < heightMax; y++)
			{
				for (int x = 0; x < width; x++)
				{
					SVector3 radiance = cVector3Zero;

					SVector3 rayStart, rayDir;
					for (int j = 0; j < samplesCountY; j++)
					{
						for (int i = 0; i < samplesCountY; i++)
						{
							camera->Ray((float)x + i*sampleOffsetX, (float)y + j*sampleOffsetY, rayStart, rayDir);
							radiance += rayTracer->Radiance_Recursive(y*width + x, rayStart, rayDir, 0) * oneOverSamplesCount;
						}
					}

					Pixel(x, y, radiance);
				}
			}

			return 0;
		}

		int DOFAndAA(int samplesCountX, int samplesCountY)
		{
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
					SVector3 rayStart, rayDir;
					for (int j = 0; j < samplesCountY; j++)
					{
						for (int i = 0; i < samplesCountY; i++)
						{
							camera->RayDOF((float)x+ i*sampleOffsetX, (float)y + j*sampleOffsetY, dofDX, dofDY, 8.0f, rayStart, rayDir);
							radiance += rayTracer->Radiance_Recursive(y*width + x, rayStart, rayDir, 0) * oneOverSamplesCount;
						}
					}

					Pixel(x, y, radiance);
				}
			}

			return 0;
		}

		int Do()
		{
			if (!dof)
			{
				if (!aa)
					return Standard();
				else
					return AA(3, 3);
			}
			else
			{
				if (!aa)
					return DOF();
				else
					return DOFAndAA(3, 3);
			}
		}

	public:
		uint8* data;
		int width;
		int heightMin, heightMax;
		const CRayTracer* rayTracer;
		const CCamera* camera;
		bool dof;
		bool aa;
	};
}
