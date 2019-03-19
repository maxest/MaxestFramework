#pragma once


#include "../../../src/essentials/stl.h"
#include "../../../src/essentials/string.h"
#include "../../../src/essentials/coding.h"
#include "../../../src/math/main.h"


using namespace NMaxestFramework;
using namespace NMath;


namespace NRayTracer
{
	class CSamplerHemispherical
	{
	public:
		inline void Create(int width, int height, int samplesCount_sqrt)
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

		inline int SamplesCount() const
		{
			return samples_hemisphere1_cartesian[0].size();
		}

		inline SVector3 Get(int setIndex, int sampleIndex) const
		{
			return samples_hemisphere1_cartesian[setIndex][sampleIndex].Get();
		}

	private:
		vector< vector<SHemisphericalCartesian> > samples_hemisphere1_cartesian; // 2D array n x m of n sets of m samples
	};
}
