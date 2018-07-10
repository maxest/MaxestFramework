#pragma once


#include "macros.h"
#include "stl.h"

#include <cstring>


namespace NMaxestFramework { namespace NEssentials
{
	struct SBytesArray
	{
		vector<uint8> data;

		void Clear()
		{
			data.clear();
		}

		void Add(uint8 data)
		{
			this->data.push_back(data);
		}

		void Add(const uint8* data, int dataSize)
		{
			int currentDataSize = (int)this->data.size();
			this->data.resize(currentDataSize + dataSize);
			memcpy(&this->data[currentDataSize], data, dataSize);
		}

		void Add(const SBytesArray& bytesArray)
		{
			if (bytesArray.DataSize() == 0)
				return;

			int currentDataSize = (int)this->data.size();
			this->data.resize(currentDataSize + bytesArray.DataSize());
			memcpy(&this->data[currentDataSize], bytesArray.Data(), bytesArray.DataSize());
		}

		const uint8* Data() const
		{
			return &data[0];
		}

		int DataSize() const
		{
			return (int)data.size();
		}
	};

	template<int BITS_COUNT>
	struct SStaticBitArray
	{
		uint8 bytes[(BITS_COUNT + 7) / 8];

		int Count() const { return BITS_COUNT; }
		void Clear() { memset(bytes, 0, sizeof(bytes)); }

		void Set(int index, bool state) { NBit::Set(bytes, index, state); }
		bool Get(int index) const { return NBit::Get(bytes, index); }
		void Set(int index, uint value, int valueBitsOffset, int valueBitsCount) { NBit::Set(bytes, index, value, valueBitsOffset, valueBitsCount); }
		uint Get(int index, int bitsCountToRead) const { return NBit::Get(bytes, index, bitsCountToRead); }
		void Flip(int index) { NBit::Flip(bytes, index); }
		void Flip(int index, int count) { NBit::Flip(bytes, index, count); }
		void Revert() { NBit::Revert(&bytes[0], 0, BITS_COUNT); }
		void Print(int index, int count) const { NBit::Print(bytes, index, count); }
		void Print() const { Print(0, BITS_COUNT); }
	};

	struct SDynamicBitArray
	{
		vector<uint8> bytes;
		int bitsCount;

		SDynamicBitArray()
		{
			bitsCount = 0;
		}

		void Clear()
		{
			bytes.clear();
			bitsCount = 0;
		}

		void Set(int index, bool state) { NBit::Set(&bytes[0], index, state); }
		bool Get(int index) const { return NBit::Get(&bytes[0], index); }
		void Set(int index, uint value, int valueBitsOffset, int valueBitsCount) { NBit::Set(&bytes[0], index, value, valueBitsOffset, valueBitsCount); }
		uint Get(int index, int bitsCountToRead) const { return NBit::Get(&bytes[0], index, bitsCountToRead); }
		void Flip(int index) { NBit::Flip(&bytes[0], index); }
		void Flip(int index, int count) { NBit::Flip(&bytes[0], index, count); }
		void Revert() { NBit::Revert(&bytes[0], 0, bitsCount); }
		void Print(int index, int count) const { NBit::Print(&bytes[0], index, count); }
		void Print() const { Print(0, bitsCount); }

		void Add(bool state)
		{
			uint bytesNeeded = (bitsCount+1 + 7) / 8;
			while (bytes.size() < bytesNeeded)
				bytes.push_back(0);

			NBit::Set(&bytes[0], bitsCount, state);

			bitsCount++;
		}

		void Add(uint value, int valueBitsOffset, int valueBitsCount)
		{
			uint bytesNeeded = (bitsCount+valueBitsCount + 7) / 8;
			while (bytes.size() < bytesNeeded)
				bytes.push_back(0);

			NBit::Set(&bytes[0], bitsCount, value, valueBitsOffset, valueBitsCount);

			bitsCount += valueBitsCount;
		}

		void Add(const SDynamicBitArray& dba)
		{
			for (int i = 0; i < dba.bitsCount; i++)
				Add(dba.Get(i));
		}

