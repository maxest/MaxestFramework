#pragma once


#include "d3d11.h"

#include "../essentials/stl.h"
#include "../../src/system/file.h"


#define FRAMES_COUNT_IN_ONE_MEASUREMENT		100
#define BUFFERS_COUNT						4


namespace NMaxestFramework { namespace NGPU
{
	class CProfiler
	{
	public:
		struct SQuery
		{
			struct SFrame
			{
				bool used;
				float time;

				SFrame()
				{
					used = false;
				}
			};

			string name;
			vector<SFrame> frames;
			float avgTime;

			ID3D11Query* begin[BUFFERS_COUNT];
			ID3D11Query* end[BUFFERS_COUNT];

			SQuery()
			{
				frames.resize(FRAMES_COUNT_IN_ONE_MEASUREMENT);
			}
		};

	public:
		CProfiler();

		void Create();
		void Destroy();

		void StartProfiling();
		void StopProfiling();

		void StartFrame();
		void EndFrame();

		int Begin(const string& name);
		void End(int index);
		void End(const string& name);

		const SQuery& Query(int index);
		const SQuery& Query(const string& name);

	public: // readonly
		void AddQuery(const string& name);
		int QueryIndex(const string& name);

	public: // readonly
		bool isProfiling;
		int currentFrameIndex;

		vector<SQuery> queries;
		ID3D11Query* timestampDisjointQuery[BUFFERS_COUNT];

		NSystem::CFile file;
	};

	class CProfilerScopedQuery
	{
	public:
		CProfilerScopedQuery(const string& name);
		~CProfilerScopedQuery();

	public: // readonly
		int queryIndex;
	};
} }


extern NMaxestFramework::NGPU::CProfiler gGPUProfiler;
