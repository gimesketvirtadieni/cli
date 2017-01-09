#include <cli/SocketBuffer.h>
#include <log/log.h>


SocketBuffer::SocketBuffer(Socket* s) : socketPtr(s) {}


SocketBuffer::~SocketBuffer()
{
	sync();
}


int SocketBuffer::overflow(int c)
{
	if (c != EOF)
	{
		buffer << static_cast<char>(c);
		if (c == '\n')
		{
			sync();
		}
	}
	else
	{
		sync();
	}

	return c;
}


int SocketBuffer::sync()
{
	if (buffer.str().length())
	{
		socketPtr->send(buffer.str().c_str());
	}
	buffer.str("");
	buffer.clear();

	return 0;
}
