#pragma once


#include "../essentials/common.h"


namespace NMaxestFramework { namespace NMath
{
	struct SStats
	{
		uint _samplesCount;
		double _sum;
		double _min;
		double _max;
		double _mean;

		SStats()
		{
			Reset();
		}

		void Reset()
		{
			_samplesCount = 0;
			_sum = 0.0;
			_min = cFloatMax;
			_max = cFloatMin;
			_mean = 0.0;
		}

		void Add(double x)
		{
			_samplesCount++;
			_sum += x;
			_min = NEssentials::Min(_min, x);
			_max = NEssentials::Max(_max, x);
			_mean += (x - _mean) / _samplesCount;
		}
	};
} }
