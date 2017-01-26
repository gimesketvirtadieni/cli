#ifndef Context_INCLUDED
#define Context_INCLUDED

#include <cli/Socket.h>
#include <cli/SocketBuffer.h>
#include <g3log/logworker.hpp>
#include <memory>
#include <utility>
#include <vector>


// forward declaration
class Command;


class Context
{
	public:
		explicit       Context(Command*);
		explicit       Context(Command*, std::shared_ptr<std::vector<std::string>>);
		g3::LogWorker& getLogger();
		std::ostream&  getOutput();
		Socket&        getSocket();
		void           setCancelHandler(std::function<void()>);

	private:
		Command*                                  commandPtr;
		SocketBuffer                              buffer;
		std::ostream                              output;
		std::shared_ptr<std::vector<std::string>> parameters;
};

#endif // Context_INCLUDED
