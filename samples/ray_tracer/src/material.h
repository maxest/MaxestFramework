#pragma once


#include "brdf.h"

#include "../../../src/essentials/stl.h"
#include "../../../src/essentials/string.h"
#include "../../../src/essentials/coding.h"
#include "../../../src/math/main.h"


using namespace NMaxestFramework;
using namespace NMath;


namespace NRayTracer
{
	class CMaterial
	{
	public:
		CMaterial()
		{
			diffuseBRDF = nullptr;
			specularBRDF = nullptr;
		}

		inline SVector3 BRDF(const SVector3& wi, const SVector3& wo, const SVector3& normal) const
		{
			return diffuseBRDF->f(wi, wo, normal) + specularBRDF->f(wi, wo, normal);
		}

	public:
		CBRDF* diffuseBRDF;
		CBRDF* specularBRDF;
		float transmittance;
		float transmittanceEta;
		float reflectivity;
	};
}
