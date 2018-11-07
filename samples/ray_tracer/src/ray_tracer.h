#pragma once


#include "types.h"
#include "brdf.h"
#include "camera.h"

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
	bool SceneIntersection_Primary(const SScene& scene, const SVector3& rayStart, const SVector3& rayDir, float maxDistance, SSceneIntersectionResult& sceneIntersectionResult);
	bool SceneIntersection_Shadow(const SScene& scene, const SVector3& rayStart, const SVector3& rayDir, float maxDistance, int triangleIndex);
	SVector3 SceneRadiance_Recursive(const SScene& scene, int samplesSetIndex, const SVector3& rayStart, const SVector3& rayDir, int depth, int maxDepth);
	void SceneAddMesh(SScene& scene, const NMesh::SMesh& mesh, const SMatrix& transform, int materialIndex);

	//

	class CRayTraceJob: public CJob
	{
	public:
		CRayTraceJob(uint8* data, int width, int heightMin, int heightMax, const SScene& scene, const SCamera& camera, bool dof, bool aa)
		{
			this->data = data;
			this->width = width;
			this->heightMin = heightMin;
			this->heightMax = heightMax;
			this->scene = &scene;
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
					RayPerspective(*camera, (float)x, (float)y, rayStart, rayDir);
					radiance += SceneRadiance_Recursive(*scene, y*width + x, rayStart, rayDir, 0, MAX_DEPTH) / 1.0f;

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
					RayPerspectiveDOF(*camera, (float)x, (float)y, RandomFloat(-0.16f, 0.16f), RandomFloat(-0.16f, 0.16f), 8.0f, rayStart, rayDir);
					radiance += SceneRadiance_Recursive(*scene, y*width + x, rayStart, rayDir, 0, MAX_DEPTH) / 1.0f;

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
							RayPerspective(*camera, (float)x + i*sampleOffsetX, (float)y + j*sampleOffsetY, rayStart, rayDir);
							radiance += SceneRadiance_Recursive(*scene, y*width + x, rayStart, rayDir, 0, MAX_DEPTH) * oneOverSamplesCount;
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
							RayPerspectiveDOF(*camera, (float)x+ i*sampleOffsetX, (float)y + j*sampleOffsetY, dofDX, dofDY, 8.0f, rayStart, rayDir);
							radiance += SceneRadiance_Recursive(*scene, y*width + x, rayStart, rayDir, 0, MAX_DEPTH) * oneOverSamplesCount;
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
		const SScene* scene;
		const SCamera* camera;
		bool dof;
		bool aa;
	};
}
