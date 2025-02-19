#pragma once


#include "constants.h"
#include "../essentials/types.h"
#include "../essentials/common.h"

#include <cmath>


namespace NMaxestFramework { namespace NMath
{
	float Pow(float x, float y);
	float Sqrt(float x);
	double Sqrt(double x);
	float Exp(float x);
	double Exp(double x);
	float Sin(float x);
	float Cos(float x);
	float Tan(float x);
	float ASin(float x);
	float ACos(float x);
	float ACos_Clamped(float x);
	float ATan2(float y, float x);
	int Wrap(int x, int min, int max);
	int Pad(int x, int valueToPadTo);
	template<typename TYPE> TYPE Lerp(TYPE a, TYPE b, float t);
	template<typename TYPE> TYPE Sqr(TYPE x);
	template<typename TYPE> bool IsPowerOfTwo(TYPE x);
	template<typename TYPE> void Swap(TYPE& x, TYPE& y);
	template<typename TYPE> TYPE FlipFlop(TYPE value, TYPE flip, TYPE flop);
	template<typename TYPE> TYPE FlipFlapFlop(TYPE value, TYPE flip, TYPE flap, TYPE flop);
	float Floor(float x);
	float Ceil(float x);
	float Round(float x);
	float Frac(float x);
	float Log(float x);
	float Log2(float x);
	double Log2(double x);
	int Balance(int value, int index, int count);

	//

	inline float Pow(float x, float y)
	{
		return powf(x, y);
	}

	inline float Sqrt(float x)
	{
		return sqrtf(x);
	}

	inline double Sqrt(double x)
	{
		return sqrt(x);
	}

	inline float Exp(float x)
	{
		return expf(x);
	}

	inline double Exp(double x)
	{
		return exp(x);
	}

	inline float Sin(float x)
	{
		return sinf(x);
	}

	inline float Cos(float x)
	{
		return cosf(x);
	}

	inline float Tan(float x)
	{
		return tanf(x);
	}

	inline float ASin(float x)
	{
		return asinf(x);
	}

	inline float ACos(float x)
	{
		return acosf(x);
	}

	inline float ACos_Clamped(float x)
	{
		if (x <= -1.0f)
			return cPi;
		else if (x >= 1.0f)
			return 0.0f;
		else
			return acosf(x);
	}

	inline float ATan2(float y, float x)
	{
		return atan2f(y, x);
	}

	inline int Wrap(int x, int min, int max)
	{
		int rangeSize = max - min + 1;

		while (x < min)
			x += rangeSize;
		while (x > max)
			x -= rangeSize;

		return x;
	}

	inline int Pad(int x, int valueToPadTo)
	{
		return valueToPadTo * ( (x + (valueToPadTo - 1)) / valueToPadTo );
	}

	template<typename TYPE> TYPE Lerp(TYPE a, TYPE b, float t)
	{
		return (1.0f-t)*a + t*b;
	}

	template<typename TYPE> TYPE Sqr(TYPE x)
	{
		return x * x;
	}

	template<typename TYPE> bool IsPowerOfTwo(TYPE x)
	{
		return !(x & (x-1));
	}

	template<typename TYPE> void Swap(TYPE& x, TYPE& y)
	{
		TYPE z = x;
		x = y;
		y = z;
	}

	template<typename TYPE> TYPE FlipFlop(TYPE value, TYPE flip, TYPE flop)
	{
		if (value == flip)
			return flop;
		else
			return flip;
	}

	template<typename TYPE> TYPE FlipFlapFlop(TYPE value, TYPE flip, TYPE flap, TYPE flop)
	{
		if (value == flip)
			return flap;
		else if (value == flap)
			return flop;
		else
			return flip;
	}

	inline float Floor(float x)
	{
		return floorf(x);
	}

	inline float Ceil(float x)
	{
		return ceilf(x);
	}

	inline float Round(float x)
	{
		if (x > 0.0f)
			return Floor(x + 0.5f);
		else
			return -Floor(-x + 0.5f);
	}

	inline float Frac(float x)
	{
		return x - Floor(x);
	}

	inline float Log(float x)
	{
		return logf(x);
	}

	inline float Log2(float x)
	{
		return logf(x) / logf(2.0f);
	}

	inline double Log2(double x)
	{
		return log(x) / log(2.0);
	}

	inline int Balance(int value, int index, int count)
	{
		float f = (float)value / (float)count;

		int count2 = (int)(Frac(f) * count);
		int count1 = count - count2;

		if (index < count1)
			return (int)f;
		else
			return (int)f + 1;
	}
} }
