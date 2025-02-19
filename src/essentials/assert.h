#pragma once


#include "string.h"
#include "common.h"

#include <cstdio>
#ifdef MAXEST_FRAMEWORK_WINDOWS
	#include <Windows.h>
	#include <DbgHelp.h>
#else
	#include <signal.h>
#endif


#ifdef MAXEST_FRAMEWORK_WINDOWS
	#pragma comment(lib, "dbghelp.lib")
#endif


#define MF_ASSERT(condition)					NMaxestFramework::NEssentials::Assert(condition, __FILE__, __LINE__)
#define MF_ASSERT_INTS_EQUAL(value1, value2)	NMaxestFramework::NEssentials::AssertIntsEqual(value1, value2, __FILE__, __LINE__)
#define MF_ASSERT_INTS_GREATER(value1, value2)	NMaxestFramework::NEssentials::AssertIntsGreater(value1, value2, __FILE__, __LINE__) // value1 greater than value2


namespace NMaxestFramework { namespace NEssentials
{
	inline std::vector<std::string> Callstack(bool& callstackValid)
	{
		callstackValid = true;
		vector<string> entries;

	#ifdef MAXEST_FRAMEWORK_WINDOWS
		// taken from http://blog.aaronballman.com/2011/04/generating-a-stack-crawl/

		::SymSetOptions(SYMOPT_DEFERRED_LOADS | SYMOPT_INCLUDE_32BIT_MODULES | SYMOPT_UNDNAME);

		if (!::SymInitialize(::GetCurrentProcess(), "http://msdl.microsoft.com/download/symbols", TRUE))
		{
			callstackValid = false;
			return entries;
		}

		void* addrs[25] = { 0 };
		WORD framesCount = CaptureStackBackTrace(0, 25, addrs, NULL);

		for (WORD i = 0; i < framesCount; i++)
		{
			SYMBOL_INFO* info = (SYMBOL_INFO*)malloc(sizeof(SYMBOL_INFO) + 1024);
			info->SizeOfStruct = sizeof(SYMBOL_INFO);
			info->MaxNameLen = 1024;

			IMAGEHLP_LINE64* line = (IMAGEHLP_LINE64*)malloc(sizeof(IMAGEHLP_LINE64));
			line->SizeOfStruct = sizeof(IMAGEHLP_LINE64);

			DWORD64 displacement1 = 0;
			DWORD displacement2 = 0;
			if (::SymFromAddr(::GetCurrentProcess(), (DWORD64)addrs[i], &displacement1, info))
			{
				if (::SymGetLineFromAddr64(::GetCurrentProcess(), (DWORD64)addrs[i], &displacement2, line))
				{
					entries.push_back(string(line->FileName) + ":" + ToString(line->LineNumber) + " (called in " + string(info->Name) + ")");
				}
			}

			free(info);
			free(line);
		}

		::SymCleanup(::GetCurrentProcess());
	#else
		callstackValid = false;
	#endif

		return entries;
	}

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
			fprintf(file, "%s\n", CurrentTime().c_str());
			fprintf(file, "%s:%d\n\n", fileName, line);
			{
				bool callstackValid;
				vector<string> callstackEntries = Callstack(callstackValid);

				fprintf(file, "callstack valid: %d\n", (int)callstackValid);
				for (uint i = 0; i < callstackEntries.size(); i++)
					fprintf(file, "%s\n", callstackEntries[i].c_str());
			}
			fclose(file);

		#ifdef MAXEST_FRAMEWORK_WINDOWS
			__debugbreak();
		#else
			raise(SIGTRAP);
		#endif
		}
	}

	inline void AssertIntsEqual(int value1, int value2, const char* fileName, int line)
	{
		if (!(value1 == value2))
		{
			FILE* file;
		#ifdef MAXEST_FRAMEWORK_WINDOWS
			fopen_s(&file, "assert.txt", "w");
		#else
			file = fopen("assert.txt", "w");
		#endif
			fprintf(file, "%s\n", CurrentTime().c_str());
			fprintf(file, "%s:%d\n", fileName, line);
			fprintf(file, "value1: %d\n", value1);
			fprintf(file, "value2: %d\n\n", value2);
			{
				bool callstackValid;
				vector<string> callstackEntries = Callstack(callstackValid);

				fprintf(file, "callstack valid: %d\n", (int)callstackValid);
				for (uint i = 0; i < callstackEntries.size(); i++)
					fprintf(file, "%s\n", callstackEntries[i].c_str());
			}
			fclose(file);

		#ifdef MAXEST_FRAMEWORK_WINDOWS
			__debugbreak();
		#else
			raise(SIGTRAP);
		#endif
		}
	}

	inline void AssertIntsGreater(int value1, int value2, const char* fileName, int line)
	{
		if (!(value1 > value2))
		{
			FILE* file;
		#ifdef MAXEST_FRAMEWORK_WINDOWS
			fopen_s(&file, "assert.txt", "w");
		#else
			file = fopen("assert.txt", "w");
		#endif
			fprintf(file, "%s\n", CurrentTime().c_str());
			fprintf(file, "%s:%d\n", fileName, line);
			fprintf(file, "value1: %d\n", value1);
			fprintf(file, "value2: %d\n\n", value2);
			{
				bool callstackValid;
				vector<string> callstackEntries = Callstack(callstackValid);

				fprintf(file, "callstack valid: %d\n", (int)callstackValid);
				for (uint i = 0; i < callstackEntries.size(); i++)
					fprintf(file, "%s\n", callstackEntries[i].c_str());
			}
			fclose(file);

		#ifdef MAXEST_FRAMEWORK_WINDOWS
			__debugbreak();
		#else
			raise(SIGTRAP);
		#endif
		}
	}
} }
