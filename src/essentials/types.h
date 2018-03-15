#pragma once


#include <cfloat>
#include <climits>


typedef wchar_t wchar;

typedef unsigned int uint;
typedef unsigned short ushort;
typedef unsigned long ulong;

typedef char int8;
typedef short int16;
typedef int int32;
typedef long long int64;

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef unsigned long long uint64;


const int cIntMin = (-2147483647 - 1);
const int cIntMax = 2147483647;
const uint cUIntMin = 0;
const uint cUIntMax = 0xffffffffu;
const float cFloatMin = -FLT_MAX;
const float cFloatMax = FLT_MAX;
const double cDoubleMin = -DBL_MAX;
const double cDoubleMax = DBL_MAX;

const int8 cInt8Min = (-127 - 1);
const int8 cInt8Max = 127;
const int16 cInt16Min = (-32767 - 1);
const int16 cInt16Max = 32767;
const int32 cInt32Min = (-2147483647 - 1);
const int32 cInt32Max = 2147483647;
const int64 cInt64Min = (-9223372036854775807 - 1);
const int64 cInt64Max = 9223372036854775807;

const uint8 cUInt8Min = 0;
const uint8 cUInt8Max = 0xffu;
const uint16 cUInt16Min = 0;
const uint16 cUInt16Max = 0xffffu;
const uint32 cUInt32Min = 0;
const uint32 cUInt32Max = 0xffffffffu;
const uint64 cUInt64Min = 0;
const uint64 cUInt64Max = 0xffffffffffffffffu;
