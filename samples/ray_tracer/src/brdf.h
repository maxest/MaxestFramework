#pragma once


#include "types.h"
#include "utils.h"
#include "sampler.h"

#include "../../../src/essentials/common.h"
#include "../../../src/math/main.h"


using namespace NMaxestFramework;
using namespace NEssentials;
using namespace NMath;


namespace NRayTracer
{
	class CBRDF
	{
	public:
        virtual ~CBRDF() {}

		virtual SVector3 f(const SVector3& wi, const SVector3& wo, const SVector3& normal) = 0;
		virtual SVector3 f_sample(int samplesSetIndex, int sampleIndex, const SMatrix& tangentToWorld, const SVector3& wo, const SVector3& normal, SVector3& wi, float& NdotWi, float& pdf) = 0;
		virtual SVector3 rho() = 0;

		int SamplesCount() { return sampler64->SamplesCount(); }

	public:
		CSamplerHemispherical* sampler64;
	};

	//

	class CLambertianBRDF: public CBRDF
	{
	public:
		CLambertianBRDF() {}
		CLambertianBRDF(const SVector3& albedo) { this->albedo = albedo; }

		SVector3 f(const SVector3& wi, const SVector3& wo, const SVector3& normal)
		{
			UNUSED(wi);
			UNUSED(wo);
			UNUSED(normal);

			return albedo / cPi;
		}

		SVector3 f_sample(int samplesSetIndex, int sampleIndex, const SMatrix& tangentToWorld, const SVector3& wo, const SVector3& normal, SVector3& wi, float& NdotWi, float& pdf)
		{
			const SVector3& wi_tangent = sampler64->Get(samplesSetIndex, sampleIndex);
			wi = wi_tangent * tangentToWorld;

			NdotWi = Dot(wi, normal); // should never be zero because the samples should never be perfectly parallel to the surface
			pdf = NdotWi / cPi; // samples used are cosine-weighted

			return albedo * cInvPi;
		}

		SVector3 rho()
		{
			return albedo;
		}

	public:
		SVector3 albedo;
	};

	//

	class CGlossySpecularBRDF: public CBRDF
	{
	public:
		CGlossySpecularBRDF(float exponent) { this->exponent = exponent; }

		SVector3 f(const SVector3& wi, const SVector3& wo, const SVector3& normal)
		{
			if (exponent > 0.0f)
			{
				SVector3 r = Reflect(-wi, normal);
				float RdotWo = Dot(r, wo);

				SVector3 l = cVector3Zero;
				if (RdotWo > 0.0f)
					l = cVector3One * Pow(RdotWo, exponent);

				return l;
			}
			else
			{
				return cVector3Zero;
			}
		}

		SVector3 f_sample(int samplesSetIndex, int sampleIndex, const SMatrix& tangentToWorld, const SVector3& wo, const SVector3& normal, SVector3& wi, float& NdotWi, float& pdf)
		{
			return cVector3Zero; // TODO
		}

		SVector3 rho()
		{
			return cVector3Zero;
		}

	public:
		float exponent;
	};
}
