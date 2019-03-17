#pragma once


#include "../../../src/essentials/stl.h"
#include "../../../src/essentials/string.h"
#include "../../../src/essentials/coding.h"
#include "../../../src/math/main.h"


using namespace NMaxestFramework;
using namespace NMath;


//#define USE_SAMPLES_HALF_PRECISION // remember to remove cached samples file


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

	struct SSceneIntersectionResult
	{
		int32 materialIndex;
		SVector3 point;
		SVector3 normal;
		bool backside;
		int32 triangleIndex;
	};
	
	struct SHemisphericalCartesian
	{
	#ifndef USE_SAMPLES_HALF_PRECISION
		float x, y;

		void Set(const SVector3& v)
		{
			x = v.x;
			y = v.y;
		}

		SVector3 Get() const
		{
			SVector3 temp;

			temp.x = x;
			temp.y = y;
			temp.z = Sqrt(1.0f - (x*x + y*y));

			return temp;
		}
	#else
		uint16 x, y;

		void Set(const SVector3& v)
		{
			x = NEssentials::FloatToHalf(v.x);
			y = NEssentials::FloatToHalf(v.y);
		}

		SVector3 Get() const
		{
			SVector3 temp;

			temp.x = NEssentials::HalfToFloat(x);
			temp.y = NEssentials::HalfToFloat(y);
			temp.z = Sqrt(1.0f - (temp.x*temp.x + temp.y*temp.y));

			return temp;
		}
	#endif
	};

	struct SScene //!! (sample i ten Create do klasy CRayTracer)
	{
		vector<STrianglePrimitive> triangles;
		vector<SSpherePrimitive> spheres;
		vector<SMaterial> materials;
		vector<SDirLight> dirLights;
		vector<SPointLight> pointLights;
		vector< vector<SHemisphericalCartesian> > samples_hemisphere1_cartesian;

		float ambientConst;
		float ambientOcclusionFactor;

		void Create(int samplesCount_sqrt, int width, int height)
		{
			samples_hemisphere1_cartesian.resize(width * height);
			string path = "samples_hemisphere1_" + NEssentials::ToString(samplesCount_sqrt) + "_" + NEssentials::ToString(width) + "x" + NEssentials::ToString(height) + ".cache";

			NEssentials::CFile file;
            if (file.Open(path, NEssentials::CFile::EOpenMode::ReadBinary))
            {
				for (int i = 0; i < width * height; i++)
				{
					uint samplesCount;
					file.ReadBin((char*)&samplesCount, sizeof(uint));

					samples_hemisphere1_cartesian[i].resize(samplesCount);
					file.ReadBin((char*)&samples_hemisphere1_cartesian[i][0], sizeof(SHemisphericalCartesian) * samplesCount);
				}
            	file.Close();
            }
            else
            {
				for (int i = 0; i < width * height; i++)
				{
					vector<SVector2> samples = MultiJitteredRectSamples2D(samplesCount_sqrt);
					// make sure samples don't end up on edges so we don't have "flat" samples that penetrate the surface
					for (uint j = 0; j < samples.size(); j++)
					{
						if (samples[j].x <= 0.0f)
							samples[j].x = 0.01f;
						if (samples[j].x >= 1.0f)
							samples[j].x = 0.99f;

						if (samples[j].y <= 0.0f)
							samples[j].y = 0.01f;
						if (samples[j].y >= 1.0f)
							samples[j].y = 0.99f;
					}

					vector<SSpherical> samples_hemispherical = MapRectSamplesToHemisphere(samples, 1.0f);

					for (uint j = 0; j < samples_hemispherical.size(); j++)
					{
						SHemisphericalCartesian sample;
						sample.Set(SphericalToCartesian(samples_hemispherical[j]));
						samples_hemisphere1_cartesian[i].push_back(sample);
					}
				}

				MF_ASSERT(file.Open(path, NEssentials::CFile::EOpenMode::WriteBinary));
				for (int i = 0; i < width * height; i++)
				{
					uint samplesCount = (uint)samples_hemisphere1_cartesian[i].size();
					file.WriteBin((char*)&samplesCount, sizeof(uint));
					file.WriteBin((char*)&samples_hemisphere1_cartesian[i][0], sizeof(SHemisphericalCartesian) * samplesCount);
				}
				file.Close();
            }
		}
	};
}