		bool operator == (const SDynamicBitArray& dba) const
		{
			if (dba.bitsCount != bitsCount)
				return false;
			else
				return memcmp(&dba.bytes[0], &bytes[0], bytes.size()) == 0;
		}

		bool operator < (const SDynamicBitArray& dba) const
		{
			if (dba.bitsCount > bitsCount)
			{
				return true;
			}
			else if (dba.bitsCount < bitsCount)
			{
				return false;
			}
			else
			{
				for (int i = 0; i < bitsCount; i++)
				{
					if (dba.Get(i) != Get(i))
					{
						return dba.Get(i) > Get(i);
					}
				}

				return false;
			}
		}
	};

	void ArrayUInt8Set(uint8* data, int bitIndex, uint8 value, int valueBitsCount);
	uint8 ArrayUInt8Get(uint8* data, int bitIndex, int bitsCount);
	void ArrayUInt16Set(uint16* data, int bitIndex, uint16 value, int valueBitsCount);
	uint16 ArrayUInt16Get(uint16* data, int bitIndex, int bitsCount);
	void ArrayUInt32Set(uint32* data, int bitIndex, uint32 value, int valueBitsCount);
	uint32 ArrayUInt32Get(uint32* data, int bitIndex, int bitsCount);
	void ArrayUInt64Set(uint64* data, int bitIndex, uint64 value, int valueBitsCount);
	uint64 ArrayUInt64Get(uint64* data, int bitIndex, int bitsCount);

	int ArrayFindSmallest(const float* elements, int elementsCount);
	int ArrayFindBiggest(const float* elements, int elementsCount);

	template<typename TYPE> void Array2DSubGet(const TYPE* data, int width, TYPE* subData, int subX, int subY, int subWidth, int subHeight);
	template<typename TYPE> void Array2DSubSet(const TYPE* subData, int subWidth, int subHeight, TYPE* data, int x, int y, int width);
	template<typename TYPE> void Array2DCopy(const TYPE* dstData, const TYPE* srcData, int width, int height);

	//

	FORCE_INLINE void ArrayUInt8Set(uint8* data, int bitIndex, uint8 value, int valueBitsCount)
	{
		uint part1ByteIndex = bitIndex >> 3;
		uint part2ByteIndex = (bitIndex + valueBitsCount) >> 3;

		uint part1StartBitIndex = bitIndex & 7;
		uint part1BitsCount = (part1ByteIndex == part2ByteIndex ? valueBitsCount : 8 - part1StartBitIndex);
		uint part2BitsCount = (part1ByteIndex == part2ByteIndex ? 0 : valueBitsCount - part1BitsCount);

		uint8 part1 = value;
		part1 &= (part1BitsCount < 8 ? (1U << part1BitsCount) - 1 : 0xFF);
		uint8 part2 = value >> part1BitsCount;
		part2 &= (part2BitsCount < 8 ? (1U << part2BitsCount) - 1 : 0xFF);

		data[part1ByteIndex] |= part1 << part1StartBitIndex;
		data[part2ByteIndex] |= part2;
	}

	FORCE_INLINE uint8 ArrayUInt8Get(uint8* data, int bitIndex, int bitsCount)
	{
		uint part1ByteIndex = bitIndex >> 3;
		uint part2ByteIndex = (bitIndex + bitsCount) >> 3;

		uint part1StartBitIndex = bitIndex & 7;
		uint part1BitsCount = (part1ByteIndex == part2ByteIndex ? bitsCount : 8 - part1StartBitIndex);
		uint part2BitsCount = (part1ByteIndex == part2ByteIndex ? 0 : bitsCount - part1BitsCount);

		uint8 part1 = data[part1ByteIndex] >> part1StartBitIndex;
		part1 &= (part1BitsCount < 8 ? (1U << part1BitsCount) - 1 : 0xFF);
		uint8 part2 = data[part2ByteIndex];
		part2 &= (part2BitsCount < 8 ? (1U << part2BitsCount) - 1 : 0xFF);

		return (part1) | (part2 << part1BitsCount);
	}

