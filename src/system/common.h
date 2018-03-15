#pragma once


#include "../essentials/types.h"

#include "../../dependencies/SDL2-2.0.5/include/SDL.h"
#ifdef MAXEST_FRAMEWORK_WINDOWS
	#include <Windows.h>
#else
	#include <sys/time.h>
#endif


namespace NMaxestFramework { namespace NSystem
{
	int DisplaysCount();
	void ScreenSize(int& width, int& height);
	
	void Sleep(int ms);

	uint64 TickCount(); // high-precision
	uint32 TickCountLow(); // low-precision

	//

	inline int DisplaysCount()
	{
		return SDL_GetNumVideoDisplays();
	}

	inline void ScreenSize(int& width, int& height)
	{
	#ifdef MAXEST_FRAMEWORK_WINDOWS
		width = GetSystemMetrics(SM_CXSCREEN);
		height = GetSystemMetrics(SM_CYSCREEN);
	#else
		MF_ASSERT(false);
	#endif
	}

	inline void Sleep(int ms)
	{
	#ifdef MAXEST_FRAMEWORK_WINDOWS
		::Sleep(ms);
	#else
		usleep(1000 * ms);
	#endif
	}

	inline uint64 TickCount()
	{
	#ifdef MAXEST_FRAMEWORK_WINDOWS
		LARGE_INTEGER frequency, counter;

		QueryPerformanceFrequency(&frequency);
		QueryPerformanceCounter(&counter);

		double time = (double)counter.QuadPart / ((double)frequency.QuadPart / 1000000.0);

		return (uint64)time;
	#else
		struct timeval tv;
		struct timezone tz;
		uint64_t sec;
		uint64_t usec;

		gettimeofday(&tv, &tz);

		sec = (uint64_t)tv.tv_sec;
		usec = (uint64_t)tv.tv_usec;

		return sec*1000000l + usec;
	#endif
	}

	inline uint32 TickCountLow()
	{
		return SDL_GetTicks();
	}
} }
