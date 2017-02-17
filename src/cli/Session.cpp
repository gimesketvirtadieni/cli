#include <asio.hpp>
#include <cli/Server.h>
#include <cli/Session.h>
#include <cli/Socket.h>
#include <log/log.h>


Session::Session(Server* serverPtr) :
	SessionBase(serverPtr, std::move(std::make_unique<Socket>(this, *serverPtr->getProcessorProxy()->getDispatcher()))) {
	LOG(DEBUG) << g3::Labels{"cli"} << "Session object was created (id=" << this << ")";
}


Session::~Session() {
	LOG(DEBUG) << g3::Labels{"cli"} << "Session object was deleted (id=" << this << ")";
}


std::unique_ptr<Command> Session::createCommand(const char* buffer, std::size_t bufferSize, std::size_t bufferUsed) {
	return std::move(Command::createCommand(this, buffer, bufferSize, bufferUsed));
}
