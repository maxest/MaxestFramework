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
		vector< vector<SSpherical> > samples_hemisphere1;

		float ambientConst;
		float ambientOcclusionFactor;

		void Create(int samplesCount_sqrt, int width, int height)
		{
			samples_hemisphere1.resize(width * height);
			string path = "samples_hemisphere1_" + NEssentials::ToString(samplesCount_sqrt) + "_" + NEssentials::ToString(width) + "x" + NEssentials::ToString(height) + ".cache";

			NEssentials::CFile file;
            if (file.Open(path, NEssentials::CFile::EOpenMode::ReadBinary))
            {
				for (int i = 0; i < width * height; i++)
				{
					uint samplesCount;
					file.ReadBin((char*)&samplesCount, sizeof(uint));

					vector<SSpherical> samples;
					samples.resize(samplesCount);
					file.ReadBin((char*)&samples[0], sizeof(SSpherical) * samplesCount);

					samples_hemisphere1[i] = samples;
				}
            	file.Close();
            }
            else
            {
				for (int i = 0; i < width * height; i++)
				{
					vector<SVector2> samples = MultiJitteredRectSamples2D(samplesCount_sqrt);
					samples_hemisphere1[i] = MapRectSamplesToHemisphere(samples, 1.0f);
				}

				MF_ASSERT(file.Open(path, NEssentials::CFile::EOpenMode::WriteBinary));
				for (int i = 0; i < width * height; i++)
				{
					uint samplesCount = (uint)samples_hemisphere1[i].size();
					file.WriteBin((char*)&samplesCount, sizeof(uint));
					file.WriteBin((char*)&samples_hemisphere1[i][0], sizeof(SSpherical) * samplesCount);
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
