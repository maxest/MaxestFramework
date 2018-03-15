#pragma once


#include "types.h"
#include "stl.h"


namespace NMaxestFramework { namespace NEssentials
{
	template<typename TYPE> TYPE Max(TYPE x, TYPE y);
	template<typename TYPE> TYPE Min(TYPE x, TYPE y);
	template<typename TYPE> TYPE Abs(TYPE x);

	uint32 Idx(uint32 x, uint32 y, uint32 width);
	uint32 Idx(uint32 x, uint32 y, uint32 z, uint32 width, uint32 height);

	int BitsToBytes(int bitsCount);
	int BytesToBits(int bytesCount);

	template<typename TYPE, int COUNT> vector<TYPE> ArrayToVector(TYPE array[COUNT]);
	template<typename TYPE, int COUNT> void VectorToArray(const vector<TYPE>& vec, TYPE array[COUNT]);

	//

	template<typename TYPE> TYPE Max(TYPE x, TYPE y)
	{
		return x > y ? x : y;
	}

	template<typename TYPE> TYPE Min(TYPE x, TYPE y)
	{
		return x < y ? x : y;
	}

	template<typename TYPE> TYPE Abs(TYPE x)
	{
		if (x < 0)
			return -x;
		else
			return x;
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
} }
