#include "../essentials/string.h"
#include "../essentials/file.h"


namespace NMaxestFramework { namespace NCommon
{
	class CConfigFile
	{
	public:
		CConfigFile()
		{
			fileOpened = false;
		}

		void Open(const string& path)
		{
			this->path = path;

			if (file.Open(path, NEssentials::CFile::EOpenMode::ReadText))
			{
				fileOpened = true;

				while (!file.EndOfFile())
				{
					string word1, word2;
					file.ReadText(word1);
					file.ReadText(word2);

					if (word1 != "" && word2 != "")
					{
						pair<string, string> keyValue;
						keyValue.first = word1;
						keyValue.second = word2;
						options.push_back(keyValue);
					}
				}
			}
		}

		void Close()
		{
			if (fileOpened)
				file.Close();
			fileOpened = false;

			if (file.Open(path, NEssentials::CFile::EOpenMode::WriteText))
			{
				for (uint i = 0; i < options.size(); i++)
					file.WriteText(options[i].first + " " + options[i].second + "\n");
				file.Close();
			}
		}

		template<typename TYPE> void Process(const string& key, TYPE& value)
		{
			for (uint i = 0; i < options.size(); i++)
			{
				if (options[i].first == key)
				{
					value = FromString(options[i].second, value);
					return;
				}
			}

			// not found, add
			pair<string, string> keyValue;
			keyValue.first = key;
			keyValue.second = ToString(value);
			options.push_back(keyValue);
		}

	private:
		string path;
		NEssentials::CFile file;
		bool fileOpened;
		vector< pair<string, string> > options;
	};
} }
