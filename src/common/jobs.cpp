#include "jobs.h"


using namespace NMaxestFramework;
using namespace NSystem;


//


int NCommon::CJob::DoneStatus()
{
	return doneStatus;
}


bool NCommon::CJob::IsDone()
{
	return doneStatus >= 0;
}


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
	AtomicIncrement32(&jobsDoneCount); // this has to be atomic because all job threads call OnJobDone

	if (jobsDoneCount == jobsCount)
		SemaphoreRelease(waitingSemaphore, 1);
}


//


THREAD_FUNCTION_RETURN_VALUE NCommon::CJobSystem::JobThread(void* data)
{
	NCommon::CJobSystem::SThread* thread = (NCommon::CJobSystem::SThread*)data;
	NCommon::CJobSystem* jobSystem = thread->jobSystem;

	for (;;)
	{
		SemaphoreAcquire(jobSystem->activeJobsSemaphore);

		if (thread->requestStop)
			return 0;

		MutexLock(jobSystem->jobsQueueMutex);
		NCommon::CJob* job = jobSystem->jobs.front();
		jobSystem->jobs.pop();
		MutexUnlock(jobSystem->jobsQueueMutex);

		job->doneStatus = job->Do();
		if (job->owner)
			job->owner->OnJobDone();
	}

	return 0;
}


void NCommon::CJobSystem::Create(int threadsCount)
{
	activeJobsSemaphore = SemaphoreCreate(0, cIntMax);
	jobsQueueMutex = MutexCreate();

	for (int i = 0; i < threadsCount; i++)
	{
		SThread* thread = new SThread();
		thread->jobSystem = this;
		thread->handle = ThreadCreate(JobThread, thread);
		threads.push_back(thread);
	}
}


void NCommon::CJobSystem::Destroy()
{
	for (uint i = 0; i < threads.size(); i++)
	{
		threads[i]->requestStop = true;
	}
	SemaphoreRelease(activeJobsSemaphore, (int)threads.size());
	for (uint i = 0; i < threads.size(); i++)
	{
		// SemaphoreRelease(activeJobsSemaphore, 1); // can't do that because we don't know which thread will first acquire the semaphore. Hence we release the semaphore before the loop with a value equal to the number of threads
		ThreadJoin(threads[i]->handle);
		ThreadDestroy(threads[i]->handle);
		delete threads[i];
	}
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
