#ifndef Server_INCLUDED
#define Server_INCLUDED

#include <asio.hpp>
#include <cli/Actions.h>
#include <conwrap/ProcessorQueue.hpp>
#include <conwrap/ProcessorAsioProxy.hpp>
#include <functional>
#include <g3log/logworker.hpp>
#include <memory>
#include <vector>


// forward declaration
class Session;
class Socket;


class Server {
	public:
		                                     Server(unsigned int, unsigned int, g3::LogWorker*);
		                                    ~Server();
		asio::ip::tcp::acceptor*             getAcceptor();
		Actions&                             getActions();
		g3::LogWorker*                       getLogger();
	    std::shared_ptr<std::string>         getPromptMessage();
	    conwrap::ProcessorAsioProxy<Server>* getProcessorProxy();
	    void                                 setProcessorProxy(conwrap::ProcessorAsioProxy<Server>*);
		void                                 start();
		void                                 stop();

	protected:
		std::unique_ptr<conwrap::ProcessorQueue<Session>> createSession();
		void                                              deleteSession(Session*);
		void                                              startAcceptor();
		void                                              stopAcceptor();

	private:
		conwrap::ProcessorAsioProxy<Server>*                           processorProxyPtr;
		unsigned int                                                   port;
		unsigned int                                                   maxSessions;
		g3::LogWorker*                                                 logWorkerPtr;
		Actions                                                        actions;
		std::unique_ptr<asio::ip::tcp::acceptor>                       acceptorPtr;
		std::vector<std::unique_ptr<conwrap::ProcessorQueue<Session>>> sessions;
		bool                                                           stopping;
};

#endif // Server_INCLUDED
