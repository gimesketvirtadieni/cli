#include <cli/Context.h>
#include <cli/Command.h>
#include <cli/Session.h>
#include <cli/Socket.h>
#include <log/log.h>


Context::Context(Command* c)
	: Context(c, std::shared_ptr<std::vector<std::string>>())
{
	LOG(DEBUG) << "CLI: Context object was created (id=" << this << ")";
}


Context::Context(Command* c, std::shared_ptr<std::vector<std::string>> p)
	: commandPtr(c)
	, buffer(c->getSession()->getSocket())
	, output(&buffer)
	, parameters(p)
{
	// TODO: validate
	if (!parameters)
	{
		LOG(DEBUG) << "CLI: Context (!parameters)==true (id=" << this << ")";
		parameters = std::make_shared<std::vector<std::string>>();
	} else {
		LOG(DEBUG) << "CLI: Context (!parameters)==false (id=" << this << ")";
	}

	LOG(DEBUG) << "CLI: Context object was deleted (id=" << this << ")";
}


g3::LogWorker& Context::getLogger()
{
	return *commandPtr->getSession()->getServer()->getLogger();
}


std::ostream& Context::getOutput()
{
	return output;
}


Socket& Context::getSocket()
{
	return *commandPtr->getSession()->getSocket();
}


void Context::setCancelHandler(std::function<void()> cancelHandler)
{
	commandPtr->setCancelHandler(cancelHandler);
}
