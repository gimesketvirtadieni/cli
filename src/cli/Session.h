#ifndef Session_INCLUDED
#define Session_INCLUDED

#include <asio.hpp>
#include <cli/SessionBase.hpp>
#include <conwrap/ProcessorQueue.hpp>
#include <functional>
#include <log/log.h>
#include <memory>
#include <vector>


// forward declaration
class Server;
class Socket;


class Session : public SessionBase<Session, Socket> {
	public:
		explicit Session(Server*);
		virtual ~Session();

	protected:
		virtual std::unique_ptr<Command> createCommand(const char*, std::size_t, std::size_t);
};

#endif // Session_INCLUDED
