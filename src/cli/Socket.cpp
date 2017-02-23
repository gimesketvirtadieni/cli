#include <cli/Server.h>
#include <cli/Session.h>
#include <cli/Socket.h>
#include <log/log.h>


Socket::Socket(Session* sessionPtr, asio::io_service& dispatcher)
	: sessionPtr(sessionPtr)
	, nativeSocket(dispatcher) {

	LOG(DEBUG) << LABELS{"cli"} << "Socket object was created (id=" << this << ")";
}


Socket::~Socket() {

	// closing the socket
	close();

	LOG(DEBUG) << LABELS{"cli"} << "Socket object was deleted (id=" << this << ")";
}


void Socket::close() {
	if (nativeSocket.is_open()) {
		LOG(DEBUG) << LABELS{"cli"} << "Closing socket (id=" << this << ")...";

		nativeSocket.shutdown(asio::socket_base::shutdown_both);
		nativeSocket.close();

		LOG(DEBUG) << LABELS{"cli"} << "Socket was closed (id=" << this << ")";
	}
}


Session* Socket::getSession() {
	return sessionPtr;
}


void Socket::open(std::function<void(const std::error_code&)> callback) {
	auto acceptorPtr  = sessionPtr->getServer()->getAcceptor();
	if (acceptorPtr) {
		LOG(DEBUG) << LABELS{"cli"} << "Opening socket (id=" << this << ")...";

		// registering callback on open event
		acceptorPtr->async_accept(
			nativeSocket,
			callback
		);

		LOG(DEBUG) << LABELS{"cli"} << "Socket was opened for incoming requests (id=" << this << ")";
	} else {
		LOG(DEBUG) << LABELS{"cli"} << "Could not open socket due to unset acceptor (id=" << this << ")";
	}
}


void Socket::receive(char* data, const std::size_t size, std::function<void(const std::error_code& error, std::size_t bytes_transferred)> callback) {

	// no need to check if socket is open here as callback must receive an error if any
	nativeSocket.async_read_some(
		asio::buffer(data, size),
		callback
	);
}


void Socket::send(const char* string) {
	send(string, strlen(string));
}


void Socket::send(const char* data, const std::size_t size) {

	// writting to a socket
	if (nativeSocket.is_open()) {
		nativeSocket.send(asio::buffer(data, size));
	}
}


void Socket::send(std::shared_ptr<std::string> stringPtr) {
	send(stringPtr->c_str(), stringPtr->size());
}
