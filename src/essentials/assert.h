#pragma once


#include <cstdio>
#ifndef MAXEST_FRAMEWORK_WINDOWS
	#include <signal.h>
#endif

#define MF_ASSERT(condition)	NMaxestFramework::NEssentials::Assert(condition, __FILE__, __LINE__)

// this macro is just a dummy so that we can actually call the function
#define MF_ASSERT_INT_FUNCTION_EQUAL_PRIVATE(function, referenceValue, file, line) \
	{ \
		int returnValue = function; \
		NMaxestFramework::NEssentials::Assert(returnValue, referenceValue, file, line); \
	}
#define MF_ASSERT_INT_FUNCTION_EQUAL(function, referenceValue)	MF_ASSERT_INT_FUNCTION_EQUAL_PRIVATE(function, referenceValue, __FILE__, __LINE__)


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

	inline void Assert(int returnValue, int referenceValue, const char* fileName, int line)
	{
		if (returnValue != referenceValue)
		{
			FILE* file;
		#ifdef MAXEST_FRAMEWORK_WINDOWS
			fopen_s(&file, "assert.txt", "w");
		#else
			file = fopen("assert.txt", "w");
		#endif
			fprintf(file, "%s:%d\n", fileName, line);
			fprintf(file, "return value: %d\n", returnValue, line);
			fprintf(file, "reference value: %d\n", referenceValue, line);
			fclose(file);

		#ifdef MAXEST_FRAMEWORK_WINDOWS
			__debugbreak();
		#else
			raise(SIGTRAP);
		#endif
		}
	}
} }
