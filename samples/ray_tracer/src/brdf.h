#pragma once


#include "types.h"

#include "../../../src/essentials/common.h"
#include "../../../src/math/main.h"


using namespace NMaxestFramework;
using namespace NEssentials;
using namespace NMath;


namespace NRayTracer
{
	inline float CosTheta(const SVector3& v) { return v.z; }

	inline float SinThetaSqr(const SVector3& v) { return Max(0.0f, 1.0f - Sqr(CosTheta(v))); }

	inline float SinTheta(const SVector3& v) { return Sqrt(SinThetaSqr(v)); }

	inline float CosPhi(const SVector3& v)
	{
		float sinTheta = SinTheta(v);
		if (sinTheta == 0.0f)
			return 1.0f;
		return Clamp(v.x / sinTheta, -1.0f, 1.0f);
	}

	inline float SinPhi(const SVector3& v)
	{
		float sinTheta = SinTheta(v);
		if (sinTheta == 0.0f)
			return 1.0f;
		return Clamp(v.y / sinTheta, -1.0f, 1.0f);
	}

	inline SMatrix WorldToTangent(const SVector3& normal)
	{
		SVector3 random = Normalize(VectorCustom(1.0f, 1.0f, 1.0f));
		SVector3 tangent = Normalize(Cross(normal, random));
		SVector3 bitangent = Normalize(Cross(normal, tangent));

		SMatrix worldToTangent;
		worldToTangent.m[0][0] = tangent.x;
		worldToTangent.m[1][0] = tangent.y;
		worldToTangent.m[2][0] = tangent.z;
		worldToTangent.m[0][1] = bitangent.x;
		worldToTangent.m[1][1] = bitangent.y;
		worldToTangent.m[2][1] = bitangent.z;
		worldToTangent.m[0][2] = normal.x;
		worldToTangent.m[1][2] = normal.y;
		worldToTangent.m[2][2] = normal.z;

		return worldToTangent;
	}


	class CBRDF
	{
	public:
		virtual SVector3 f(const SVector3& wi, const SVector3& wo, const SVector3& normal) = 0;
		virtual SVector3 rho() = 0;
	};


	class CLambertianBRDF: public CBRDF
	{
	public:
		CLambertianBRDF() {}
		CLambertianBRDF(const SVector3& albedo) { this->albedo = albedo; }

		SVector3 f(const SVector3& wi, const SVector3& wo, const SVector3& normal)
		{
			return albedo / cPi;
		}

		SVector3 rho()
		{
			return albedo;
		}

	public:
		SVector3 albedo;
	};


	class COrenNayarBRDF: public CBRDF
	{
	public:
		COrenNayarBRDF(float sigma)
		{
			float sigma2 = sigma*sigma;
			_A = 1.f - (sigma2 / (2.f * (sigma2 + 0.33f)));
			_B = 0.45f * sigma2 / (sigma2 + 0.09f);
		}
		COrenNayarBRDF(float sigma, const SVector3& albedo): COrenNayarBRDF(sigma)	
		{
			this->albedo = albedo;
		}

		float f(const SVector3& wi, const SVector3& wo, const SSceneIntersectionResult& sir)
		{
			float sinthetai = SinTheta(wi);
			float sinthetao = SinTheta(wo);
			// Compute cosine term of Oren-Nayar model
			float maxcos = 0.f;
			if (sinthetai > 1e-4 && sinthetao > 1e-4)
			{
				float sinphii = SinPhi(wi);
				float cosphii = CosPhi(wi);
				float sinphio = SinPhi(wo);
				float cosphio = CosPhi(wo);
				float dcos = cosphii * cosphio + sinphii * sinphio;
				maxcos = Max(0.f, dcos);
			}

			// Compute sine and tangent terms of Oren-Nayar model
			float sinalpha, tanbeta;
			if (Abs(CosTheta(wi)) > Abs(CosTheta(wo)))
			{
				sinalpha = sinthetao;
				tanbeta = sinthetai / Abs(CosTheta(wi));
			}
			else
			{
				sinalpha = sinthetai;
				tanbeta = sinthetao / Abs(CosTheta(wo));
			}
			return (_A + _B * maxcos * sinalpha * tanbeta) / cPi;
		}

		SVector3 rho()
		{
			return albedo;
		}

	public:
		SVector3 albedo;

	public:
		float _A, _B;
	};


	class CGlossySpecularBRDF: public CBRDF
	{
	public:
		CGlossySpecularBRDF(float exponent) { this->exponent = exponent; }

		SVector3 f(const SVector3& wi, const SVector3& wo, const SVector3& normal)
		{
			if (exponent > 0.0f)
			{
				float NdotWi = Dot(normal, wi);
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

		SVector3 rho()
		{
			return cVector3Zero;
		}

	public:
		float exponent;
	};
}
