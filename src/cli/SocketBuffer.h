#ifndef SocketBuffer_INCLUDED
#define SocketBuffer_INCLUDED

#include <cli/Socket.h>
#include <iostream>
#include <utility>


class SocketBuffer : public std::streambuf
{
	public:
		            SocketBuffer(Socket*);
		            SocketBuffer(SocketBuffer&&);
		virtual    ~SocketBuffer();
		virtual int overflow(int c);
		virtual int sync();

	private:
		std::stringstream buffer;
		Socket*           socketPtr;
};

#endif // SocketBuffer_INCLUDED
