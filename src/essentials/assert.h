#pragma once


#ifndef MAXEST_FRAMEWORK_WINDOWS
	#include <signal.h>
#endif


#define MF_ASSERT(condition)	NMaxestFramework::NEssentials::Assert(condition)


namespace NMaxestFramework { namespace NEssentials
{
	inline void Assert(bool condition)
	{	
		if (!(condition))
		{
        #ifdef MAXEST_FRAMEWORK_WINDOWS
			__debugbreak();
        #else
			raise(SIGTRAP);
        #endif
		}
	}
} }
