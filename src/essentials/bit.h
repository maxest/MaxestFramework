#pragma once


#include "types.h"
#include "common.h"


namespace NMaxestFramework { namespace NEssentials
{
	namespace NBit
	{
		uint32 Mask32(uint8 bitsCount);
		uint64 Mask64(uint8 bitsCount);
		uint8 Count(uint64 value);
		uint64 Clamp(uint64 value, uint8 bitsCount);

		void Set(uint8& data, int index, bool state);
		void Set(uint8* bytes, int index, bool state);
		void Set(uint16* bytes, int index, bool state);
		void Set(uint32* bytes, int index, bool state);
		void Set(uint64* bytes, int index, bool state);
		template<typename TYPE> void Set(const TYPE* bytes, int index, uint32 value, int valueBitsOffset, int valueBitsCount);

		bool Get(uint8 data, int index);
		bool Get(const uint8* bytes, int index);
		bool Get(const uint16* bytes, int index);
		bool Get(const uint32* bytes, int index);
		bool Get(const uint64* bytes, int index);
		template<typename TYPE> uint32 Get(const TYPE bytes, int index, int bitsCountToRead);

		void Flip(uint8* bytes, int index);
		void Flip(uint8* bytes, int index, int count);
		void Revert(uint8* bytes, int index, int count);

		void Print(const uint8* bytes, int index, int count);

		//

		inline uint32 Mask32(uint8 bitsCount)
		{
			return ((uint32)1 << bitsCount) - 1;
		}

		inline uint64 Mask64(uint8 bitsCount)
		{
			return ((uint64)1 << bitsCount) - 1;
		}

		inline uint8 Count(uint64 value)
		{
			if (value == 0)
				return 1;

			uint8 bitsCount = 0;
			while (value > 0)
			{
				bitsCount++;
				value >>= 1;
			}

			return bitsCount;
		}

		inline uint64 Clamp(uint64 value, uint8 bitsCount)
		{
			return NEssentials::Min(value, Mask64(bitsCount));
		}

		inline void Set(uint8& data, int index, bool state)
		{
			if (state)
				data |= (1 << index);
			else
				data &= ~(1 << index);
		}

		inline void Set(uint8* bytes, int index, bool state)
		{
			int byteIndex = index / 8;
			int bitIndex = index % 8;

			if (state)
				bytes[byteIndex] |= (1 << bitIndex);
			else
				bytes[byteIndex] &= ~(1 << bitIndex);
		}

		inline void Set(uint16* bytes, int index, bool state)
		{
			int byteIndex = index / 16;
			int bitIndex = index % 16;

			if (state)
				bytes[byteIndex] |= (1 << bitIndex);
			else
				bytes[byteIndex] &= ~(1 << bitIndex);
		}

		inline void Set(uint32* bytes, int index, bool state)
		{
			int byteIndex = index / 32;
			int bitIndex = index % 32;

			if (state)
				bytes[byteIndex] |= (1 << bitIndex);
			else
				bytes[byteIndex] &= ~(1 << bitIndex);
		}

		inline void Set(uint64* bytes, int index, bool state)
		{
			int byteIndex = index / 64;
			int bitIndex = index % 64;

			if (state)
				bytes[byteIndex] |= ((uint64)1 << bitIndex);
			else
				bytes[byteIndex] &= ~((uint64)1 << bitIndex);
		}

		template<typename TYPE> void Set(const TYPE bytes, int index, uint32 value, int valueBitsOffset, int valueBitsCount)
		{
			for (int i = 0; i < valueBitsCount; i++)
			{
				bool b = ( value >> (valueBitsOffset + i) ) & 1;
				Set(bytes, index + i, b);
			}
		}

		inline bool Get(uint8 data, int index)
		{
			return (data >> index) & 1;
		}

		inline bool Get(const uint8* bytes, int index)
		{
			int byteIndex = index / 8;
			int bitIndex = index % 8;

			return (bytes[byteIndex] >> bitIndex) & 1;
		}

		inline bool Get(const uint16* bytes, int index)
		{
			int byteIndex = index / 16;
			int bitIndex = index % 16;

			return (bytes[byteIndex] >> bitIndex) & 1;
		}

		inline bool Get(const uint32* bytes, int index)
		{
			int byteIndex = index / 32;
			int bitIndex = index % 32;

			return (bytes[byteIndex] >> bitIndex) & 1;
		}

		inline bool Get(const uint64* bytes, int index)
		{
			int byteIndex = index / 64;
			int bitIndex = index % 64;

			return (bytes[byteIndex] >> bitIndex) & 1;
		}

		template<typename TYPE> uint32 Get(const TYPE bytes, int index, int bitsCountToRead)
		{
			uint value = 0;

			for (int i = 0; i < bitsCountToRead; i++)
			{
				if (Get(bytes, index + i))
					value |= (1 << i);
			}

			return value;
		}

		inline void Flip(uint8* bytes, int index)
		{
			int byteIndex = index / 8;
			int bitIndex = index % 8;

			bytes[byteIndex] ^= (1 << bitIndex);
		}

		inline void Flip(uint8* bytes, int index, int count)
		{
			for (int i = 0; i < count; i++)
				Flip(bytes, index + i);
		}

		inline void Revert(uint8* bytes, int index, int count)
		{
			uint8* temp = new uint8[(count + 7) / 8];

			for (int i = 0; i < count; i++)
			{
				bool b = Get(bytes, index + i);
				Set(temp, i, b);
			}

			for (int i = 0; i < count; i++)
			{
				bool b = Get(temp, count - 1 - i);
				Set(bytes, index + i, b);
			}

			delete[] temp;
		}

		inline void Print(const uint8* bytes, int index, int count)
		{
			for (int i = 0; i < count; i++)
				cout << Get(bytes, index + i);
		}
	}
} }
