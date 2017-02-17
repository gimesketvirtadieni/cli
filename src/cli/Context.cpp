#include <cli/Context.h>
#include <cli/Command.h>
#include <cli/Session.h>
#include <cli/Socket.h>
#include <log/log.h>


Context::Context(Command* c, std::vector<std::string> p)
	: commandPtr(c)
	, buffer(c->getSession()->getSocket())
	, output(&buffer)
	, parameters(p)
{
	LOG(DEBUG) << g3::Labels{"cli"} << "Context object was created (id=" << this << ")";
}


Context::~Context()
{
	LOG(DEBUG) << g3::Labels{"cli"} << "Context object was deleted (id=" << this << ")";
}


g3::LogWorker& Context::getLogger()
{
	return *commandPtr->getSession()->getServer()->getLogger();
}


std::ostream& Context::getOutput()
{
	return output;
}


std::vector<std::string>& Context::getParameters()
{
	return parameters;
}


Socket& Context::getSocket()
{
	return *commandPtr->getSession()->getSocket();
}


void Context::setCancelHandler(std::function<void()> cancelHandler)
{
	commandPtr->setCancelHandler(cancelHandler);
}
