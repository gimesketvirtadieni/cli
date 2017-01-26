#include <cli/Action.h>
#include <cli/Actions.h>
#include <cli/Command.h>
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
	LOG(DEBUG) << "CLI: Command object was created (id=" << this << ")";
}


Command::~Command()
{
	LOG(DEBUG) << "CLI: Command object was deleted (id=" << this << ")";
}


std::unique_ptr<Command> Command::createCommand(Session* sessionPtr, const char* buffer, std::size_t bufferSize, std::size_t dataSize) {
	bool                         found          = false;
	std::size_t                  prefixSize     = 0;
	std::size_t                  commandSize    = 0;
	std::size_t                  suffixSize     = 0;
    std::shared_ptr<std::string> prefixPtr;
    std::shared_ptr<std::string> commandPtr;
    std::shared_ptr<std::string> suffixPtr;
    std::function<void(Command*)> handler;
    std::unique_ptr<Command>     cliCommandPtr;
    bool                         async          = false;

	std::cout << "Command::createCommand begin ";
	for (std::size_t j = 0; j < dataSize; j++) {
		std::cout << ((unsigned int)((unsigned char)buffer[j])) << " " << std::flush;
	}
	std::cout << std::endl << std::flush;

	for (std::size_t i = 0; i < dataSize && !found; i++) {
		switch ((unsigned char)buffer[i]) {
			case 0:
				found       = true;
				commandSize = 1;
				handler     = &Command::handleNone;
				break;
			case 3:
				found       = true;
				commandSize = 1;
				handler     = &Command::handleCancel;
				break;
			case 4:
				found       = true;
				commandSize = 1;
				handler     = &Command::handleClose;
				break;
			case 10:
				found       = true;
				commandSize = 1;
				handler     = &Command::handleNone;
				break;
			case 13:
				found       = true;
				commandSize = 1;
				handler     = &Command::handleAction;
				async       = true;
				break;
			case 255:
				cliCommandPtr = createTelnetCommand(sessionPtr, buffer, bufferSize, dataSize, i);
				found         = cliCommandPtr.get();
				break;
		}

		// setting prefix size
		if (found) {
			prefixSize = i;
		}
	}

	// creating command object if needed
	if (found && !cliCommandPtr.get()) {

		std::cout << "prefix: " << std::flush;
		for (std::size_t j = 0; j < prefixSize; j++) {
			std::cout << ((unsigned int)((unsigned char)buffer[j])) << " " << std::flush;
		}
		std::cout << std::endl << std::flush;
		std::cout << "command: " << std::flush;
		for (std::size_t j = prefixSize; j < prefixSize + commandSize; j++) {
			std::cout << ((unsigned int)((unsigned char)buffer[j])) << " " << std::flush;
		}
		std::cout << std::endl << std::flush;
		std::cout << "suffix: " << std::flush;
		for (std::size_t j = prefixSize + commandSize; j < prefixSize + commandSize + suffixSize; j++) {
			std::cout << ((unsigned int)((unsigned char)buffer[j])) << " " << std::flush;
		}
		std::cout << std::endl << std::flush;

		// creating prefix, command and suffix string object
		prefixPtr  = std::make_shared<std::string>(buffer, prefixSize);
		commandPtr = std::make_shared<std::string>(buffer + prefixSize, commandSize);
		suffixPtr  = std::make_shared<std::string>(buffer + prefixSize + commandSize, suffixSize);

		// creating CLI command object
		cliCommandPtr = std::make_unique<Command>(
			sessionPtr,
			prefixPtr,
			commandPtr,
			suffixPtr,
			handler,
			async
		);
	}
	std::cout << "Command::createCommand end" << std::endl << std::flush;

	return std::move(cliCommandPtr);
}


std::unique_ptr<Command> Command::createTelnetCommand(Session* sessionPtr, const char* buffer, std::size_t bufferSize, std::size_t dataSize, std::size_t prefixSize) {
	bool                         found         = false;
	std::size_t                  commandSize   = 0;
	std::size_t                  suffixSize    = 0;
    std::shared_ptr<std::string> prefixPtr;
    std::shared_ptr<std::string> commandPtr;
    std::shared_ptr<std::string> suffixPtr;
    std::function<void(Command*)> handler       = nullptr;
    std::unique_ptr<Command>     cliCommandPtr;

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

		std::cout << "prefix: " << std::flush;
		for (std::size_t j = 0; j < prefixPtr->size(); j++) {
			std::cout << ((unsigned int)((unsigned char)prefixPtr->at(j))) << " " << std::flush;
		}
		std::cout << std::endl << std::flush;
		std::cout << "command: " << std::flush;
		for (std::size_t j = 0; j < commandPtr->size(); j++) {
			std::cout << ((unsigned int)((unsigned char)commandPtr->at(j))) << " " << std::flush;
		}
		std::cout << std::endl << std::flush;
		std::cout << "suffix: " << std::flush;
		for (std::size_t j = 0; j < suffixPtr->size(); j++) {
			std::cout << ((unsigned int)((unsigned char)suffixPtr->at(j))) << " " << std::flush;
		}
		std::cout << std::endl << std::flush;

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

	auto parameters    = splitIntoWords(prefixPtr);
	auto actionHandler = sessionPtr->getServer()->getActions()->findAction(parameters).getHandler();

	// creating a context with all data needed for the action
	Context context(this, parameters);

	// echoing end-of-line back to the client before processing action
	context.getOutput() << cli::Messages::endOfLine;

	// invoking command action handler
	actionHandler(context);

	// displaying prompt message
	sessionPtr->sendPrompt(false);
}


void Command::handleCancel() {
	LOG(DEBUG) << "CLI: Command::handleCancel begin (id=" << this << ")";

	// echoing Control-C sign back to the client
	sessionPtr->getSocket()->send("^C");

	// canceling an action being processed currently
	sessionPtr->cancel();

	// displaying end-of-line and prompt message
	sessionPtr->sendPrompt(true);

	LOG(DEBUG) << "CLI: Command::handleCancel end (id=" << this << ")";
}


void Command::handleClose() {
	LOG(DEBUG) << "CLI: Command::handleClose begin (id=" << this << ")";

	sessionPtr->close();

	LOG(DEBUG) << "CLI: Command::handleClose end (id=" << this << ")";
}


void Command::handleInvalid() {
	std::cout << "Command::handleInvalid begin" << std::endl << std::flush;
	std::cout << "Command::handleInvalid end" << std::endl << std::flush;
}


void Command::handleNone() {
	std::cout << "Command::handleNone begin" << std::endl << std::flush;
	std::cout << "Command::handleNone end" << std::endl << std::flush;
}


void Command::setCancelHandler(std::function<void()> cancelHandler) {
	this->cancelHandler = cancelHandler;
}


std::shared_ptr<std::vector<std::string>> Command::splitIntoWords(std::shared_ptr<std::string> stringPtr) {
	std::stringstream stringStream1(*stringPtr.get());
	std::stringstream stringStream2;
	const char        delimeter1 = ' ';
	const char        delimeter2 = '\t';
	std::string       word1;
	std::string       word2;
	auto              wordsPtr = std::make_shared<std::vector<std::string>>();

	while (std::getline(stringStream1, word1, delimeter1)) {

		// reseting the second string stream
		stringStream2.str(word1);
		stringStream2.clear();

		while (std::getline(stringStream2, word2, delimeter2)) {
			if (!word2.empty()) {
				wordsPtr->push_back(word2);
			}
		}
	}

	return wordsPtr;
}
