#pragma once


#include "../essentials/main.h"
#include "../essentials/threads.h"


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
			doneStatus = -1;
			owner = nullptr;
		}
		virtual ~CJob() {}

		virtual int Do() = 0;

		int DoneStatus();
		bool IsDone();

	public: // readonly
		int doneStatus;
		CJobGroup* owner;
	};

	class CJobGroup
	{
	public:
		CJobGroup()
		{
			waitingSemaphore = NEssentials::SemaphoreCreate(0, 1);
			jobsCount = 0;
			jobsDoneCount = 0;
		}
		~CJobGroup()
		{
			NEssentials::SemaphoreDestroy(waitingSemaphore);
		}

		void AddJob(CJob* job);
		void Wait();

	public:
		void OnJobDone();

	public: // readonly
		NEssentials::TSemaphoreHandle waitingSemaphore;
		int32 jobsCount;
		int32 jobsDoneCount;
		vector<CJob*> jobs;
	};

	class CJobSystem
	{
	private:
		struct SThread
		{
			CJobSystem* jobSystem;
			NEssentials::TThreadHandle handle;
			bool requestStop;

			SThread()
			{
				requestStop = false;
			}
		};

	private:
		static THREAD_FUNCTION_RETURN_VALUE JobThread(void* data);

	public:
		void Create(int threadsCount);
		void Destroy();

		void AddJob(CJob* job);
		void AddJobGroup(const CJobGroup& jobGroup);

	public: // readonly
		NEssentials::TSemaphoreHandle activeJobsSemaphore;
		NEssentials::TMutexHandle jobsQueueMutex;
		vector<SThread*> threads;
		queue<CJob*> jobs;
	};
} }
