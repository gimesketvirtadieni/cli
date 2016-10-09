#include <boost/exception/all.hpp>
#include <boost/throw_exception.hpp>
#include <cli/Server.h>
#include <cli/Session.h>
#include <cli/Socket.h>
#include <log/log.h>

#include <iostream>


Socket::Socket(Session* sessionPtr, boost::asio::io_service* dispatcherPtr) :
	sessionPtr(sessionPtr) {

	// creating native socket
	nativeSocketPtr.reset(
		new boost::asio::ip::tcp::socket(
			*dispatcherPtr
		),

		// using empty deletor as socket object will be deleted explicitly by destructor
		[](boost::asio::ip::tcp::socket*) {}
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


void Socket::cancel() {

	// saving socket object in local variable
	auto socketPtr = nativeSocketPtr.get();

	// detaching all weak pointers from the socket object
	nativeSocketPtr.reset(
		socketPtr,

		// using empty deletor as socket object will be deleted by destructor
		[](boost::asio::ip::tcp::socket*) {}
	);

	LOG(DEBUG) << "CLI: Current socket operations were canceled";
}


void Socket::close() {

	if (nativeSocketPtr->is_open()) {
		LOG(DEBUG) << "CLI: Closing socket (id=" << this << ")...";

		try {
			nativeSocketPtr->shutdown(boost::asio::socket_base::shutdown_both);
		} catch (...) {}
		try {
			nativeSocketPtr->close();
		} catch (...) {}

		LOG(DEBUG) << "CLI: Socket was closed (id=" << this << ")";
	}
}


Session* Socket::getSession() {
	return sessionPtr;
}


void Socket::open(std::function<void(const boost::system::error_code&)> callback) {
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


void Socket::receive(char* data, const std::size_t size, std::function<void(const boost::system::error_code& error, std::size_t bytes_transferred)> callback) {

	// reading from socket
	if (nativeSocketPtr && nativeSocketPtr->is_open()) try {
		nativeSocketPtr->async_read_some(
			boost::asio::buffer(data, size),
			callback
		);
	} catch (...) {
		std::cout << "Socket::receiveAsync Error: unexpected exception" << std::endl << std::flush;
	}
}


void Socket::send(const char* string) {
	send(nativeSocketPtr, string, strlen(string));
}


void Socket::send(const char* data, const std::size_t size) {
	send(nativeSocketPtr, data, size);
}


void Socket::send(std::shared_ptr<std::string> stringPtr) {
	send(nativeSocketPtr, stringPtr->c_str(), stringPtr->size());
}


void Socket::send(std::weak_ptr<boost::asio::ip::tcp::socket> nativeSocketWeakPtr, const char* data, const std::size_t size) {

	// writting to a socket
	auto nativeSocketPtr = nativeSocketWeakPtr.lock();
	if (nativeSocketPtr && nativeSocketPtr->is_open()) try {
		nativeSocketPtr->send(boost::asio::buffer(data, size));
	} catch (...) {
		std::cout << "Socket::send Error: unexpected exception" << std::endl << std::flush;
	}
}
