#ifndef SocketSink_INCLUDED
#define SocketSink_INCLUDED

#include <functional>
#include <g3log/logmessage.hpp>
#include "SinkFilter.h"


// forward declaration
class Socket;


class SocketSink : public SinkFilter {
	public:
		            SocketSink(Socket*, std::function<bool(g3::LogMessage&)> = NULL);
		            SocketSink(const SocketSink&) = delete;
		virtual    ~SocketSink();
		SocketSink& operator=(const SocketSink&) = delete;
		void        log(const g3::LogMessageMover);

	private:
		Socket* socketPtr;
};


#endif // SocketSink_INCLUDED
