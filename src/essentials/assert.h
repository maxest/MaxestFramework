#pragma once


#include <cstdio>
#ifndef MAXEST_FRAMEWORK_WINDOWS
	#include <signal.h>
#endif

#define MF_ASSERT(condition)	NMaxestFramework::NEssentials::Assert(condition, __FILE__, __LINE__)


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
			fprintf(file, "%s  %d\n", fileName, line);
			fclose(file);

		#ifdef MAXEST_FRAMEWORK_WINDOWS
			debugbreak();
		#else
			raise(SIGTRAP);
		#endif
		}
	}
} }
