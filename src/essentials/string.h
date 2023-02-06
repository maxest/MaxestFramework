#pragma once


#include "types.h"
#include "stl.h"
#include "macros.h"


namespace NMaxestFramework { namespace NEssentials
{
	const string NewLine = "\r\n";

	//

	wstring StringToWString(const string& s);
	string WStringToString(const wstring& s);

	bool ToBool(const string& s);
	int ToInt(const string& s);
	float ToFloat(const string& s);
	double ToDouble(const string& s);
	template<typename TYPE> TYPE FromString(const string& s, const TYPE& dummy); // dummy is here only to force compiler to generate different functions for different parameters; in overloading it's not enough that functions differ only by return's type
	template<typename TYPE> string ToString(const TYPE& value);

	bool Find(const string& str, const string& subStr);
	string Reverse(const string& s);
	string TrimBegin(const string& s);
	string TrimEnd(const string& s);
	string Trim(const string& s);
	vector<string> Split(string s, char separator);
	string Merge(const vector<string>& lines, char separator = '\n');

	int LastSlashIndex(const string& path);
	string ExtractExtension(const string& path);
	string ExtractDir(const string& path);
	string ExtractFileName(const string& path);
	string AddPrefixToFileName(const string& path, const string& prefix);

	//

	inline wstring StringToWString(const string& s)
	{
		return wstring(s.begin(), s.end());
	}

	inline string WStringToString(const wstring& ws)
	{
		string s = "";
		for (size_t i = 0; i < ws.size(); i++)
		{
			s += (char)ws[i];
		}
		return s;
	}

	inline bool ToBool(const string& s)
	{
		bool b;
		istringstream in(s);
		in >> b;
		return b;
	}

	inline int ToInt(const string& s)
	{
		int i;
		istringstream in(s);
		in >> i;
		return i;
	}

	inline float ToFloat(const string& s)
	{
		float f;
		istringstream in(s);
		in >> f;
		return f;
	}

	inline double ToDouble(const string& s)
	{
		double f;
		istringstream in(s);
		in >> f;
		return f;
	}

	template<typename TYPE> TYPE FromString(const string& s, const TYPE& dummy)
	{
		TYPE out;
		istringstream in(s);
		in >> out;
		return out;
	}
	template<> inline bool FromString(const string& s, const bool& dummy)
	{
		UNUSED(dummy);

		if (s == "true")
			return true;
		else
			return false;
	}

	template<typename TYPE> string ToString(const TYPE& value)
	{
		ostringstream out;
		out << value;
		return out.str();
	}
	template<> inline string ToString<bool>(const bool& value)
	{
		if (value)
			return "true";
		else
			return "false";
	}

	inline bool Find(const string& str, const string& subStr)
	{
		return str.find(subStr, 0) != string::npos;
	}

	inline string Reverse(const string& s)
	{
		string temp = s;

		for (uint i = 0; i < s.length(); i++)
			temp[s.length() - 1 - i] = s[i];

		return temp;
	}

	inline string TrimBegin(const string& s)
	{
		if (s.length() == 0)
			return "";

		int offset = 0;
		int newSize = (int)s.length();

		for (int i = 0; i < (int)s.length(); i++)
		{
			if (!isspace(s[i]))
			{
				offset = i;
				newSize = (int)s.length() - i;
				break;
			}
		}

		return s.substr(offset, newSize);
	}

	inline string TrimEnd(const string& s)
	{
		if (s.length() == 0)
			return "";

		int newSize = (int)s.length();

		for (int i = (int)s.length() - 1; i >= 0; i--)
		{
			if (!isspace(s[i]))
			{
				newSize = i + 1;
				break;
			}
		}

		return s.substr(0, newSize);
	}

	inline string Trim(const string& s)
	{
		return TrimBegin(TrimEnd(s));
	}

	inline vector<string> Split(string s, char separator)
	{
		vector<string> strings;
		int currentStringIndex = 0;
		size_t stringEndingCharIndex;

		if (s.empty())
			return strings;

		s += separator;

		while ( (stringEndingCharIndex = s.find(separator, 0)) != string::npos )
		{
			strings.push_back("");

			for (uint i = 0; i < stringEndingCharIndex; i++)
				strings[currentStringIndex].push_back(s[i]);

			s.erase(0, stringEndingCharIndex + 1);
			currentStringIndex++;
		}

		return strings;
	}

	inline string Merge(const vector<string>& lines, char separator)
	{
		string s;

		for (uint i = 0; i < lines.size(); i++)
		{
			s += lines[i];
			if (i < lines.size() - 1)
				s += separator;
		}

		return s;
	}

	inline int LastSlashIndex(const string& path)
	{
		for (int i = (int)path.length() - 1; i > -1; i--)
		{
			if (path[i] == '/')
				return i;
		}

		return -1;
	}

	inline string ExtractExtension(const string& path)
	{
		string extension = "";

		for (int i = (int)path.length() - 1; i > -1; i--)
		{
			if (path[i] == '.')
				return Reverse(extension);
			else
				extension += path[i];
		}

		return "";
	}

	inline string ExtractDir(const string& path)
	{
		return path.substr(0, LastSlashIndex(path) + 1);
	}

	inline string ExtractFileName(const string& path)
	{
		int lastSlashIndex = LastSlashIndex(path);
		return path.substr(lastSlashIndex + 1, path.length() - lastSlashIndex - 1);
	}

	inline string AddPrefixToFileName(const string& path, const string& prefix)
	{
		return ExtractDir(path) + prefix + ExtractFileName(path);
	}
} }
