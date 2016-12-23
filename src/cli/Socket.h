#ifndef Socket_INCLUDED
#define Socket_INCLUDED

#include <asio.hpp>
#include <functional>
#include <memory>


// forward declaration
class Session;


class Socket {
	public:
		         Socket(Session*, asio::io_service*);
		        ~Socket();
		void     cancel();
		void     close();
		Session* getSession();
		void     open(std::function<void(const std::error_code&)>);
		void     receive(char*, const std::size_t, std::function<void(const std::error_code&, std::size_t)>);
		void     send(const char*);
		void     send(const char*, const std::size_t);
		void     send(const std::shared_ptr<std::string>);
		void     sendEndOfLine();

	protected:
		static void send(std::weak_ptr<asio::ip::tcp::socket>, const char*, const std::size_t);

	private:
		Session*                               sessionPtr;
		std::shared_ptr<asio::ip::tcp::socket> nativeSocketPtr;
};

#endif // Socket_INCLUDED
