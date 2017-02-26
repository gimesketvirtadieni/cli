#ifndef Messages_INCLUDED
#define Messages_INCLUDED

#include <string>
#include <vector>


namespace cli {
	namespace Messages {
		const char* const endOfLine         = "\r\n";
		const char* const logout            = "logout";
		const char* const negotiateCommands =
			"\xFF\xFB\x03"   // WILL SUPRESS GO AHEAD
			"\xFF\xFB\x01"   // WILL ECHO
			"\xFF\xFD\x03"   // DO SUPRESS GO AHEAD
			"\xFF\xFD\x01";  // DO ECHO
	};

	std::string              leftTrim(const std::string& s);
	std::string              rightTrim(const std::string& s);
	std::vector<std::string> splitIntoWords(const std::string& string, const char separator);
	std::vector<std::string> splitIntoWords(std::string& string, std::string& separators);
	std::string              trim(const std::string& s);
};

#endif // Messages_INCLUDED
