#include <cli/Server.h>
#include <cli/Session.h>
#include <cli/Socket.h>
#include <log/log.h>


Socket::Socket(Session* sessionPtr, asio::io_service* dispatcherPtr) :
	sessionPtr(sessionPtr) {

	// creating native socket
	nativeSocketPtr.reset(
		new asio::ip::tcp::socket(
			*dispatcherPtr
		),

		// using empty deletor as socket object will be deleted explicitly by destructor
		[](asio::ip::tcp::socket*) {}
	);

	LOG(DEBUG) << "CLI: Socket object was created (id=" << this << ")";
}


Socket::~Socket() {

	// closing the socket
	close();

	nativeSocketPtr.reset();

	// deleting socket object explicitly since empty deleter is used
	delete nativeSocketPtr.get();

	LOG(DEBUG) << "CLI: Socket object was deleted (id=" << this << ")";
}


void Socket::close() {
	if (nativeSocketPtr->is_open()) {
		LOG(DEBUG) << "CLI: Closing socket (id=" << this << ")...";

		nativeSocketPtr->shutdown(asio::socket_base::shutdown_both);
		nativeSocketPtr->close();

		LOG(DEBUG) << "CLI: Socket was closed (id=" << this << ")";
	}
}


Session* Socket::getSession() {
	return sessionPtr;
}


void Socket::open(std::function<void(const std::error_code&)> callback) {
	auto acceptorPtr  = sessionPtr->getServer()->getAcceptor();
	if (acceptorPtr) {
		LOG(DEBUG) << "CLI: Opening socket (id=" << this << ")...";

		// registering callback on open event
		acceptorPtr->async_accept(
			*nativeSocketPtr.get(),
			callback
		);

		LOG(DEBUG) << "CLI: Socket was opened for incoming requests (id=" << this << ")";
	} else {
		LOG(DEBUG) << "CLI: Could not open socket due to unset acceptor (id=" << this << ")";
	}
}


void Socket::receive(char* data, const std::size_t size, std::function<void(const std::error_code& error, std::size_t bytes_transferred)> callback) {

	// no need to check if socket is open here as callback must receive an error if any
	nativeSocketPtr->async_read_some(
		asio::buffer(data, size),
		callback
	);
}


void Socket::send(const char* string) {
	send(string, strlen(string));
}


void Socket::send(const char* data, const std::size_t size) {

	// writting to a socket
	if (nativeSocketPtr->is_open()) {
		nativeSocketPtr->send(asio::buffer(data, size));
	}
}


void Socket::send(std::shared_ptr<std::string> stringPtr) {
	send(stringPtr->c_str(), stringPtr->size());
}


void Socket::sendEndOfLine() {
	send("\n\r");
}
