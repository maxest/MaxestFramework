// can't be function because of compiler bug regarding passing groupshared buffers as params
#define BITS_WRITE_TO_MEMORY(buffer, bitsOffset, value, valueBitsCount) \
	{\
		uint part1ByteIndex = (bitsOffset) >> 5;\
		uint part2ByteIndex = (bitsOffset + valueBitsCount) >> 5;\
\
		uint part1StartBitIndex = (bitsOffset) & 31;\
		uint part1BitsCount = (part1ByteIndex == part2ByteIndex ? (valueBitsCount) : 32 - part1StartBitIndex);\
		uint part2BitsCount = (part1ByteIndex == part2ByteIndex ? 0 : (valueBitsCount) - part1BitsCount);\
\
		uint part1 = (value);\
		part1 &= (part1BitsCount < 32 ? (1 << part1BitsCount) - 1 : 0xFFFFFFFF);\
		uint part2 = (value) >> part1BitsCount;\
		part2 &= (part2BitsCount < 32 ? (1 << part2BitsCount) - 1 : 0xFFFFFFFF);\
\
		InterlockedOr(buffer[part1ByteIndex], part1 << part1StartBitIndex);\
		InterlockedOr(buffer[part2ByteIndex], part2);\
	}
	
	
uint BitMask(uint bitsCount)
{
	return (1 << bitsCount) - 1;
}


uint BitCount(uint value)
{
	return firstbithigh(value) + 1;
}


void BitSetOn(inout uint value, uint bitIndex)
{
	value |= (1 << bitIndex);
}


void BitSetOff(inout uint value, uint bitIndex)
{
	value &= ~(1 << bitIndex);
}
