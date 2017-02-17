#include <cli/Actions.h>
#include <cli/Command.h>
#include <cli/Server.h>
#include <cli/Session.h>
#include <cli/Socket.h>
#include <log/log.h>


Server::Server(unsigned int p, unsigned int m, g3::LogWorker* l)
	: port(p)
	, maxSessions(m)
	, logWorkerPtr(l)
	, stopping(false)
{
	LOG(DEBUG) << g3::Labels{"cli"} << "Server object was created (id=" << this << ")";
}


Server::~Server()
{
	// deleting all sessions which will flush and delete processors used by sessions
	sessions.clear();

	LOG(DEBUG) << g3::Labels{"cli"} << "Server object was deleted (id=" << this << ")";
}


std::unique_ptr<conwrap::ProcessorQueue<Session>> Server::createSession()
{
	// creating session object per connection
	auto processorSessionPtr = std::make_unique<conwrap::ProcessorQueue<Session>>(this);

	// setting onOpen event handler
	processorSessionPtr->getResource()->setOpenCallback([this](Session* sessionPtr)
	{
		LOG(DEBUG) << g3::Labels{"cli"} << "Open session callback started (id=" << sessionPtr << ", stopping=" << stopping << ", sessions=" << sessions.size() << ")";

		// registering a new session if capacity allows so new requests can be accepted
		if (sessions.size() < maxSessions)
		{
			sessions.push_back(std::move(createSession()));
			LOG(DEBUG) << g3::Labels{"cli"} << "Session was added (id=" << this << ", sessions=" << sessions.size() << ")";
		} else {
			LOG(DEBUG) << g3::Labels{"cli"} << "Limit of active sessions was reached (id=" << this << ", sessions=" << sessions.size() << " max=" << maxSessions << ")";
			stopAcceptor();
		}

		LOG(DEBUG) << g3::Labels{"cli"} << "Open session callback completed (id=" << sessionPtr << ", stopping=" << stopping << ", sessions=" << sessions.size() << ")";
	});

	// setting onClose event handler
	processorSessionPtr->getResource()->setCloseCallback([this](Session* sessionPtr, const std::error_code& error)
	{
		LOG(DEBUG) << g3::Labels{"cli"} << "Close session callback started (id=" << sessionPtr << ", stopping=" << stopping << ", sessions=" << sessions.size() << ")";

		// session cannot be deleted at this moment as this method is called withing this session
		processorProxyPtr->process([this, sessionPtr]
		{
			// by the time this handler is processed, session might be deleted however sessionPtr is good enough for comparison
			deleteSession(sessionPtr);

			// starting acceptor if required
			if (!acceptorPtr) {
				startAcceptor();
			}
		});

		LOG(DEBUG) << g3::Labels{"cli"} << "Close session callback completed (id=" << sessionPtr << ", stopping=" << stopping << ", sessions=" << sessions.size() << ")";
	});

	// start listening to the incoming requests
	processorSessionPtr->getResource()->open();

	LOG(DEBUG) << g3::Labels{"cli"} << "New session was created (id=" << processorSessionPtr->getResource() << ")";
	return std::move(processorSessionPtr);
}


void Server::deleteSession(Session* sessionPtr)
{
	// removing session from the vector
	sessions.erase(
		std::remove_if(
			sessions.begin(),
			sessions.end(),
			[&](auto& processorSessionPtr) -> bool
			{
				return sessionPtr == processorSessionPtr->getResource();
			}
		),
		sessions.end()
	);
	LOG(DEBUG) << g3::Labels{"cli"} << "Session was removed (id=" << this << ", sessions=" << sessions.size() << ")";
}


asio::ip::tcp::acceptor* Server::getAcceptor()
{
	return acceptorPtr.get();
}


Actions* Server::getActions()
{
	return &actions;
}


g3::LogWorker* Server::getLogger()
{
	return logWorkerPtr;
}


conwrap::ProcessorAsioProxy<Server>* Server::getProcessorProxy()
{
	return processorProxyPtr;
}


std::shared_ptr<std::string> Server::getPromptMessage()
{
	return std::make_shared<std::string>("prompt>");
}


void Server::setProcessorProxy(conwrap::ProcessorAsioProxy<Server>* p)
{
	processorProxyPtr = p;
}


void Server::start()
{
	LOG(INFO) << g3::Labels{"cli"} << "Starting new server (id=" << this << ", port=" << port << ", max sessions=" << maxSessions << ")...";

	// setting the stopping flag
	stopping = false;

	// start accepting new sessions
	startAcceptor();

	// creating new session used for accepting connections
	sessions.push_back(std::move(createSession()));

	LOG(INFO) << g3::Labels{"cli"} << "Server was started (id=" << this << ")";
}


void Server::startAcceptor()
{
	// creating an acceptor if required
	if (!acceptorPtr && !stopping)
	{
		LOG(DEBUG) << g3::Labels{"cli"} << "Starting acceptor...";
		acceptorPtr = std::make_unique<asio::ip::tcp::acceptor>(
			*processorProxyPtr->getDispatcher(),
			asio::ip::tcp::endpoint(
				asio::ip::tcp::v4(),
				port
			)
		);
		LOG(DEBUG) << g3::Labels{"cli"} << "Acceptor was started (id=" << acceptorPtr.get() << ")";
	}
}


void Server::stop()
{
	LOG(INFO) << g3::Labels{"cli"} << "Stopping server...";

	// setting flag required for clearn up routine
	stopping = true;

	// stop accepting any new connections
	stopAcceptor();

	// closing active sessions; sessions will be deleted by default Session's destructor
	for (auto& sessionProcessorPtr : sessions)
	{
		sessionProcessorPtr->getResource()->close();
	}

	LOG(INFO) << g3::Labels{"cli"} << "Server was stopped (id=" << this << ")";
}


void Server::stopAcceptor() {

	// disposing acceptor to prevent from new incomming requests
	if (acceptorPtr) {
		LOG(DEBUG) << g3::Labels{"cli"} << "Stopping acceptor (id=" << acceptorPtr.get() << ")...";

		acceptorPtr->cancel();
		acceptorPtr->close();

		LOG(DEBUG) << g3::Labels{"cli"} << "Acceptor was stopped (id=" << acceptorPtr.get() << ")";
		acceptorPtr.reset();
	}
}
