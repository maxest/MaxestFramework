#pragma once


#include "bit.h"
#include "assert.h"

#include <cstring>


namespace NMaxestFramework { namespace NEssentials
{
	uint8 Encode_R8SInt_In_R8UInt(int8 r);
	int8 Decode_R8SInt_From_R8UInt(uint8 value_encoded);

	uint16 Encode_R8G8SInt_In_R16UInt(int8 r, int8 g);
	void Decode_R8G8SInt_From_R16UInt(uint16 value_encoded, int8& r, int8& g);

	uint32 Encode_R8G8G8A8UInt_In_R32UInt(uint8 r, uint8 g, uint8 b, uint8 a);
	void Decode_R8G8SInt_From_R16UInt(uint32 value_encoded, uint8& r, uint8& g, uint8& b, uint8& a);

	uint32 Encode_R32SInt_In_R32UInt(int32 value, uint8 bitsCount);
	int32 Decode_R32SInt_From_R32UInt(uint32 value_encoded, uint8 bitsCount);

	// float: exp - 8 bits, mantissa - 23 bits
	// half: exp - 5 bits, mantissa - 10 bits
	uint16 FloatToHalf(float flt);
	float HalfToFloat(uint16 half);

	//

	inline uint8 Encode_R8SInt_In_R8UInt(int8 r)
	{
		return r + 128;
	}

	inline int8 Decode_R8SInt_From_R8UInt(uint8 value_encoded)
	{
		return value_encoded - 128;
	}

	inline uint16 Encode_R8G8SInt_In_R16UInt(int8 r, int8 g)
	{
		uint8 r2 = r + 128;
		uint8 g2 = g + 128;

		return (r2) | ((uint16)g2 << 8);
	}

	inline void Decode_R8G8SInt_From_R16UInt(uint16 value_encoded, int8& r, int8& g)
	{
		r = (value_encoded & 255) - 128;
		g = ((value_encoded >> 8) & 255) - 128;
	}

	inline uint32 Encode_R8G8G8A8UInt_In_R32UInt(uint8 r, uint8 g, uint8 b, uint8 a)
	{
		uint32 value_encoded = r & 255;
		value_encoded |= (g & 255) << 8;
		value_encoded |= (b & 255) << 16;
		value_encoded |= (a & 255) << 24;

		return value_encoded;
	}

	inline void Decode_R8G8SInt_From_R16UInt(uint32 value_encoded, uint8& r, uint8& g, uint8& b, uint8& a)
	{
		r = (value_encoded >> 0) & 255;
		g = (value_encoded >> 8) & 255;
		b = (value_encoded >> 16) & 255;
		a = (value_encoded >> 24) & 255;
	}

	inline uint32 Encode_R32SInt_In_R32UInt(int32 value, uint8 bitsCount)
	{
		uint32 mask = NBit::Mask32(bitsCount - 1);
		uint32 value_abs = Abs(value);

		MF_ASSERT(value_abs <= mask);

		uint32 value_encoded = value_abs & mask;
		if (value > 0)
			NBit::Set((uint8*)&value_encoded, bitsCount - 1, true);

		return value_encoded;
	}

	inline int32 Decode_R32SInt_From_R32UInt(uint32 value_encoded, uint8 bitsCount)
	{
		uint32 mask = NBit::Mask32(bitsCount - 1);
		int32 value = value_encoded & mask;

		if (NBit::Get((uint8*)&value_encoded, bitsCount - 1) == 0)
			value = -value;

		return value;
	}

	inline uint16 FloatToHalf(float flt)
	{
		uint32 flt_data;
		memcpy(&flt_data, &flt, sizeof(flt));

		uint32 sign = (flt_data >> 31) & 1;
		uint32 exp = (flt_data >> 23) & 255;
		uint32 mantissa = flt_data & 0x7FFFFF;

		int newExp_signed = 16 + (exp - 127);
		newExp_signed = Clamp(newExp_signed, 0, 31);
		uint32 newExp = (uint32)newExp_signed;
		uint32 newMantissa = mantissa >> 13;

		uint16 half = (sign << 15);
		half |= (newExp << 10);
		half |= newMantissa;

		return half;
	}

	inline float HalfToFloat(uint16 half)
	{
		uint32 sign = (half >> 15) & 1;
		uint32 exp = (half >> 10) & 31;
		uint32 mantissa = half & 1023;

		int newExp_signed = (exp + 127) - 16;
		newExp_signed = Clamp(newExp_signed, 0, 255);
		uint32 newExp = (uint32)newExp_signed;
		uint32 newMantissa = mantissa << 13;

		uint32 flt_data = (sign << 31);
		flt_data |= (newExp << 23);
		flt_data |= newMantissa;

		float flt;
		memcpy(&flt, &flt_data, sizeof(flt));
		return flt;
	}
} }
