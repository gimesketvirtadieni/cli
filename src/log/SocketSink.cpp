#include <cli/Server.h>
#include <cli/Session.h>
#include <cli/Socket.h>
#include <log/log.h>
#include <log/SocketSink.h>


SocketSink::SocketSink(Socket* socketPtr, std::function<bool(g3::LogMessage&)> filter) :
	socketPtr(socketPtr),
	SinkFilter(filter) {

	LOG(DEBUG) << "CLI: SocketSink object was created (id=" << this << ")";
}


SocketSink::~SocketSink() {
	LOG(DEBUG) << "CLI: SocketSink object was deleted (id=" << this << ")";
}


void SocketSink::log(g3::LogMessageMover logEntry) {
	auto logMessage = logEntry.get();

	if (!filter(logMessage)) {

		// creating message string
		std::string logString(
			logMessage.timestamp() + " " +
			logMessage.level() +
			" [" + logMessage.threadID() + "]" +
			" (" + logMessage.file() + ":" + logMessage.line() + ") - " +
			rightTrim(logMessage.message()) +
			"\r\n"
		);

		socketPtr->send(logString.c_str());
	}
}
