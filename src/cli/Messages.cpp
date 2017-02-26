#include <algorithm>
#include <cli/Messages.h>
#include <sstream>

//#include <iostream>


namespace cli
{
	std::string leftTrim(const std::string& s)
	{
		auto r = std::find_if_not(s.begin(), s.end(), [](int c)
		{
			return std::isspace(c);
		});

		return std::string(r, s.end());
	}


	std::string rightTrim(const std::string& s)
	{
		auto r = std::find_if_not(s.rbegin(), s.rend(), [](int c)
		{
			return std::isspace(c);
		}).base();

		return std::string(s.begin(), r);
	}


	std::vector<std::string> splitIntoWords(const std::string& stringValue, const char separator)
	{
		std::stringstream stringStream(stringValue);
		std::string       word;
		auto              words = std::vector<std::string>();

		while(std::getline(stringStream, word, separator))
		{
			word = trim(word);
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


	std::string trim(const std::string& s)
	{
		return leftTrim(rightTrim(s));
	}
}
