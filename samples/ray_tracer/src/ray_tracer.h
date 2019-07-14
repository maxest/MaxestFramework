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

	public:
		int maxRecursionDepth;

		float ambientConst;
		float ambientOcclusionFactor;

		bool globalIllumination;

	private:
		int width, height;
		const CScene* scene;
		CSamplerHemispherical sampler64;

		uint8* outputData; // in RGBA8
	};
}
