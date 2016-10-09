#ifndef Server_INCLUDED
#define Server_INCLUDED

#include <boost/asio.hpp>
#include <conwrap/ProcessorQueue.hpp>
#include <conwrap/ProcessorAsio.hpp>
#include <functional>
#include <g3log/logworker.hpp>
#include <memory>
#include <vector>


// forward declaration
class Actions;
class Session;
class Socket;


class Server {
	public:
		                                Server(unsigned int, unsigned int, g3::LogWorker*, std::unique_ptr<Actions>);
		                               ~Server();
		boost::asio::ip::tcp::acceptor* getAcceptor();
		//Processor<boost::asio::ip::tcp::acceptor>* getAcceptorFacade();
		Actions*                        getActions() const;
		g3::LogWorker*                  getLogger();
	    std::shared_ptr<std::string>    getPromptMessage();
	    conwrap::ProcessorAsio<Server>* getProcessor();
	    void                            setProcessor(conwrap::ProcessorAsio<Server>*);
		void                            start();
		void                            stop();

	protected:
		std::unique_ptr<conwrap::ProcessorQueue<Session>> createSession();
		void                                              startAcceptor();
		void                                              stopAcceptor();

	private:
		conwrap::ProcessorAsio<Server>*                                processorPtr;
		unsigned int                                                   port;
		unsigned int                                                   maxSessions;
		g3::LogWorker*                                                 logWorkerPtr;
		std::unique_ptr<Actions>                                       actionsPtr;
		std::unique_ptr<boost::asio::ip::tcp::acceptor>                acceptorPtr;
		std::vector<std::unique_ptr<conwrap::ProcessorQueue<Session>>> sessions;
		bool                                                           stopping;
};

#endif // Server_INCLUDED
