#pragma once


#include <cstdio>
#ifndef MAXEST_FRAMEWORK_WINDOWS
	#include <signal.h>
#endif

#define MF_ASSERT(condition)					NMaxestFramework::NEssentials::Assert(condition, __FILE__, __LINE__)
#define MF_ASSERT_INTS_EQUAL(value1, value2)	NMaxestFramework::NEssentials::AssertIntsEqual(value1, value2, __FILE__, __LINE__)


namespace NMaxestFramework { namespace NEssentials
{
	inline void Assert(bool condition, const char* fileName, int line)
	{
		if (!(condition))
		{
			FILE* file;
		#ifdef MAXEST_FRAMEWORK_WINDOWS
			fopen_s(&file, "assert.txt", "w");
		#else
			file = fopen("assert.txt", "w");
		#endif
			fprintf(file, "%s:%d\n", fileName, line);
			fclose(file);

		#ifdef MAXEST_FRAMEWORK_WINDOWS
			__debugbreak();
		#else
			raise(SIGTRAP);
		#endif
		}
	}

	inline void AssertIntsEqual(int value1, int value2, const char* fileName, int line)
	{
		if (!(value1 == value2))
		{
			FILE* file;
		#ifdef MAXEST_FRAMEWORK_WINDOWS
			fopen_s(&file, "assert.txt", "w");
		#else
			file = fopen("assert.txt", "w");
		#endif
			fprintf(file, "%s:%d\n", fileName, line);
			fprintf(file, "value1: %d\n", value1, line);
			fprintf(file, "value2: %d\n", value2, line);
			fclose(file);

		#ifdef MAXEST_FRAMEWORK_WINDOWS
			__debugbreak();
		#else
			raise(SIGTRAP);
		#endif
		}
	}
} }
