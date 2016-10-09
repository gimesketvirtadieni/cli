#ifndef Socket_INCLUDED
#define Socket_INCLUDED

#include <boost/asio.hpp>
#include <functional>
#include <memory>


// forward declaration
class Session;


class Socket {
	public:
		         Socket(Session*, boost::asio::io_service* nativeDispatcherPtr);
		        ~Socket();
		void     cancel();
		void     close();
		Session* getSession();
		void     open(std::function<void(const boost::system::error_code&)>);
		void     receive(char*, const std::size_t, std::function<void(const boost::system::error_code&, std::size_t)>);
		void     send(const char*);
		void     send(const char*, const std::size_t);
		void     send(const std::shared_ptr<std::string>);
		bool     sentEndOfLine();

	protected:
		static void send(std::weak_ptr<boost::asio::ip::tcp::socket>, const char*, const std::size_t);

	private:
		Session*                                      sessionPtr;
		std::shared_ptr<boost::asio::ip::tcp::socket> nativeSocketPtr;
};

#endif // Socket_INCLUDED
