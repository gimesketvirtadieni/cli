#include <cli/Actions.h>
#include <cli/Command.h>
#include <cli/Server.h>
#include <cli/Session.h>
#include <cli/Socket.h>
#include <log/log.h>

#include <chrono>
#include <thread>
//#include <iostream>


Server::Server(unsigned int port, unsigned int maxSessions, g3::LogWorker* logWorkerPtr, std::unique_ptr<Actions> actionsPtr) :
	port(port),
	maxSessions(maxSessions),
	logWorkerPtr(logWorkerPtr),
	actionsPtr(std::move(actionsPtr)),
	stopping(false) {
	LOG(DEBUG) << "CLI: Server object was created (id=" << this << ")";
}


Server::~Server() {

	// deleting all sessions which will flush and delete processors used by sessions
	sessions.clear();

	LOG(DEBUG) << "CLI: Server object was deleted (id=" << this << ")";
}


std::unique_ptr<conwrap::ProcessorQueue<Session>> Server::createSession() {

	// creating session object per connection
	auto sessionProcessorPtr = std::make_unique<conwrap::ProcessorQueue<Session>>(this);

	// setting onOpen event handler
	sessionProcessorPtr->getResource()->setOpenCallback(
		[this](Session* sessionPtr) {
			LOG(DEBUG) << "CLI: Open session callback started (id=" << sessionPtr << ", stopping=" << stopping << ", sessions=" << sessions.size() << ")";

			// no need to do anything while stopping the server
			if (!stopping) {

				// registering a new session if capacity allows so new requests can be accepted
				if (sessions.size() < maxSessions) {
					sessions.push_back(createSession());
					LOG(DEBUG) << "CLI: Session was added (id=" << this << ", sessions=" << sessions.size() << ")";
				} else {
					stopAcceptor();
				}
			}

			LOG(DEBUG) << "CLI: Open session callback completed (id=" << sessionPtr << ", stopping=" << stopping << ", sessions=" << sessions.size() << ")";
		}
	);

	// setting onClose event handler
	sessionProcessorPtr->getResource()->setCloseCallback(
		[this](Session* sessionPtr, const boost::system::error_code& error) {
			LOG(DEBUG) << "CLI: Close session callback started (id=" << sessionPtr << ", stopping=" << stopping << ", sessions=" << sessions.size() << ")";

			// remove session routing is different while stopping the server
			if (!stopping) {

				// posponing removing session from the vector
				processorPtr->process(
					[this, sessionPtr] {

						// by the time this handler is processed session might be deleted however sessionPtr is good enough for comparison
						sessions.erase(
							std::remove_if(
								sessions.begin(),
								sessions.end(),
								[&](auto& savedSessionPtr) -> bool {
									return sessionPtr == savedSessionPtr->getResource();
								}
							),
							sessions.end()
						);
						LOG(DEBUG) << "CLI: Session was removed (id=" << this << ", sessions=" << sessions.size() << ")";

						// starting acceptor if required
						if (!acceptorPtr) {
							startAcceptor();
						}

						// start accepting new requests if required
						if (sessions.size() < maxSessions) {
							sessions.push_back(createSession());
						}
					}
				);
			}

			LOG(DEBUG) << "CLI: Close session callback completed (id=" << sessionPtr << ", stopping=" << stopping << ", sessions=" << sessions.size() << ")";
		}
	);

	// start listening to the incoming requests
	processorPtr->process(
		[this, sessionPtr = sessionProcessorPtr->getResource()] {
			sessionPtr->open();
		}
	);

	LOG(DEBUG) << "CLI: New session was created (id=" << sessionProcessorPtr->getResource() << ")";
	return std::move(sessionProcessorPtr);
}


boost::asio::ip::tcp::acceptor* Server::getAcceptor() {
	return acceptorPtr.get();
}


//Processor<boost::asio::ip::tcp::acceptor>* Server::getAcceptorFacade() {
//	return nullptr;
//}


Actions* Server::getActions() const {
	return actionsPtr.get();
}


g3::LogWorker* Server::getLogger() {
	return logWorkerPtr;
}


conwrap::ProcessorAsio<Server>* Server::getProcessor() {
	return processorPtr;
}


std::shared_ptr<std::string> Server::getPromptMessage() {
	return std::make_shared<std::string>("prompt>");
}


void Server::setProcessor(conwrap::ProcessorAsio<Server>* p)
{
	processorPtr = p;
}


void Server::start() {
	LOG(INFO) << "CLI: Starting new server (id=" << this << ", port=" << port << ", max sessions=" << maxSessions << ")...";

	// resetting flag
	stopping = false;

	// start accepting new sessions
	startAcceptor();

	// creating new session used for accepting connections
	sessions.push_back(createSession());

	LOG(INFO) << "CLI: Server was started (id=" << this << ")";
}


void Server::startAcceptor() {
	LOG(DEBUG) << "CLI: Starting acceptor...";

	// creating an acceptor
	acceptorPtr = std::make_unique<boost::asio::ip::tcp::acceptor>(
		*processorPtr->getDispatcher(),
		boost::asio::ip::tcp::endpoint(
			boost::asio::ip::tcp::v4(),
			port
		)
	);

	LOG(DEBUG) << "CLI: Acceptor was started (id=" << acceptorPtr.get() << ")";
}


void Server::stop() {
	LOG(INFO) << "CLI: Stopping server...";

	// setting flag required for clearn up routine
	stopping = true;

	// stop accepting any new connections
	stopAcceptor();

	// closing active sessions; sessions will be deleted by default Session's destructor
	for (auto& sessionProcessorPtr : sessions) {
		sessionProcessorPtr->getResource()->close();
	}

	std::chrono::milliseconds wait{1000};
	std::this_thread::sleep_for(wait);

	LOG(INFO) << "CLI: Server was stopped (id=" << this << ")";
}


void Server::stopAcceptor() {

	// disposing acceptor to prevent from new incomming requests
	if (acceptorPtr) {
		LOG(DEBUG) << "CLI: Stopping acceptor (id=" << acceptorPtr.get() << ")...";

		acceptorPtr->cancel();
		acceptorPtr->close();

		LOG(DEBUG) << "CLI: Acceptor was stopped (id=" << acceptorPtr.get() << ")";
		acceptorPtr.reset();
	}
}