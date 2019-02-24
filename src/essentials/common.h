#pragma once


#include "types.h"
#include "stl.h"

#include <ctime>


namespace NMaxestFramework { namespace NEssentials
{
	template<typename TYPE> TYPE Max(TYPE x, TYPE y);
	template<typename TYPE> TYPE Min(TYPE x, TYPE y);
	template<typename TYPE> TYPE Clamp(TYPE x, TYPE min, TYPE max);
	template<typename TYPE> TYPE Abs(TYPE x);

	int Clamp256(int x);
	float Saturate(float x);
	double Saturate(double x);

	uint32 Idx(uint32 x, uint32 y, uint32 width);
	uint32 Idx(uint32 x, uint32 y, uint32 z, uint32 width, uint32 height);

	int BitsToBytes(int bitsCount);
	int BytesToBits(int bytesCount);

	template<typename TYPE, int COUNT> vector<TYPE> ArrayToVector(TYPE array[COUNT]);
	template<typename TYPE, int COUNT> void VectorToArray(const vector<TYPE>& vec, TYPE array[COUNT]);

	void CurrentTime(int* year, int* month, int* day, int* hour, int* minute, int* second);
	string CurrentTime();

	//

	template<typename TYPE> TYPE Max(TYPE x, TYPE y)
	{
		return x > y ? x : y;
	}

	template<typename TYPE> TYPE Min(TYPE x, TYPE y)
	{
		return x < y ? x : y;
	}

	template<typename TYPE> TYPE Clamp(TYPE x, TYPE min, TYPE max)
	{
		return Max(Min(x, max), min);
	}

	template<typename TYPE> TYPE Abs(TYPE x)
	{
		if (x < 0)
			return -x;
		else
			return x;
	}

	inline int Clamp256(int x)
	{
		return Clamp(x, 0, 255);
	}

	inline float Saturate(float x)
	{
		return Clamp(x, 0.0f, 1.0f);
	}

	inline double Saturate(double x)
	{
		return Clamp(x, 0.0, 1.0);
	}

	inline uint32 Idx(uint32 x, uint32 y, uint32 width)
	{
		return y*width + x;
	}

	inline uint32 Idx(uint32 x, uint32 y, uint32 z, uint32 width, uint32 height)
	{
		return z*width*height + y*height + x;
	}

	inline int BitsToBytes(int bitsCount)
	{
		return (bitsCount + 7) / 8;
	}

	inline int BytesToBits(int bytesCount)
	{
		return 8 * bytesCount;
	}

	template<typename TYPE, int COUNT> vector<TYPE> ArrayToVector(TYPE array[COUNT])
	{
		vector<TYPE> vec;

		for (int i = 0; i < COUNT; i++)
			vec.push_back(array[i]);

		return vec;
	}

	template<typename TYPE, int COUNT> void VectorToArray(const vector<TYPE>& vec, TYPE array[COUNT])
	{
		for (int i = 0; i < COUNT; i++)
			array[i] = vec[i];
	}

	inline void CurrentTime(int* year, int* month, int* day, int* hour, int* minute, int* second)
	{
		time_t rawTime = time(0);

	#ifdef MAXEST_FRAMEWORK_WINDOWS
		tm time;
		localtime_s(&time, &rawTime);

		if (year)
			*year = time.tm_year;
		if (month)
			*month = time.tm_mon;
		if (day)
			*day = time.tm_mday;
		if (hour)
			*hour = time.tm_hour;
		if (minute)
			*minute = time.tm_min;
		if (second)
			*second = time.tm_sec;
	#else
		tm* time = localtime(&rawTime);

		if (year)
			*year = time->tm_year;
		if (month)
			*month = time->tm_mon;
		if (day)
			*day = time->tm_mday;
		if (hour)
			*hour = time->tm_hour;
		if (minute)
			*minute = time->tm_min;
		if (second)
			*second = time->tm_sec;
	#endif
	}

	inline string CurrentTime()
	{
		time_t rawTime = time(0);

	#ifdef MAXEST_FRAMEWORK_WINDOWS
		tm time;
		localtime_s(&time, &rawTime);

		char timeStr[512];
		asctime_s(timeStr, &time);

		return string(timeStr);
	#else
		tm* time = localtime(&rawTime);
		return string(asctime(time));
	#endif
	}
} }
