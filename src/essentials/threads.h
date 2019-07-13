#pragma once


#include "../essentials/assert.h"
#include "../essentials/types.h"

#ifdef MAXEST_FRAMEWORK_WINDOWS
	#include <Windows.h>
#else
	#include <pthread.h>
	#include <semaphore.h>
#endif


namespace NMaxestFramework { namespace NEssentials
{
#ifdef MAXEST_FRAMEWORK_WINDOWS
	typedef HANDLE TThreadHandle;
	typedef HANDLE TMutexHandle;
	typedef HANDLE TSemaphoreHandle;

	typedef DWORD (__stdcall *TThreadFunction)(void* data);
	#define THREAD_FUNCTION_RETURN_VALUE DWORD _stdcall
#else
	typedef pthread_t* TThreadHandle;
	typedef pthread_mutex_t* TMutexHandle;
	typedef sem_t* TSemaphoreHandle;

	typedef void* (*TThreadFunction)(void* data);
	#define THREAD_FUNCTION_RETURN_VALUE void*
#endif

	enum class EPriority { Normal, RealTime };

	//

	void ProcessSetPriority(EPriority processPriority);

	TThreadHandle ThreadCreate(TThreadFunction threadFunction, void* data = nullptr);
	void ThreadDestroy(TThreadHandle threadHandle);
	void ThreadSetPriority(TThreadHandle threadHandle, EPriority threadPriority);
	bool ThreadJoin(TThreadHandle threadHandle); // wait

	TMutexHandle MutexCreate();
	void MutexDestroy(TMutexHandle mutexHandle);
	bool MutexLock(TMutexHandle mutexHandle);
	bool MutexTryLock(TMutexHandle mutexHandle);
	void MutexUnlock(TMutexHandle mutexHandle);

	TSemaphoreHandle SemaphoreCreate(int initialCount, int maxCount);
	void SemaphoreDestroy(TSemaphoreHandle semaphoreHandle);
	bool SemaphoreAcquire(TSemaphoreHandle semaphoreHandle);
	bool SemaphoreTryAcquire(TSemaphoreHandle semaphoreHandle);
	void SemaphoreRelease(TSemaphoreHandle semaphoreHandle, int count = 1);

	void AtomicSet32(int32* data, int x);
	int32 AtomicIncrement32(int32* data);
	int32 AtomicDecrement32(int32* data);
	int32 AtomicAdd32(int32* data, int x);
	int32 AtomicCompareExchange32(int32* data, int32 comparand, int32 exchange);

	//

	inline void ProcessSetPriority(EPriority processPriority)
	{
	#ifdef MAXEST_FRAMEWORK_WINDOWS
		DWORD processPriority_winapi = NORMAL_PRIORITY_CLASS;
		if (processPriority == EPriority::RealTime)
			processPriority_winapi = REALTIME_PRIORITY_CLASS;

		SetPriorityClass(GetCurrentProcess(), processPriority_winapi);
	#else
		UNUSED(processPriority);
	#endif
	}

	inline TThreadHandle ThreadCreate(TThreadFunction threadFunction, void* data)
	{
	#ifdef MAXEST_FRAMEWORK_WINDOWS
		return ::CreateThread(0, 0, threadFunction, data, 0, 0);
	#else
		TThreadHandle threadHandle = new pthread_t();
		pthread_create(threadHandle, nullptr, threadFunction, data);
		return threadHandle;
	#endif
	}

	inline void ThreadDestroy(TThreadHandle threadHandle)
	{
	#ifdef MAXEST_FRAMEWORK_WINDOWS
		TerminateThread(threadHandle, 0);
		CloseHandle(threadHandle);
	#elif MAXEST_FRAMEWORK_ANDROID
		delete threadHandle;
	#else
		pthread_cancel(*threadHandle);
		delete threadHandle;
	#endif
	}

	inline void ThreadSetPriority(TThreadHandle threadHandle, EPriority threadPriority)
	{
	#ifdef MAXEST_FRAMEWORK_WINDOWS
		int threadPriority_winapi = THREAD_PRIORITY_NORMAL;
		if (threadPriority == EPriority::RealTime)
			threadPriority_winapi = THREAD_PRIORITY_TIME_CRITICAL;

		SetThreadPriority(threadHandle == 0 ? GetCurrentThread() : threadHandle, threadPriority_winapi);
	#else
		UNUSED(threadHandle);
		UNUSED(threadPriority);
	#endif
	}

	inline bool ThreadJoin(TThreadHandle threadHandle)
	{
	#ifdef MAXEST_FRAMEWORK_WINDOWS
		int state = WaitForSingleObject(threadHandle, INFINITE);
		MF_ASSERT(state == WAIT_OBJECT_0 || state == WAIT_TIMEOUT);
		return state == WAIT_OBJECT_0;
	#else
		return pthread_join(*threadHandle, nullptr) == 0;
	#endif
	}

	//

	inline TMutexHandle MutexCreate()
	{
	#ifdef MAXEST_FRAMEWORK_WINDOWS
		return ::CreateMutex(nullptr, false, nullptr);
	#else
		TMutexHandle mutexHandle = new pthread_mutex_t();
		pthread_mutex_init(mutexHandle, nullptr);
		return mutexHandle;
	#endif
	}

