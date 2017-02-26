#ifndef Context_INCLUDED
#define Context_INCLUDED

#include <cli/Socket.h>
#include <cli/SocketBuffer.h>
#include <functional>
#include <g3log/logworker.hpp>
#include <string>


// forward declaration
class Actions;
class Command;


class Context
{
	public:
		explicit                  Context(Command*, std::vector<std::string>);
		virtual                  ~Context();
		Actions&                  getActions();
		g3::LogWorker&            getLogger();
		std::ostream&             getOutput();
		std::vector<std::string>& getParameters();
		Socket&                   getSocket();
		void                      setCancelHandler(std::function<void()>);

	private:
		Command*                 commandPtr;
		SocketBuffer             buffer;
		std::ostream             output;
		std::vector<std::string> parameters;
};

#endif // Context_INCLUDED
