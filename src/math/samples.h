#pragma once


#include "types.h"
#include "vector.h"
#include "../essentials/main.h"


namespace NMaxestFramework { namespace NMath
{
	// disk
	vector<SVector2> VogelDiskSamples(uint n);
	vector<SVector2> AlchemySpiralDiskSamples(uint n, uint spiralsCount = 7);

	// rect
	vector<SVector2> RandomRectSamples2D(uint sqrtN);
	vector<SVector2> JitteredRectSamples2D(uint sqrtN);
	vector<SVector2> MultiJitteredRectSamples2D(uint sqrtN);
	vector<SSpherical> MapRectSamplesToHemisphere(const vector<SVector2>& samples, float e);

	// helper
	vector<string> DumpSamples(const vector<SVector2>& samples);

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

	inline vector<string> DumpSamples(const vector<SVector2>& samples)
	{
		vector<string> content;

		for (int i = 0; i < samples.size(); i++)
		{
			string xs = NEssentials::ToString(samples[i].x);
			string ys = NEssentials::ToString(samples[i].y);

			if (xs.length() == 1)
				xs += ".0";
			if (ys.length() == 1)
				ys += ".0";

			string s = "\tfloat2(" + xs + "f, " + ys + "f),";
			content.push_back(s);
		}

		return content;
	}
} }
