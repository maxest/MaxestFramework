#pragma once


#include "../essentials/main.h"

#ifdef MAXEST_FRAMEWORK_WINDOWS
	#include <Windows.h>
#endif


namespace NMaxestFramework { namespace NSystem
{
	class CFile
	{
	public:
		enum class EOpenMode
		{
			ReadText,
			ReadBinary,
			WriteText,
			WriteBinary,
		};

	public:
		bool Open(const string& path, EOpenMode openMode)
		{
			ios_base::openmode openMode_stl = (ios_base::openmode)0;

			if (openMode == EOpenMode::ReadText)
				openMode_stl = ios::in;
			if (openMode == EOpenMode::ReadBinary)
				openMode_stl = ios::in | ios::binary;
			if (openMode == EOpenMode::WriteText)
				openMode_stl = ios::out;
			if (openMode == EOpenMode::WriteBinary)
				openMode_stl = ios::out | ios::binary;

			file.open(path.c_str(), openMode_stl);

			if (file)
				return true;
			else
				return false;
		}

		void Close()
		{
			file.close();
		}

		bool EndOfFile()
		{
			return file.eof();
		}

		void Seek(int position)
		{
			file.clear();
			file.seekg(position, ios::beg);
		}

		int ReadBin(char* data, int size)
		{
			file.read(data, size);
			return (int)file.gcount();
		}

		int ReadBin(string& s)
		{
			uint32 length;
			ReadBin((char*)&length, sizeof(uint32));

			char* data = new char[length + 1];
			ReadBin(data, length);
			data[length] = '\0';

			s = string(data);
			delete[] data;

			return length;
		}

		int ReadLine(string& s)
		{
			char c;
			string line = "";
			int length = 0;

			while (ReadBin(&c, 1))
			{
				if (c == '\n')
					break;

				line += c;
				length++;
			}

			s = line;
			return length;
		}

		int ReadLines(vector<string>& lines)
		{
			int length = 0;

			while (!EndOfFile())
			{
				string s;
				length += ReadLine(s);
				lines.push_back(s);
			}

			return length;
		}

		int WriteBin(char* data, int size)
		{
			int before = (int)file.tellp();
			file.write(data, size);
			return ((int)file.tellp() - before);
		}

		int WriteBin(const string& s)
		{
			uint32 bytesWritten = 0;

			uint32 length = (uint32)s.length();
			bytesWritten += WriteBin((char*)&length, sizeof(uint32));
			bytesWritten += WriteBin((char*)s.c_str(), length);

			return bytesWritten;
		}

		bool ReadText(bool& b) { file >> b; return b; }
		int32 ReadText(int32& i) { file >> i; return i; }
		uint32 ReadText(uint32& i) { file >> i; return i; }
		int16 ReadText(int16& i) { file >> i; return i; }
		uint16 ReadText(uint16& i) { file >> i; return i; }
		float ReadText(float& f) { file >> f; return f; }
		double ReadText(double& d) { file >> d; return d; }
		string ReadText(string& s) { file >> s; return s; }

		void WriteText(const bool& b) { file << b; }
		void WriteText(const int32& i) { file << i; }
		void WriteText(const uint32& i) { file << i; }
		void WriteText(const int16& i) { file << i; }
		void WriteText(const uint16& i) { file << i; }
		void WriteText(const float& f) { file << f; }
		void WriteText(const double& d) { file << d; }
		void WriteText(const string& s) { file << s; }
		void WriteText(const char* s) { file << string(s); }
		void WriteTextNewline() { file << "\n"; }

	private:
		fstream file;
	};

	bool CreateDir(const string& path);
	int FileOpenAndRead(const string& path, vector<string>& lines);
	int FileOpenAndRead(const string& path, string& s);
	bool FileOpenAndWrite(const string& path, const uint8* data, int dataSize);
	bool FileOpenAndWrite(const string& path, const vector<string>& lines);
	bool FileExists(const string& path);
	bool FileMoreRecentlyModified(const string& path, const string& refFilePath);

	//

	inline bool CreateDir(const string& path)
	{
	#ifdef MAXEST_FRAMEWORK_WINDOWS
		BOOL result = CreateDirectoryA(path.c_str(), 0);
		return (result == 0 ? false : true);
	#else
		MF_ASSERT(false);
	#endif
	}

	inline int FileOpenAndRead(const string& path, vector<string>& lines)
	{
		CFile file;
		if (file.Open(path, CFile::EOpenMode::ReadText))
		{
			int length = file.ReadLines(lines);
			file.Close();
			return length;
		}

		lines.clear();
		return 0;
	}

	inline int FileOpenAndRead(const string& path, string& content)
	{
		vector<string> lines;

		if (int length = FileOpenAndRead(path, lines))
		{
			content = NEssentials::Merge(lines);
			return length;
		}
		else
		{
			content = "";
			return 0;
		}
	}

	inline bool FileOpenAndWrite(const string& path, const uint8* data, int dataSize)
	{
		CFile file;
		if (file.Open(path, CFile::EOpenMode::WriteBinary))
		{
			file.WriteBin((char*)data, dataSize);
			file.Close();
			return true;
		}

		return false;
	}

	inline bool FileOpenAndWrite(const string& path, const vector<string>& lines)
	{
		CFile file;
		if (file.Open(path, CFile::EOpenMode::WriteBinary))
		{
			for (uint i = 0; i < lines.size(); i++)
			{
				file.WriteText(lines[i]);
				file.WriteTextNewline();
			}
			file.Close();
			return true;
		}

		return false;
	}

	inline bool FileExists(const string& path)
	{
		CFile file;
		if (file.Open(path, CFile::EOpenMode::ReadText))
		{
			file.Close();
			return true;
		}
		else
		{
			return false;
		}
	}

	inline bool FileMoreRecentlyModified(const string& path, const string& refFilePath)
	{
	#ifdef MAXEST_FRAMEWORK_WINDOWS
		HANDLE fileHandle;
		FILETIME ft1, ft2, ft3;

		fileHandle = CreateFileW(NEssentials::StringToWString(path).c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, NULL);
		if (fileHandle == INVALID_HANDLE_VALUE)
			return true;
		GetFileTime(fileHandle, &ft1, &ft2, &ft3);
		uint64 t1 = ((uint64)ft3.dwHighDateTime << 32) | ((uint64)ft3.dwLowDateTime);
		CloseHandle(fileHandle);

		fileHandle = CreateFileW(NEssentials::StringToWString(refFilePath).c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, NULL);
		if (fileHandle == INVALID_HANDLE_VALUE)
			return true;
		GetFileTime(fileHandle, &ft1, &ft2, &ft3);
		uint64 t2 = ((uint64)ft3.dwHighDateTime << 32) | ((uint64)ft3.dwLowDateTime);
		CloseHandle(fileHandle);

		return t1 > t2;
	#else
		MF_ASSERT(false);
	#endif
	}
} }