	inline void MutexDestroy(TMutexHandle mutexHandle)
	{
	#ifdef MAXEST_FRAMEWORK_WINDOWS
		::CloseHandle(mutexHandle);
	#else
		pthread_mutex_destroy(mutexHandle);
		delete mutexHandle;
	#endif
	}

	inline bool MutexLock(TMutexHandle mutexHandle)
	{
	#ifdef MAXEST_FRAMEWORK_WINDOWS
		int state = WaitForSingleObject(mutexHandle, INFINITE);
		return state == WAIT_OBJECT_0;
	#else
		return pthread_mutex_lock(mutexHandle) == 0;
	#endif
	}

	inline bool MutexTryLock(TMutexHandle mutexHandle)
	{
	#ifdef MAXEST_FRAMEWORK_WINDOWS
		int state = WaitForSingleObject(mutexHandle, 0);
		MF_ASSERT(state == WAIT_OBJECT_0 || state == WAIT_TIMEOUT);
		return state == WAIT_OBJECT_0;
	#else
		return pthread_mutex_trylock(mutexHandle) == 0;
	#endif
	}

	inline void MutexUnlock(TMutexHandle mutexHandle)
	{
	#ifdef MAXEST_FRAMEWORK_WINDOWS
		ReleaseMutex(mutexHandle);
	#else
		pthread_mutex_unlock(mutexHandle);
	#endif
	}

	//

	inline TSemaphoreHandle SemaphoreCreate(int initialCount, int maxCount)
	{
	#ifdef MAXEST_FRAMEWORK_WINDOWS
		return ::CreateSemaphore(nullptr, initialCount, maxCount, nullptr);
	#else
		UNUSED(maxCount);
		TSemaphoreHandle semaphoreHandle = new sem_t();
		sem_init(semaphoreHandle, 0, initialCount);
		return semaphoreHandle;
	#endif
	}

	inline void SemaphoreDestroy(TSemaphoreHandle semaphoreHandle)
	{
	#ifdef MAXEST_FRAMEWORK_WINDOWS
		::CloseHandle(semaphoreHandle);
	#else
		sem_destroy(semaphoreHandle);
		delete semaphoreHandle;
	#endif
	}

	inline bool SemaphoreAcquire(TSemaphoreHandle semaphoreHandle)
	{
	#ifdef MAXEST_FRAMEWORK_WINDOWS
		int state = WaitForSingleObject(semaphoreHandle, INFINITE);
		return state == WAIT_OBJECT_0;
	#else
		return sem_wait(semaphoreHandle) == 0;
	#endif
	}

	inline bool SemaphoreTryAcquire(TSemaphoreHandle semaphoreHandle)
	{
	#ifdef MAXEST_FRAMEWORK_WINDOWS
		int state = WaitForSingleObject(semaphoreHandle, 0);
		MF_ASSERT(state == WAIT_OBJECT_0 || state == WAIT_TIMEOUT);
		return state == WAIT_OBJECT_0;
	#else
		return sem_trywait(semaphoreHandle) == 0;
	#endif
	}

	inline void SemaphoreRelease(TSemaphoreHandle semaphoreHandle, int count)
	{
	#ifdef MAXEST_FRAMEWORK_WINDOWS
		::ReleaseSemaphore(semaphoreHandle, count, nullptr);
	#else
		for (int i = 0; i < count; i++)
			sem_post(semaphoreHandle);
	#endif
	}

	//

	inline void AtomicSet32(int32* data, int x)
	{
	#ifdef MAXEST_FRAMEWORK_WINDOWS
		::InterlockedExchange((long*)data, x);
	#else
		UNUSED(data);
		UNUSED(x);
		MF_ASSERT(false);
	#endif
	}

	inline int32 AtomicIncrement32(int32* data)
	{
	#ifdef MAXEST_FRAMEWORK_WINDOWS
		int32 newValue = ::InterlockedIncrement((long*)data);
	#else
		int32 newValue = __sync_add_and_fetch(data, 1);
	#endif

		return newValue - 1;
	}

	inline int32 AtomicDecrement32(int32* data)
	{
	#ifdef MAXEST_FRAMEWORK_WINDOWS
		int32 newValue = ::InterlockedDecrement((long*)data);
	#else
		int32 newValue = __sync_sub_and_fetch(data, 1);
	#endif

		return newValue + 1;
	}

	inline int32 AtomicAdd32(int32* data, int x)
	{
	#ifdef MAXEST_FRAMEWORK_WINDOWS
		return ::InterlockedExchangeAdd((long*)data, x);
	#else
		return __sync_fetch_and_add(data, x);
	#endif
	}

	inline int32 AtomicCompareExchange32(int32* data, int32 comparand, int32 exchange)
	{
	#ifdef MAXEST_FRAMEWORK_WINDOWS
		return ::InterlockedCompareExchange((long*)data, exchange, comparand);
	#else
		return __sync_val_compare_and_swap(data, comparand, exchange);
	#endif
	}
} }
