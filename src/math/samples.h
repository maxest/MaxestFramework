#pragma once


#include "types.h"
#include "vector.h"
#include "../essentials/main.h"
#include "../system/file.h"


namespace NMaxestFramework { namespace NMath
{
	// disk
	vector<SVector2> VogelDiskSamples(uint n);
	vector<SVector2> AlchemySpiralDiskSamples(uint n, uint spiralsCount = 7);
	vector<SVector2> BlueNoiseDiskSamples(uint n);

	// rect - all in [0, 1] x [0, 1] range
	vector<SVector2> RandomRectSamples2D(uint sqrtN);
	vector<SVector2> BlueNoiseRectSamples2D(uint n);
	vector<SVector2> JitteredRectSamples2D(uint sqrtN);
	vector<SVector2> MultiJitteredRectSamples2D(uint sqrtN);

	// conversion
	void Range01ToRange11(vector<SVector2>& samples); // [0, 1] x [0, 1] to [-1, 1,] x [-1, 1]
	void Range11ToRange01(vector<SVector2>& samples); // [-1, 1] x [-1, 1] to [0, 1,] x [0, 1]

	// remapping
	vector<SVector2> MapRectSamplesToDisk(const vector<SVector2>& samples); // samples should be in [-1, 1] x [-1, 1] range
	vector<SSpherical> MapRectSamplesToHemisphere(const vector<SVector2>& samples, float e); // samples should be in [0, 1] x [0, 1] range

	// helper
	void DumpSamples(const string& path, const vector<SVector2>& samples);

	//

	inline vector<SVector2> VogelDiskSamples(uint n)
	{
		// https://www.shadertoy.com/view/XtXXDN

		vector<SVector2> samples;
		samples.resize(n);

		for (uint i = 0; i < n; i++)
		{
			float r = sqrt(i + 0.5f) / sqrt((float)n);
			float theta = i * cGoldenAngle;

			samples[i].x = r * Cos(theta);
			samples[i].y = r * Sin(theta);
		}

		return samples;
	}

	inline vector<SVector2> AlchemySpiralDiskSamples(uint n, uint spiralsCount)
	{
		vector<SVector2> samples;
		samples.resize(n);

		for (uint i = 0; i < n; i++)
		{
			float alpha = float(i + 0.5f) / (float)n;
			float theta = spiralsCount * cTwoPi * alpha;

			samples[i].x = Cos(theta);
			samples[i].y = Sin(theta);
		}

		return samples;
	}

	inline vector<SVector2> BlueNoiseDiskSamples(uint n)
	{
		vector<SVector2> samples = BlueNoiseRectSamples2D(n);
		Range01ToRange11(samples);
		return MapRectSamplesToDisk(samples);
	}

	inline vector<SVector2> RandomRectSamples2D(uint sqrtN)
	{
		uint n = sqrtN * sqrtN;

		vector<SVector2> samples;
		samples.resize(n);

		for (uint i = 0; i < n; i++)
		{
			samples[i].x = RandomFloat();
			samples[i].y = RandomFloat();
		}

		return samples;
	}
	
	inline vector<SVector2> BlueNoiseRectSamples2D(uint n)
	{
		// https://blog.demofox.org/2017/10/20/generating-blue-noise-sample-points-with-mitchells-best-candidate-algorithm/

		vector<SVector2> samples;
		samples.push_back(VectorCustom(RandomFloat(), RandomFloat()));

		for (uint i = 1; i < n; i++)
		{
			SVector2 bestPos = VectorCustom(0.0f, 0.0f);
			float bestMinDist = 0.0f;

			int candidatesCount = (int)samples.size() + 1;
			for (int j = 0; j < candidatesCount; j++)
			{
				SVector2 candidatePos = VectorCustom(RandomFloat(), RandomFloat());

				float candidateMinDistToExistingSamples = cFloatMax;
				for (uint k = 0; k < samples.size(); k++)
				{
					float dist = Distance_RectWarped(candidatePos, samples[k], 1.0f);
					candidateMinDistToExistingSamples = NEssentials::Min(candidateMinDistToExistingSamples, dist);
				}

				if (candidateMinDistToExistingSamples > bestMinDist)
				{					
					bestPos.x = candidatePos.x;
					bestPos.y = candidatePos.y;
					bestMinDist = candidateMinDistToExistingSamples;
				}
			}

			samples.push_back(bestPos);
		}

		return samples;
	}

