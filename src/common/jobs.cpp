#include "jobs.h"


using namespace NMaxestFramework;
using namespace NSystem;


//


void NCommon::CJobGroup::AddJob(CJob* job)
{
	job->owner = this;
	jobsCount++;
	jobs.push_back(job);
}


void NCommon::CJobGroup::Wait()
{
	SemaphoreAcquire(waitingSemaphore);
}


void NCommon::CJobGroup::OnJobDone()
{
	AtomicIncrement32(&jobsDoneCount);

	if (jobsDoneCount == jobsCount)
		SemaphoreRelease(waitingSemaphore, 1);
}


//


THREAD_FUNCTION_RETURN_VALUE JobThread(void* data)
{
	NCommon::CJobSystem* jobSystem = (NCommon::CJobSystem*)data;

	for (;;)
	{
		SemaphoreAcquire(jobSystem->activeJobsSemaphore);

		MutexLock(jobSystem->jobsQueueMutex);
		NCommon::CJob* job = jobSystem->jobs.front();
		jobSystem->jobs.pop();
		MutexUnlock(jobSystem->jobsQueueMutex);

		if (job->Do())
		{
			job->done = true;

			if (job->owner)
				job->owner->OnJobDone();
		}
	}

	return 0;
}


void NCommon::CJobSystem::Create(int threadsCount)
{
	activeJobsSemaphore = SemaphoreCreate(0, cIntMax);
	jobsQueueMutex = MutexCreate();

	for (int i = 0; i < threadsCount; i++)
	{
		TThreadHandle thread = ThreadCreate(JobThread, this);
		threads.push_back(thread);
	}
}


void NCommon::CJobSystem::Destroy()
{
	for (uint i = 0; i < threads.size(); i++)
		ThreadDestroy(threads[i]);
	threads.clear();

	MutexDestroy(jobsQueueMutex);
	SemaphoreDestroy(activeJobsSemaphore);
}


void NCommon::CJobSystem::AddJob(CJob* job)
{
	MutexLock(jobsQueueMutex);
	jobs.push(job);
	MutexUnlock(jobsQueueMutex);

	SemaphoreRelease(activeJobsSemaphore, 1);
}


void NCommon::CJobSystem::AddJobGroup(const CJobGroup& jobGroup)
{
	MutexLock(jobsQueueMutex);
	for (uint i = 0; i < jobGroup.jobs.size(); i++)
		jobs.push(jobGroup.jobs[i]);
	MutexUnlock(jobsQueueMutex);

	SemaphoreRelease(activeJobsSemaphore, (int)jobGroup.jobs.size());
}
