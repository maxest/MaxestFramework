#include "profiler.h"


using namespace NMaxestFramework;
using namespace NEssentials;
using namespace NSystem;


NGPU::CProfiler gGPUProfiler;


// CProfiler


NGPU::CProfiler::CProfiler()
{
	isProfiling = false;
}


void NGPU::CProfiler::Create()
{
	for (uint j = 0; j < BUFFERS_COUNT; j++)
	{
		D3D11_QUERY_DESC qd;
		qd.MiscFlags = 0;
		qd.Query = D3D11_QUERY_TIMESTAMP_DISJOINT;

		MF_ASSERT(device->CreateQuery(&qd, &timestampDisjointQuery[j]) == S_OK);
	}

	file.Open("profiler.txt", CFile::EOpenMode::WriteText);
};


void NGPU::CProfiler::Destroy()
{
	file.Close();

	for (uint i = 0; i < queries.size(); i++)
	{
		for (uint j = 0; j < BUFFERS_COUNT; j++)
		{
			SAFE_RELEASE(queries[i].begin[j]);
			SAFE_RELEASE(queries[i].end[j]);
		}
	}
	for (uint i = 0; i < BUFFERS_COUNT; i++)
		SAFE_RELEASE(timestampDisjointQuery[i]);
};


void NGPU::CProfiler::StartProfiling()
{
	isProfiling = true;
	currentFrameIndex = 0;

	for (uint i = 0; i < queries.size(); i++)
	{
		for (uint j = 0; j < queries[i].frames.size(); j++)
			queries[i].frames[j].used = false;
	}
}


void NGPU::CProfiler::StopProfiling()
{
	if (!isProfiling)
		return;

	if (currentFrameIndex == FRAMES_COUNT_IN_ONE_MEASUREMENT + BUFFERS_COUNT - 1)
	{
		for (uint i = 0; i < queries.size(); i++)
		{
			int usedFramesCount = 0;
			queries[i].avgTime = 0.0;

			for (uint j = 0; j < queries[i].frames.size(); j++)
			{
				if (queries[i].frames[j].used)
				{
					usedFramesCount++;
					queries[i].avgTime += queries[i].frames[j].time;
				}
			}

			if (usedFramesCount > 0)
			{
				queries[i].avgTime /= (float)usedFramesCount;
				file.WriteText(queries[i].name + " - " + ToString(queries[i].avgTime) + "\n");
			}
		}
		file.WriteTextNewline();

		isProfiling = false;
	}
}


void NGPU::CProfiler::StartFrame()
{
	if (!isProfiling)
		return;

	if (currentFrameIndex >= 0 && currentFrameIndex < FRAMES_COUNT_IN_ONE_MEASUREMENT)
		deviceContext->Begin(timestampDisjointQuery[currentFrameIndex % BUFFERS_COUNT]);
}


void NGPU::CProfiler::EndFrame()
{
	if (!isProfiling)
		return;

	//

	if (currentFrameIndex >= 0 && currentFrameIndex < FRAMES_COUNT_IN_ONE_MEASUREMENT)
		deviceContext->End(timestampDisjointQuery[currentFrameIndex % BUFFERS_COUNT]);

	//

	int frameBeingCollectedIndex = (currentFrameIndex - BUFFERS_COUNT + 1);
	int bufferIndex = frameBeingCollectedIndex % BUFFERS_COUNT;

	if (frameBeingCollectedIndex >= 0 && frameBeingCollectedIndex < FRAMES_COUNT_IN_ONE_MEASUREMENT)
	{
		while (deviceContext->GetData(timestampDisjointQuery[bufferIndex], NULL, 0, 0) == S_FALSE)
			Sleep(1);

		D3D11_QUERY_DATA_TIMESTAMP_DISJOINT qdtd;
		deviceContext->GetData(timestampDisjointQuery[bufferIndex], &qdtd, sizeof(qdtd), 0);

		if (!qdtd.Disjoint)
		{
			for (uint i = 0; i < queries.size(); i++)
			{
				uint64 begin, end;

				deviceContext->GetData(queries[i].begin[bufferIndex], &begin, sizeof(uint64), 0);
				deviceContext->GetData(queries[i].end[bufferIndex], &end, sizeof(uint64), 0);

				queries[i].frames[frameBeingCollectedIndex].time = float(end - begin) / float(qdtd.Frequency) * 1000.0f;
			}
		}
	}

	//

	currentFrameIndex++;
}


int NGPU::CProfiler::Begin(const string& name)
{
	if (!isProfiling)
		return -1;

	if (currentFrameIndex >= 0 && currentFrameIndex < FRAMES_COUNT_IN_ONE_MEASUREMENT)
	{
		int queryIndex = QueryIndex(name);
		if (queryIndex < 0)
		{
			AddQuery(name);
			queryIndex = (int)queries.size() - 1;
		}

		queries[queryIndex].frames[currentFrameIndex].used = true;
		deviceContext->End(queries[queryIndex].begin[currentFrameIndex % BUFFERS_COUNT]);

		return queryIndex;
	}

	return -1;
}


void NGPU::CProfiler::End(int index)
{
	if (!isProfiling)
		return;

	if (currentFrameIndex >= 0 && currentFrameIndex < FRAMES_COUNT_IN_ONE_MEASUREMENT)
	{
		if (queries[index].frames[currentFrameIndex].used)
			deviceContext->End(queries[index].end[currentFrameIndex % BUFFERS_COUNT]);
	}
}


void NGPU::CProfiler::End(const string& name)
{
	if (!isProfiling)
		return;

	int queryIndex = QueryIndex(name);

	if (queryIndex >= 0)
		End(queryIndex);
}


const NGPU::CProfiler::SQuery& NGPU::CProfiler::Query(int index)
{
	return queries[index];
}


const NGPU::CProfiler::SQuery& NGPU::CProfiler::Query(const string& name)
{
	int queryIndex = QueryIndex(name);
	if (queryIndex < 0)
	{
		AddQuery(name);
		queryIndex = (int)queries.size() - 1;
	}

	return queries[queryIndex];
}


void NGPU::CProfiler::AddQuery(const string& name)
{
	SQuery query;

	query.name = name;

	D3D11_QUERY_DESC qd;
	qd.MiscFlags = 0;
	qd.Query = D3D11_QUERY_TIMESTAMP;

	for (int j = 0; j < BUFFERS_COUNT; j++)
	{
		MF_ASSERT(device->CreateQuery(&qd, &query.begin[j]) == S_OK);
		MF_ASSERT(device->CreateQuery(&qd, &query.end[j]) == S_OK);
	}

	queries.push_back(query);
}


int NGPU::CProfiler::QueryIndex(const string& name)
{
	for (uint i = 0; i < queries.size(); i++)
	{
		if (queries[i].name == name)
			return i;
	}

	return -1;
}


// CProfilerScopedQuery


NGPU::CProfilerScopedQuery::CProfilerScopedQuery(const string& name)
{
	queryIndex = gGPUProfiler.Begin(name);
}


NGPU::CProfilerScopedQuery::~CProfilerScopedQuery()
{
	gGPUProfiler.End(queryIndex);
}
