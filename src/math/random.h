#pragma once


#include "vector.h"
#include "../essentials/types.h"

#include <cstdlib>
#include <ctime>


namespace NMaxestFramework { namespace NMath
{
	void Randomize();
	void Randomize(int seed);

	uint32 RandomSystem();
	uint32 RandomSystem(uint16 from, uint16 to);
	uint16 Random16();
	uint16 Random16(uint16 from, uint16 to);
	uint32 Random32();
	uint32 Random32(uint32 from, uint32 to);

	float RandomFloat(); // [0..1]
	float RandomFloat(float from, float to);

	SVector2 RandomUnitVector2();
	SVector3 RandomUnitVector3();

	void ShuffleArray(uint32* elements, int elementsCount);
	uint32* RandomArray(uint32 count);

	float InterleavedGradientNoise(const SVector2& pt);
	float InterleavedGradientNoise4(const SVector2& pt);

	//

	inline void Randomize()
	{
		srand((uint)time(nullptr));
	}

	inline void Randomize(int seed)
	{
		srand(seed);
	}

	inline uint32 RandomSystem()
	{
		return rand();
	}

	inline uint32 RandomSystem(uint16 from, uint16 to)
	{
		return from + (RandomSystem() % (to - from + 1));
	}

	inline uint16 Random16()
	{
		uint16 r1 = rand() % 256; // 2^8
		uint16 r2 = rand() % 256; // 2^8

		return (r1) | (r2 << 8);
	}

	inline uint16 Random16(uint16 from, uint16 to)
	{
		return from + (Random16() % (to - from + 1));
	}

	inline uint32 Random32()
	{
		uint32 r1 = rand() % 2048; // 2^11
		uint32 r2 = rand() % 2048; // 2^11
		uint32 r3 = rand() % 1024; // 2^10

		return (r1) | (r2 << 11) | (r3 << 22);
	}

	inline uint32 Random32(uint32 from, uint32 to)
	{
		return from + (Random32() % (to - from + 1));
	}

	inline float RandomFloat()
	{
		return (float)rand() / (float)RAND_MAX;
	}

	inline float RandomFloat(float from, float to)
	{
		return from + (RandomFloat() * (to - from));
	}

	inline SVector2 RandomUnitVector2()
	{
		SVector2 v;

		v.x = RandomFloat() - 0.5f;
		v.y = RandomFloat() - 0.5f;
		NormalizeIn(v);

		return v;
	}

	inline SVector3 RandomUnitVector3()
	{
		SVector3 v;

		v.x = RandomFloat() - 0.5f;
		v.y = RandomFloat() - 0.5f;
		v.z = RandomFloat() - 0.5f;
		NormalizeIn(v);

		return v;
	}

	inline void ShuffleArray(uint32* elements, int elementsCount)
	{
		for (int i = 0; i < elementsCount; i++)
			Swap(elements[i], elements[Random32(0, elementsCount - 1)]);
	}

	inline uint32* RandomArray(uint32 count)
	{
		uint32* arr = new uint32[count];
		for (uint32 i = 0; i < count; i++)
			arr[i] = i;

		ShuffleArray(arr, count);

		return arr;
	}

	inline float InterleavedGradientNoise(const SVector2& pt)
	{
		SVector3 magic = VectorCustom(0.06711056f, 0.00583715f, 52.9829189f);
		return Frac(magic.z * Frac(Dot(pt, VectorCustom(magic.x, magic.y))));
	}

	inline float InterleavedGradientNoise4(const SVector2& pt)
	{
		SVector3 magic = VectorCustom(0.06711056f, 4.0f * 0.00583715f, 52.9829189f);
		return Frac(magic.z * Frac(Dot(pt, VectorCustom(magic.x, magic.y))));
	}
} }