	inline vector<SVector2> JitteredRectSamples2D(uint sqrtN)
	{
		int n = sqrtN * sqrtN;
		float oneOverSqrtN = 1.0f / (float)sqrtN;

		vector<SVector2> samples;
		samples.resize(n);

		for (uint y = 0; y < sqrtN; y++)
		{
			for (uint x = 0; x < sqrtN; x++)
			{
				int index = y*sqrtN + x;

				samples[index].x = oneOverSqrtN * (x + RandomFloat());
				samples[index].y = oneOverSqrtN * (y + RandomFloat());
			}
		}

		return samples;
	}

	inline vector<SVector2> MultiJitteredRectSamples2D(uint sqrtN)
	{
		// http://graphics.pixar.com/library/MultiJitteredSampling/paper.pdf

		int n = sqrtN * sqrtN;
		float oneOverN = 1.0f / (float)n;
		float oneOverSqrtN = 1.0f / (float)sqrtN;

		vector<SVector2> samples;
		samples.resize(n);

		// canonical arrangement
		for (uint j = 0; j < sqrtN; j++)
		{
			for (uint i = 0; i < sqrtN; i++)
			{
				int index = NEssentials::Idx(i, j, sqrtN);

				samples[index].x = ((float)i + ((float)j + RandomFloat()) * oneOverSqrtN) * oneOverSqrtN;
				samples[index].y = ((float)j + ((float)i + RandomFloat()) * oneOverSqrtN) * oneOverSqrtN;
			}
		}

		// shuffle x
		for (uint j = 0; j < sqrtN; j++)
		{
			int k = j + RandomSystem(0, sqrtN - j - 1);

			for (uint i = 0; i < sqrtN; i++)
				Swap(samples[j*sqrtN + i].x, samples[k*sqrtN + i].x);
		}

		// shuffle y
		for (uint i = 0; i < sqrtN; i++)
		{
			int k = i + RandomSystem(0, sqrtN - i - 1);

			for (uint j = 0; j < sqrtN; j++)
				Swap(samples[j*sqrtN + i].y, samples[j*sqrtN + k].y);
		}

		return samples;
	}

	inline void Range01ToRange11(vector<SVector2>& samples)
	{
		for (uint i = 0; i < samples.size(); i++)
		{
			samples[i].x = 2.0f*samples[i].x - 1.0f;
			samples[i].y = 2.0f*samples[i].y - 1.0f;
		}
	}

	inline void Range11ToRange01(vector<SVector2>& samples)
	{
		for (uint i = 0; i < samples.size(); i++)
		{
			samples[i].x = 0.5f*samples[i].x + 0.5f;
			samples[i].y = 0.5f*samples[i].y + 0.5f;
		}
	}

	inline vector<SVector2> MapRectSamplesToDisk(const vector<SVector2>& samples)
	{
		vector<SVector2> samples2;
		samples2.resize(samples.size());

		for (uint i = 0; i < samples.size(); i++)
		{
			SVector2 sp = samples[i];
			float r, phi;

			if (sp.x > -sp.y)
			{
				if (sp.x > sp.y)
				{
					r = sp.x;
					phi = sp.y / sp.x;
				}
				else
				{
					r = sp.y;
					phi = 2.0f - sp.x / sp.y;
				}
			}
			else
			{
				if (sp.x < sp.y)
				{
					r = -sp.x;
					phi = 4.0f + sp.y / sp.x;
				}
				else
				{
					r = -sp.y;
					if (sp.y != 0.0f)
						phi = 6.0f - sp.x / sp.y;
					else
						phi = 0.0f;
				}
			}

			phi *= cPi / 4.0f;

			samples2[i].x = r * Cos(phi);
			samples2[i].y = r * Sin(phi);
		}

		return samples2;
	}

	inline vector<SSpherical> MapRectSamplesToHemisphere(const vector<SVector2>& samples, float e)
	{
		vector<SSpherical> samples2;
		samples2.resize(samples.size());

		e = 1.0f / (e + 1.0f);

		for (uint i = 0; i < samples.size(); i++)
		{
			samples2[i].phi = 2.0f * cPi * samples[i].x;
			samples2[i].theta = ACos(Pow(1.0f - samples[i].y, e));
		}

		return samples2;
	}

	inline void DumpSamples(const string& path, const vector<SVector2>& samples)
	{
		NSystem::CFile file;
		MF_ASSERT(file.Open(path, NSystem::CFile::EOpenMode::WriteText));

		for (int i = 0; i < samples.size(); i++)
		{
			string xs = NEssentials::ToString(samples[i].x);
			string ys = NEssentials::ToString(samples[i].y);

			if (xs.length() == 1)
				xs += ".0";
			if (ys.length() == 1)
				ys += ".0";

			string s = "\tfloat2(" + xs + "f, " + ys + "f),\n";
			file.WriteText(s);
		}

		file.Close();
	}
} }
