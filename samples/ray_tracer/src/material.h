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
		CMaterial() {}

		void Destroy()
		{
			for (uint i = 0; i < brdfs.size(); i++)
				delete brdfs[i];

			brdfs.clear();
		}

		void AddBRDF(CBRDF* brdf)
		{
			brdfs.push_back(brdf);
		}

		SVector3 LambertianRHO() const
		{
			return lambertianBRDFAlbedo;
		}

		SVector3 LambertianBRDF() const
		{
			return lambertianBRDFAlbedo * cInvPi;
		}

		SVector3 BRDF(const SVector3& wi, const SVector3& wo, const SVector3& normal) const
		{
			SVector3 brdf = cVector3Zero;

			for (uint i = 0; i < brdfs.size(); i++)
				brdf += brdfs[i]->f(wi, wo, normal);

			return brdf;
		}

	public:
		SVector3 lambertianBRDFAlbedo;
		vector< CBRDF* > brdfs;
		float transmittance;
		float transmittanceEta;
		float reflectivity;
	};
}
