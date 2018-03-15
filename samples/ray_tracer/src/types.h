#pragma once


#include "../../../src/essentials/stl.h"
#include "../../../src/math/main.h"


using namespace NMaxestFramework;
using namespace NMath;


namespace NRayTracer
{
	class CBRDF;

	struct SPrimitive
	{
		int32 materialIndex;
	};
	struct STrianglePrimitive: public SPrimitive
	{
		SVector3 p1;
		SVector3 p2;
		SVector3 p3;
		SVector3 normal;
	};
	struct SSpherePrimitive: public SPrimitive
	{
		SVector3 position;
		float radius;
	};

	struct SMaterial
	{
		CBRDF* diffuseBRDF;
		CBRDF* specularBRDF;
		float transmittance;
		float transmittanceEta;
		float reflectivity;

		SMaterial()
		{
			diffuseBRDF = nullptr;
			specularBRDF = nullptr;
		}
	};

	struct SLight
	{
		SVector3 color;
	};
	struct SDirLight: public SLight
	{
		SVector3 dir;
	};
	struct SPointLight: public SLight
	{
		SVector3 position;
	};

	struct SScene
	{
		vector<STrianglePrimitive> triangles;
		vector<SSpherePrimitive> spheres;
		vector<SMaterial> materials;
		vector<SDirLight> dirLights;
		vector<SPointLight> pointLights;
		vector< vector<SSpherical> > samples_hemisphere1;

		float ambientLightingScale;
	};

	struct SSceneIntersectionResult
	{
		int32 materialIndex;
		SVector3 point;
		SVector3 normal;
		bool backside;
		int32 triangleIndex;
	};
}
