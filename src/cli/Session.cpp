#include <asio.hpp>
#include <cli/Server.h>
#include <cli/Session.h>
#include <cli/Socket.h>
#include <log/log.h>


Session::Session(Server* serverPtr)
	: SessionBase(serverPtr, std::move(std::make_unique<Socket>(this, *serverPtr->getProcessorProxy()->getDispatcher())))
{
	LOG(DEBUG) << LABELS{"cli"} << "Session object was created (id=" << this << ")";
}


Session::~Session()
{
	LOG(DEBUG) << LABELS{"cli"} << "Session object was deleted (id=" << this << ")";
}


std::unique_ptr<Command> Session::createCommand(const char* buffer, std::size_t bufferSize, std::size_t bufferUsed)
{
	auto                     found          = false;
    auto                     commandBuilder = Command::Builder(this).setAsync(false);
    std::size_t              prefixSize     = 0;
	std::size_t              commandSize    = 0;
	std::size_t              suffixSize     = 0;
    std::unique_ptr<Command> commandPtr;

    for (std::size_t i = 0; i < bufferUsed && !found; i++)
    {
		switch ((unsigned char)buffer[i])
		{
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

				// this is a telnet command so it must be at least 2 bytes; otherwise keep receiving
				if ((bufferUsed - prefixSize) >= 2)
				{
					// the second byte must be 240..254; otherwise it is invalid command
					if (((unsigned char)buffer[prefixSize + 1]) < 240 || ((unsigned char)buffer[prefixSize + 1]) > 254)
					{
						found          = true;
						commandSize    = 1;
						commandBuilder = commandBuilder.setHandler(&Command::handleInvalid);
					}
					else switch ((unsigned char)buffer[prefixSize + 1])
					{
						case 237:  // Control Z was pressed
							found          = true;
							commandSize    = 2;
							commandBuilder = commandBuilder.setHandler(&Command::handleNone);
						break;
						case 244:  // Suspend, interrupt or abort the process (IP)
							found          = true;
							commandSize    = 2;
							commandBuilder = commandBuilder.setHandler(&Command::handleCancel);
						break;
						case 253:  // Indicates the request that the other party perform the indicated option (DO)
							found          = true;
							commandSize    = 2;
							suffixSize     = bufferUsed - prefixSize - commandSize;
							commandBuilder = commandBuilder.setHandler(&Command::handleNone);
						break;
						case 254:  // ???
							found          = true;
							commandSize    = 2;
							suffixSize     = bufferUsed - prefixSize - commandSize;
							commandBuilder = commandBuilder.setHandler(&Command::handleNone);
						break;
					}
				}
			break;
		}

		// setting prefix size
		if (found)
		{
			prefixSize = i;
		}
	}

	// if command arrived creating command object
	if (found)
	{
		//for (std::size_t j = 0; j < dataSize; j++) {
		//	std::cout << ((unsigned int)((unsigned char)buffer[j])) << " " << std::flush;
		//}
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
