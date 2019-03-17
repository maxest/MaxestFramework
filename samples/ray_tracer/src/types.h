#pragma once


#include "../../../src/essentials/stl.h"
#include "../../../src/essentials/string.h"
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

	struct SScene //!! (sample i ten Create do klasy CRayTracer)
	{
		vector<STrianglePrimitive> triangles;
		vector<SSpherePrimitive> spheres;
		vector<SMaterial> materials;
		vector<SDirLight> dirLights;
		vector<SPointLight> pointLights;
		vector< vector<SVector2> > samples_hemisphere1_cartesian;

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
					file.ReadBin((char*)&samples_hemisphere1_cartesian[i][0], sizeof(SVector2) * samplesCount);
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
						SVector3 sample_cartesian = SphericalToCartesian(samples_hemispherical[j]);
						samples_hemisphere1_cartesian[i].push_back(VectorCustom(sample_cartesian.x, sample_cartesian.y));
					}
				}

				MF_ASSERT(file.Open(path, NEssentials::CFile::EOpenMode::WriteBinary));
				for (int i = 0; i < width * height; i++)
				{
					uint samplesCount = (uint)samples_hemisphere1_cartesian[i].size();
					file.WriteBin((char*)&samplesCount, sizeof(uint));
					file.WriteBin((char*)&samples_hemisphere1_cartesian[i][0], sizeof(SVector2) * samplesCount);
				}
				file.Close();
            }
		}
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
