#include <cli/Action.h>
#include <cli/Actions.h>
#include <cli/Command.h>
#include <cli/Messages.h>
#include <cli/Server.h>
#include <cli/Session.h>
#include <cli/Socket.h>
#include <cli/SocketBuffer.h>
#include <log/log.h>
#include <memory>

//#include <iostream>


Command::Command(Session* sessionPtr, std::string prefix, std::string command, std::string suffix, std::function<void(Command*)> handler, bool async)
	: sessionPtr(sessionPtr)
	, prefix(prefix)
	, command(command)
	, suffix(suffix)
	, handler([this, handler] {handler(this);})
	, async(async)
	, cancelHandler(0)
{
	LOG(DEBUG) << LABELS{"cli"} << "Command object was created (id=" << this << ")";
}


Command::~Command()
{
	LOG(DEBUG) << LABELS{"cli"} << "Command object was deleted (id=" << this << ")";
}


const std::function<void()>& Command::getCancelHandler()
{
	return cancelHandler;
}


const std::string& Command::getCommand()
{
	return command;
}


const std::function<void()>& Command::getHandler()
{
	return handler;
}


const std::string& Command::getPrefix()
{
	return prefix;
}


Session* Command::getSession()
{
	return sessionPtr;
}


std::size_t Command::getSize()
{
	std::size_t size = 0;

	size += prefix.size();
	size += command.size();
	size += suffix.size();

	return size;
}


const std::string& Command::getSuffix()
{
	return suffix;
}


bool Command::isAsync()
{
	return async;
}


void Command::handleAction()
{
	auto separators = std::string{" \t"};
	auto parameters = cli::splitIntoWords(prefix, separators);
	auto endl       = true;

	if (parameters.size() > 0)
	{
		// creating a context with all data needed for the action
		Context context(this, parameters);

		// echoing end-of-line back to the client before processing action
		context.getOutput() << cli::Messages::endOfLine;

		// invoking command action handler
		sessionPtr->getServer()->getActions()->findAction(parameters)(context);
		endl = false;
	}

	// displaying prompt message
	sessionPtr->sendPrompt(endl);
}


void Command::handleCancel()
{
	LOG(DEBUG) << LABELS{"cli"} << "Command::handleCancel begin (id=" << this << ")";

	// echoing Control-C sign back to the client
	sessionPtr->getSocket()->send("^C");

	// canceling an action being processed currently
	sessionPtr->cancel();

	// displaying end-of-line and prompt message
	sessionPtr->sendPrompt(true);

	LOG(DEBUG) << LABELS{"cli"} << "Command::handleCancel end (id=" << this << ")";
}


void Command::handleClose()
{
	LOG(DEBUG) << LABELS{"cli"} << "Command::handleClose begin (id=" << this << ")";

	sessionPtr->close();

	LOG(DEBUG) << LABELS{"cli"} << "Command::handleClose end (id=" << this << ")";
}


void Command::handleInvalid()
{
	LOG(DEBUG) << LABELS{"cli"} << "Command::handleInvalid begin (id=" << this << ")";
	LOG(DEBUG) << LABELS{"cli"} << "Command::handleInvalid end (id=" << this << ")";
}


void Command::handleNone()
{
	LOG(DEBUG) << LABELS{"cli"} << "Command::handleNone begin (id=" << this << ")";
	LOG(DEBUG) << LABELS{"cli"} << "Command::handleNone end (id=" << this << ")";
}


void Command::setCancelHandler(std::function<void()> cancelHandler)
{
	this->cancelHandler = cancelHandler;
}


Command::Builder::Builder(Session* s)
	: sessionPtr(s)
	, handler(nullptr)
	, async(false)
	, prefix("")
	, command("")
	, suffix("") {}


std::unique_ptr<Command> Command::Builder::build()
{
	// creating CLI command object
	return std::make_unique<Command>(
		sessionPtr,
		prefix,
		command,
		suffix,
		handler,
		async
	);
}


Command::Builder& Command::Builder::setAsync(bool a)
{
	async = a;
	return *this;
}


Command::Builder& Command::Builder::setCommand(std::string c)
{
	command = c;
	return *this;
}


Command::Builder& Command::Builder::setHandler(std::function<void(Command*)> h)
{
	handler = h;
	return *this;
}


Command::Builder& Command::Builder::setPrefix(std::string p)
{
	prefix = p;
	return *this;
}


Command::Builder& Command::Builder::setSuffix(std::string s)
{
	suffix = s;
	return *this;
}
