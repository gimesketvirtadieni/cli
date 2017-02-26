#include <cli/Action.h>
#include <cli/Actions.h>
#include <cli/Command.h>
#include <cli/Messages.h>
#include <cli/Server.h>
#include <cli/Session.h>
#include <cli/Socket.h>
#include <cli/SocketBuffer.h>
#include <log/log.h>

#include <iostream>


Command::Command(Session* sessionPtr, std::shared_ptr<std::string> prefixPtr, std::shared_ptr<std::string> commandPtr, std::shared_ptr<std::string> suffixPtr, std::function<void(Command*)> handler, bool async)
	: prefixPtr(prefixPtr)
	, commandPtr(commandPtr)
	, suffixPtr(suffixPtr)
	, handler([this, handler] {handler(this);})
	, sessionPtr(sessionPtr)
	, async(async)
	, cancelHandler(0)
{
	LOG(DEBUG) << LABELS{"cli"} << "Command object was created (id=" << this << ")";
}


Command::~Command()
{
	LOG(DEBUG) << LABELS{"cli"} << "Command object was deleted (id=" << this << ")";
}


std::unique_ptr<Command> Command::createCommand(Session* sessionPtr, const char* buffer, std::size_t bufferSize, std::size_t dataSize) {
	auto                     found          = false;
    auto                     commandBuilder = Command::Builder(sessionPtr).setAsync(false);
    std::size_t              prefixSize     = 0;
	std::size_t              commandSize    = 0;
	std::size_t              suffixSize     = 0;
    std::unique_ptr<Command> commandPtr;

	//for (std::size_t j = 0; j < dataSize; j++) {
	//	std::cout << ((unsigned int)((unsigned char)buffer[j])) << " " << std::flush;
	//}

    for (std::size_t i = 0; i < dataSize && !found; i++) {
		switch ((unsigned char)buffer[i]) {
			case 0:
				found          = true;
				commandSize    = 1;
				commandBuilder = commandBuilder.setHandler(&Command::handleNone);
				break;
			case 3:
				found          = true;
				commandSize    = 1;
				commandBuilder = commandBuilder.setHandler(&Command::handleCancel);
				break;
			case 4:
				found          = true;
				commandSize    = 1;
				commandBuilder = commandBuilder.setHandler(&Command::handleClose);
				break;
			case 10:
				found          = true;
				commandSize    = 1;
				commandBuilder = commandBuilder.setHandler(&Command::handleNone);
				break;
			case 13:
				found          = true;
				commandSize    = 1;
				commandBuilder = commandBuilder.setHandler(&Command::handleAction);
				commandBuilder = commandBuilder.setAsync(true);
				break;
			case 255:
				commandPtr = createTelnetCommand(sessionPtr, buffer, bufferSize, dataSize, i);
				found      = commandPtr.get();
				break;
		}

		// setting prefix size
		if (found) {
			prefixSize = i;
		}
	}

	// creating command object if needed
	if (found && !commandPtr.get()) {

		//std::cout << "prefix: " << std::flush;
		//for (std::size_t j = 0; j < prefixSize; j++) {
		//	std::cout << ((unsigned int)((unsigned char)buffer[j])) << " " << std::flush;
		//}
		//std::cout << std::endl << std::flush;
		//std::cout << "command: " << std::flush;
		//for (std::size_t j = prefixSize; j < prefixSize + commandSize; j++) {
		//	std::cout << ((unsigned int)((unsigned char)buffer[j])) << " " << std::flush;
		//}
		//std::cout << std::endl << std::flush;
		//std::cout << "suffix: " << std::flush;
		//for (std::size_t j = prefixSize + commandSize; j < prefixSize + commandSize + suffixSize; j++) {
		//	std::cout << ((unsigned int)((unsigned char)buffer[j])) << " " << std::flush;
		//}
		//std::cout << std::endl << std::flush;

		// creating prefix, command and suffix string object
		commandBuilder.setPrefix(std::string(buffer, prefixSize));
		commandBuilder.setCommand(std::string(buffer + prefixSize, commandSize));
		commandBuilder.setSuffix(std::string(buffer + prefixSize + commandSize, suffixSize));

		// creating CLI command object
		commandPtr = commandBuilder.build();
	}

	return std::move(commandPtr);
}


