#include <cli/Messages.h>
#include <sstream>

//#include <iostream>


namespace cli
{
	std::vector<std::string> splitIntoWords(const std::string& stringValue, const char separator)
	{
		std::stringstream stringStream(stringValue);
		std::string       word;
		auto              words = std::vector<std::string>();

		while(std::getline(stringStream, word, separator))
		{
			// TODO: trimming
			// word.trim();
			if (word.size() > 0)
			{
				words.push_back(word);
			}
		}

		return words;
	}


	std::vector<std::string> splitIntoWords(std::string& string, std::string& separators)
	{
		auto words = std::vector<std::string>{string};

		for (unsigned int i = 0; i < separators.size(); i++)
		{
			// saving words in a temp variable so words variable will contain the end result
			auto ws = words;
			words.clear();

			// splitting each word and saving result into words variable
			for (auto word : ws)
			{
				auto w = cli::splitIntoWords(word, separators.c_str()[i]);
				words.insert(words.end(), w.begin(), w.end());
			}
		}

		return words;
	}
}
