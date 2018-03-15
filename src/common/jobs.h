#pragma once


#include "../essentials/main.h"
#include "../system/threads.h"


#undef AddJob // because it may happen that shitty Windows will include some file that defines AddJob to something else...


namespace NMaxestFramework { namespace NCommon
{
	class CJob;
	class CJobGroup;
	class CJobSystem;

	class CJob
	{
	public:
		CJob()
		{
			done = false;
			owner = nullptr;
		}
		virtual ~CJob() {}

		virtual bool Do() = 0;

	public:
		bool done;
		CJobGroup* owner;
	};

	class CJobGroup
	{
	public:
		CJobGroup()
		{
			waitingSemaphore = NSystem::SemaphoreCreate(0, 1);
			jobsCount = 0;
			jobsDoneCount = 0;
		}
		~CJobGroup()
		{
			NSystem::SemaphoreDestroy(waitingSemaphore);
		}

		void AddJob(CJob* job);
		void Wait();

	public:
		void OnJobDone();

	public:
		NSystem::TSemaphoreHandle waitingSemaphore;
		int32 jobsCount;
		int32 jobsDoneCount;
		vector<CJob*> jobs;
	};

	class CJobSystem
	{
	public:
		void Create(int threadsCount);
		void Destroy();

		void AddJob(CJob* job);
		void AddJobGroup(const CJobGroup& jobGroup);

	public:
		NSystem::TSemaphoreHandle activeJobsSemaphore;
		NSystem::TMutexHandle jobsQueueMutex;
		vector<NSystem::TThreadHandle> threads;
		queue<CJob*> jobs;
	};
} }