std::unique_ptr<Command> Command::createTelnetCommand(Session* sessionPtr, const char* buffer, std::size_t bufferSize, std::size_t dataSize, std::size_t prefixSize) {
	bool                          found         = false;
	std::size_t                   commandSize   = 0;
	std::size_t                   suffixSize    = 0;
    std::shared_ptr<std::string>  prefixPtr;
    std::shared_ptr<std::string>  commandPtr;
    std::shared_ptr<std::string>  suffixPtr;
    std::function<void(Command*)> handler       = nullptr;
    std::unique_ptr<Command>      cliCommandPtr;

	// telnet command must start with 255
	if (!found && ((unsigned char)buffer[prefixSize]) != 255) {
		found   = true;
		handler = &Command::handleInvalid;
	}

	// telnet command must be at least 2 bytes
	if (!found && (dataSize - prefixSize) < 2) {
		found = true;

		// TODO: if there is just one symbol and it is the last in the buffer then there is a chance to receive the rest of the command
		if ((bufferSize - prefixSize) == 1) {
			handler = &Command::handleNone;
		} else {
			commandSize = 1;
			handler     = &Command::handleInvalid;
		}
	}

	// figuring out command type
	if (!found) switch ((unsigned char)buffer[prefixSize + 1]) {
		case 237:  // Control Z was pressed
			found       = true;
			commandSize = 2;
			handler     = &Command::handleNone;
			break;
		case 244:  // Suspend, interrupt or abort the process (IP)
			found       = true;
			commandSize = 2;
			handler     = &Command::handleCancel;
			break;
		case 253:  // Indicates the request that the other party perform the indicated option (DO)
			found       = true;
			commandSize = 2;
			suffixSize  = dataSize - prefixSize - commandSize;
			handler     = &Command::handleNone;
			break;
		case 254:  // ???
			found       = true;
			commandSize = 2;
			suffixSize  = dataSize - prefixSize - commandSize;
			handler     = &Command::handleNone;
			break;
	}

	// if it is still unknown command the second byte must be 240..254
	if (!found && (((unsigned char)buffer[prefixSize + 1]) < 240 || ((unsigned char)buffer[prefixSize + 1]) > 254)) {
		found       = true;
		commandSize = 1;
		handler     = &Command::handleInvalid;
	}

	if (found) {

		// creating prefix, command and suffix string object
		prefixPtr  = std::make_shared<std::string>(buffer, prefixSize);
		commandPtr = std::make_shared<std::string>(buffer + prefixSize, commandSize);
		suffixPtr  = std::make_shared<std::string>(buffer + prefixSize + commandSize, suffixSize);

		//std::cout << "prefix: " << std::flush;
		//for (std::size_t j = 0; j < prefixPtr->size(); j++) {
		//	std::cout << ((unsigned int)((unsigned char)prefixPtr->at(j))) << " " << std::flush;
		//}
		//std::cout << std::endl << std::flush;
		//std::cout << "command: " << std::flush;
		//for (std::size_t j = 0; j < commandPtr->size(); j++) {
		//	std::cout << ((unsigned int)((unsigned char)commandPtr->at(j))) << " " << std::flush;
		//}
		//std::cout << std::endl << std::flush;
		//std::cout << "suffix: " << std::flush;
		//for (std::size_t j = 0; j < suffixPtr->size(); j++) {
		//	std::cout << ((unsigned int)((unsigned char)suffixPtr->at(j))) << " " << std::flush;
		//}
		//std::cout << std::endl << std::flush;

		// creating CLI command object
		cliCommandPtr = std::make_unique<Command>(
			sessionPtr,
			prefixPtr,
			commandPtr,
			suffixPtr,
			handler,
			false
		);
	}

	return std::move(cliCommandPtr);
}


std::function<void()> Command::getCancelHandler() {
	return cancelHandler;
}


std::shared_ptr<std::string> Command::getCommand() {
	return commandPtr;
}


std::function<void()> Command::getHandler() {
	return handler;
}


std::shared_ptr<std::string> Command::getPrefix() {
	return prefixPtr;
}


Session* Command::getSession() {
	return sessionPtr;
}


std::size_t Command::getSize() {
	std::size_t size = 0;

	if (prefixPtr.get()) {
		size += prefixPtr->size();
	}
	if (commandPtr.get()) {
		size += commandPtr->size();
	}
	if (suffixPtr.get()) {
		size += suffixPtr->size();
	}

	return size;
}


std::shared_ptr<std::string> Command::getSuffix() {
	return suffixPtr;
}


bool Command::isAsync() {
	return async;
}


void Command::handleAction() {
	bool        endl{true};
	std::string separators{" \t"};
	auto        parameters = cli::splitIntoWords(*prefixPtr, separators);

	if (parameters.size() > 0) {
		endl = false;

		// creating a context with all data needed for the action
		Context context(this, parameters);

		// echoing end-of-line back to the client before processing action
		context.getOutput() << cli::Messages::endOfLine;

		// invoking command action handler
		sessionPtr->getServer()->getActions()->findAction(parameters)(context);
	}

	// displaying prompt message
	sessionPtr->sendPrompt(endl);
}


void Command::handleCancel() {
	LOG(DEBUG) << LABELS{"cli"} << "Command::handleCancel begin (id=" << this << ")";

	// echoing Control-C sign back to the client
	sessionPtr->getSocket()->send("^C");

	// canceling an action being processed currently
	sessionPtr->cancel();

	// displaying end-of-line and prompt message
	sessionPtr->sendPrompt(true);

	LOG(DEBUG) << LABELS{"cli"} << "Command::handleCancel end (id=" << this << ")";
}


void Command::handleClose() {
	LOG(DEBUG) << LABELS{"cli"} << "Command::handleClose begin (id=" << this << ")";

	sessionPtr->close();

	LOG(DEBUG) << LABELS{"cli"} << "Command::handleClose end (id=" << this << ")";
}


void Command::handleInvalid() {
	LOG(DEBUG) << LABELS{"cli"} << "Command::handleInvalid begin (id=" << this << ")";
	LOG(DEBUG) << LABELS{"cli"} << "Command::handleInvalid end (id=" << this << ")";
}


void Command::handleNone() {
	LOG(DEBUG) << LABELS{"cli"} << "Command::handleNone begin (id=" << this << ")";
	LOG(DEBUG) << LABELS{"cli"} << "Command::handleNone end (id=" << this << ")";
}


void Command::setCancelHandler(std::function<void()> cancelHandler) {
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
		std::make_shared<std::string>(prefix),
		std::make_shared<std::string>(command),
		std::make_shared<std::string>(suffix),
		handler,
		async
	);
	//auto commandPtr = std::unique_ptr<Command>();
	//return std::move(commandPtr);
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