	FORCE_INLINE void ArrayUInt16Set(uint16* data, int bitIndex, uint16 value, int valueBitsCount)
	{
		uint part1ByteIndex = bitIndex >> 4;
		uint part2ByteIndex = (bitIndex + valueBitsCount) >> 4;

		uint part1StartBitIndex = bitIndex & 15;
		uint part1BitsCount = (part1ByteIndex == part2ByteIndex ? valueBitsCount : 16 - part1StartBitIndex);
		uint part2BitsCount = (part1ByteIndex == part2ByteIndex ? 0 : valueBitsCount - part1BitsCount);

		uint16 part1 = value;
		part1 &= (part1BitsCount < 16 ? (1U << part1BitsCount) - 1 : 0xFFFF);
		uint16 part2 = value >> part1BitsCount;
		part2 &= (part2BitsCount < 16 ? (1U << part2BitsCount) - 1 : 0xFFFF);

		data[part1ByteIndex] |= part1 << part1StartBitIndex;
		data[part2ByteIndex] |= part2;
	}

	FORCE_INLINE uint16 ArrayUInt16Get(uint16* data, int bitIndex, int bitsCount)
	{
		uint part1ByteIndex = bitIndex >> 4;
		uint part2ByteIndex = (bitIndex + bitsCount) >> 4;

		uint part1StartBitIndex = bitIndex & 15;
		uint part1BitsCount = (part1ByteIndex == part2ByteIndex ? bitsCount : 16 - part1StartBitIndex);
		uint part2BitsCount = (part1ByteIndex == part2ByteIndex ? 0 : bitsCount - part1BitsCount);

		uint16 part1 = data[part1ByteIndex] >> part1StartBitIndex;
		part1 &= (part1BitsCount < 16 ? (1U << part1BitsCount) - 1 : 0xFFFF);
		uint16 part2 = data[part2ByteIndex];
		part2 &= (part2BitsCount < 16 ? (1U << part2BitsCount) - 1 : 0xFFFF);

		return (part1) | (part2 << part1BitsCount);
	}

	FORCE_INLINE void ArrayUInt32Set(uint32* data, int bitIndex, uint32 value, int valueBitsCount)
	{
		uint part1ByteIndex = bitIndex >> 5;
		uint part2ByteIndex = (bitIndex + valueBitsCount) >> 5;

		uint part1StartBitIndex = bitIndex & 31;
		uint part1BitsCount = (part1ByteIndex == part2ByteIndex ? valueBitsCount : 32 - part1StartBitIndex);
		uint part2BitsCount = (part1ByteIndex == part2ByteIndex ? 0 : valueBitsCount - part1BitsCount);

		uint32 part1 = value;
		part1 &= (part1BitsCount < 32 ? (1U << part1BitsCount) - 1 : 0xFFFFFFFF);
		uint32 part2 = value >> part1BitsCount;
		part2 &= (part2BitsCount < 32 ? (1U << part2BitsCount) - 1 : 0xFFFFFFFF);

		data[part1ByteIndex] |= part1 << part1StartBitIndex;
		data[part2ByteIndex] |= part2;
	}

	FORCE_INLINE uint32 ArrayUInt32Get(uint32* data, int bitIndex, int bitsCount)
	{
		uint part1ByteIndex = bitIndex >> 5;
		uint part2ByteIndex = (bitIndex + bitsCount) >> 5;

		uint part1StartBitIndex = bitIndex & 31;
		uint part1BitsCount = (part1ByteIndex == part2ByteIndex ? bitsCount : 32 - part1StartBitIndex);
		uint part2BitsCount = (part1ByteIndex == part2ByteIndex ? 0 : bitsCount - part1BitsCount);

		uint32 part1 = data[part1ByteIndex] >> part1StartBitIndex;
		part1 &= (part1BitsCount < 32 ? (1U << part1BitsCount) - 1 : 0xFFFFFFFF);
		uint32 part2 = data[part2ByteIndex];
		part2 &= (part2BitsCount < 32 ? (1U << part2BitsCount) - 1 : 0xFFFFFFFF);

		return (part1) | (part2 << part1BitsCount);
	}

