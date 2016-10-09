#ifndef Messages_INCLUDED
#define Messages_INCLUDED


namespace CLI {
	namespace Messages {
		const char* const negotiateCommands =
		"\xFF\xFB\x03"   // WILL SUPRESS GO AHEAD
		"\xFF\xFB\x01"   // WILL ECHO
		"\xFF\xFD\x03"   // DO SUPRESS GO AHEAD
		"\xFF\xFD\x01";  // DO ECHO
	};
};

#endif // Messages_INCLUDED
