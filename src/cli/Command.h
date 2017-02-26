#ifndef Command_INCLUDED
#define Command_INCLUDED

#include <cli/Context.h>
#include <functional>
#include <future>
#include <memory>
#include <string>
#include <vector>


// forward declaration
class Session;


class Command {
	public:
		explicit                              Command(Session*, std::shared_ptr<std::string>, std::shared_ptr<std::string>, std::shared_ptr<std::string>, std::function<void(Command*)>, bool);
		virtual                              ~Command();
		         std::function<void()>        getCancelHandler();
		         std::shared_ptr<std::string> getCommand();
		         std::function<void()>        getHandler();
		         std::shared_ptr<std::string> getPrefix();
		         Session*                     getSession();
		         std::size_t                  getSize();
		         std::shared_ptr<std::string> getSuffix();
		         bool                         isAsync();
		         void                         setCancelHandler(std::function<void()>);
		static   std::unique_ptr<Command>     createCommand(Session*, const char*, std::size_t, std::size_t);
		static   std::unique_ptr<Command>     createTelnetCommand(Session*, const char*, std::size_t, std::size_t, std::size_t);

	protected:
		void handleAction();
		void handleCancel();
		void handleClose();
		void handleInvalid();
		void handleNone();

	private:
        Session*                     sessionPtr;
		std::shared_ptr<std::string> prefixPtr;
		std::shared_ptr<std::string> commandPtr;
		std::shared_ptr<std::string> suffixPtr;
		bool                         async;
		std::function<void()>        handler;
        std::function<void()>        cancelHandler;

	public:
        class Builder
		{
			public:
				explicit                 Builder(Session*);
				std::unique_ptr<Command> build();
				Builder&                 setAsync(bool);
				Builder&                 setCommand(std::string);
				Builder&                 setHandler(std::function<void(Command*)>);
				Builder&                 setPrefix(std::string);
				Builder&                 setSuffix(std::string);

			private:
		        Session*                      sessionPtr;
				std::function<void(Command*)> handler;
				bool                          async;
				std::string                   prefix;
				std::string                   command;
				std::string                   suffix;
		};
};

#endif // Command_INCLUDED