	FORCE_INLINE void ArrayUInt64Set(uint64* data, int bitIndex, uint64 value, int valueBitsCount)
	{
		uint64 part1ByteIndex = bitIndex >> 6;
		uint64 part2ByteIndex = (bitIndex + valueBitsCount) >> 6;

		uint64 part1StartBitIndex = bitIndex & 63;
		uint64 part1BitsCount = (part1ByteIndex == part2ByteIndex ? valueBitsCount : 64 - part1StartBitIndex);
		uint64 part2BitsCount = (part1ByteIndex == part2ByteIndex ? 0 : valueBitsCount - part1BitsCount);

		uint64 part1 = value;
		part1 &= (part1BitsCount < 64 ? ((uint64)1 << part1BitsCount) - 1 : 0xFFFFFFFFFFFFFFFF);
		uint64 part2 = value >> part1BitsCount;
		part2 &= (part2BitsCount < 64 ? ((uint64)1 << part2BitsCount) - 1 : 0xFFFFFFFFFFFFFFFF);

		data[part1ByteIndex] |= part1 << part1StartBitIndex;
		data[part2ByteIndex] |= part2;
	}

	FORCE_INLINE uint64 ArrayUInt64Get(uint64* data, int bitIndex, int bitsCount)
	{
		uint64 part1ByteIndex = bitIndex >> 6;
		uint64 part2ByteIndex = (bitIndex + bitsCount) >> 6;

		uint64 part1StartBitIndex = bitIndex & 63;
		uint64 part1BitsCount = (part1ByteIndex == part2ByteIndex ? bitsCount : 64 - part1StartBitIndex);
		uint64 part2BitsCount = (part1ByteIndex == part2ByteIndex ? 0 : bitsCount - part1BitsCount);

		uint64 part1 = data[part1ByteIndex] >> part1StartBitIndex;
		part1 &= (part1BitsCount < 64 ? ((uint64)1 << part1BitsCount) - 1 : 0xFFFFFFFFFFFFFFFF);
		uint64 part2 = data[part2ByteIndex];
		part2 &= (part2BitsCount < 64 ? ((uint64)1 << part2BitsCount) - 1 : 0xFFFFFFFFFFFFFFFF);

		return (part1) | (part2 << part1BitsCount);
	}

	inline int ArrayFindSmallest(const float* elements, int elementsCount)
	{
		int indexOfSmallest = 0;

		for (int i = 1; i < elementsCount; i++)
		{
			if (elements[i] < elements[indexOfSmallest])
				indexOfSmallest = i;
		}

		return indexOfSmallest;
	}

	inline int ArrayFindBiggest(const float* elements, int elementsCount)
	{
		int indexOfBiggest = 0;

		for (int i = 1; i < elementsCount; i++)
		{
			if (elements[i] > elements[indexOfBiggest])
				indexOfBiggest = i;
		}

		return indexOfBiggest;
	}

	template<typename TYPE> void Array2DSubGet(const TYPE* data, int width, TYPE* subData, int subX, int subY, int subWidth, int subHeight)
	{
		for (int y = 0; y < subHeight; y++)
		{
			const TYPE* srcData = &data[Idx(subX, subY + y, width)];
			TYPE* dstData = &subData[Idx(0, y, subWidth)];

			memcpy(dstData, srcData, subWidth * sizeof(TYPE));
		}
	}

	template<typename TYPE> void Array2DSubSet(const TYPE* subData, int subWidth, int subHeight, TYPE* data, int x, int y, int width)
	{
		for (int yy = 0; yy < subHeight; yy++)
		{
			const TYPE* srcData = &subData[Idx(0, yy, subWidth)];
			TYPE* dstData = &data[Idx(x, y + yy, width)];

			memcpy(dstData, srcData, subWidth * sizeof(TYPE));
		}
	}

	template<typename TYPE> void Array2DCopy(const TYPE* dstData, const TYPE* srcData, int width, int height)
	{
		memcpy((void*)dstData, (void*)srcData, width * height * sizeof(TYPE));
	}
} }
