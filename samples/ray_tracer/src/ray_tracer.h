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
		CRayTracer()
		{
			scene = nullptr;
			outputData = nullptr;
		}

		void Create(int width, int height, const CScene& scene);
		void Destroy();

		uint8* Render(CJobSystem* jobSystem, const CCamera& camera);

		SVector3 Radiance_Recursive(int samplesSetIndex, const SVector3& rayStart, const SVector3& rayDir, int depth) const;

	private:
		int width, height;
		const CScene* scene;
		CSamplerHemispherical sampler;

		uint8* outputData; // in RGBA8
	};

	//

	class CRayTraceJob: public CJob
	{
	public:
		CRayTraceJob(uint8* data, int width, int heightMin, int heightMax, const CRayTracer& rayTracer, const CCamera& camera)
		{
			this->data = data;
			this->width = width;
			this->heightMin = heightMin;
			this->heightMax = heightMax;
			this->rayTracer = &rayTracer;
			this->camera = &camera;
		}

		void SetPixel(int x, int y, SVector3& radiance)
		{
			radiance = Clamp(radiance, cVector3Zero, cVector3One);
		//	PowIn(radiance, 1.0f/2.2f);

			int index = Idx(x, y, width);
			data[4 * index + 0] = (uint8)(255.0f * radiance.z);
			data[4 * index + 1] = (uint8)(255.0f * radiance.y);
			data[4 * index + 2] = (uint8)(255.0f * radiance.x);
			data[4 * index + 3] = 255;
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
								camera->RayDOF((float)x+ i*sampleOffsetX, (float)y + j*sampleOffsetY, dofDX, dofDY, 8.0f, rayStart, rayDir);
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

	public:
		uint8* data;
		int width;
		int heightMin, heightMax;
		const CRayTracer* rayTracer;
		const CCamera* camera;
	};
}
