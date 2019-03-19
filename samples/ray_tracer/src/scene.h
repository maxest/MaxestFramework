#pragma once


#include "material.h"

#include "../../../src/essentials/stl.h"
#include "../../../src/essentials/string.h"
#include "../../../src/essentials/coding.h"
#include "../../../src/math/main.h"


using namespace NMaxestFramework;
using namespace NMath;


namespace NRayTracer
{
	struct SScene
	{
		vector<STrianglePrimitive> triangles;
		vector<SSpherePrimitive> spheres;
		vector<CMaterial> materials;
		vector<SDirLight> dirLights;
		vector<SPointLight> pointLights;

		float ambientConst;
		float ambientOcclusionFactor;
	};
}
