#ifndef Command_INCLUDED
#define Command_INCLUDED

#include <functional>
#include <string>


// forward declaration
class Session;


class Command
{
	friend Session;

	public:
		explicit                     Command(Session*, std::string, std::string, std::string, std::function<void(Command*)>, bool);
		virtual                     ~Command();
		const std::function<void()>& getCancelHandler();
		const std::string&           getCommand();
		const std::function<void()>& getHandler();
		const std::string&           getPrefix();
		Session*                     getSession();
		std::size_t                  getSize();
		const std::string&           getSuffix();
		bool                         isAsync();
		void                         setCancelHandler(std::function<void()>);

	protected:
		void handleAction();
		void handleCancel();
		void handleClose();
		void handleInvalid();
		void handleNone();

	private:
        Session*              sessionPtr;
		std::string           prefix;
		std::string           command;
		std::string           suffix;
		bool                  async;
		std::function<void()> handler;
        std::function<void()> cancelHandler;

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
