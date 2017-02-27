#ifndef SessionBase_INCLUDED
#define SessionBase_INCLUDED

#include <asio.hpp>
#include <cli/Command.h>
#include <cli/Context.h>
#include <cli/Messages.h>
#include <cli/Server.h>
#include <conwrap/ProcessorQueue.hpp>
#include <functional>
#include <log/log.h>
#include <memory>
#include <vector>


template <typename SessionType, typename SocketType>
class SessionBase {
	public:
		explicit SessionBase(Server* se, std::unique_ptr<SocketType> so)
		: serverPtr(se)
		, socketPtr(std::move(so))
		, bufferUsed(0)
		, promptWasSent(false) {}


		inline void cancel()
		{
			// required to display prompt properly
			promptWasSent = false;

			// actual cancel routine is done by onCancel hook
			onCancel();
		}


		inline void close(const std::error_code error = std::error_code())
		{
			// echoing logout message back to the client
			getSocket()->send(cli::Messages::logout);
			getSocket()->send(cli::Messages::endOfLine);

			// closing the socket; there is no need to invoke onClose here as it will be done by onOpen / onData methods
			getSocket()->close();
		}


		conwrap::ProcessorQueue<SessionType>* getProcessor()
		{
	    	return processorPtr;
	    }


		inline Server* getServer()
		{
			return serverPtr;
		}


		inline SocketType* getSocket()
		{
			return socketPtr.get();
		}


		inline void open()
		{
			// opening a socket; onOpen will be called once socket receives incomming connection
			getSocket()->open([=](const std::error_code code)
			{
				// this callback will be invoked on Asio's thread, so it must be passed to the server's thread
				serverPtr->getProcessorProxy()->wrap([=]
				{
					onOpen(code);
				})();
			});
		}


		void process(std::unique_ptr<Command> commandPtr) {

			// defining command handler lambda
			auto commandHandle = [this, commandPtr = commandPtr.get()] {
				LOG(DEBUG) << LABELS{"cli"} << "Starting command action (id=" << commandPtr << ", async=" << commandPtr->isAsync() << ")...";

				// executing command action
				if (auto handler = commandPtr->getHandler()) {
					handler();
				}

				LOG(DEBUG) << LABELS{"cli"} << "Command action completed (id=" << commandPtr << ")";
			};

			// defining lambda to delete command from the list
			auto commandDelete = [this, commandPtr = commandPtr.get()] {
				commands.erase(
					std::remove_if(
						commands.begin(),
						commands.end(),
						[&](auto& savedCommandPtr) -> bool {
							return commandPtr == savedCommandPtr.get();
						}
					),
					commands.end()
				);
				LOG(DEBUG) << LABELS{"cli"} << "Command was deleted from the list (id=" << commandPtr << ", commands=" << commands.size() << ")";
			};

			// keeping command id and async flag locally
			auto commandRawPtr = commandPtr.get();
			auto async         = commandPtr->isAsync();

			// saving command in the vector so it can be canceled
			commands.push_back(std::move(commandPtr));
			LOG(DEBUG) << LABELS{"cli"} << "Command was added to the list (id=" << commandRawPtr << ", commands=" << commands.size() << ")";

			// if command is async then executing action on the session's thread
			if (async) {

				// setting a flag to be used by cancel handler
				promptWasSent = false;

				// processing command on the session's thread and deleteing from the vector on the server's thread
				getProcessor()->process(commandHandle).then([this, commandDelete] {
					serverPtr->getProcessorProxy()->process(commandDelete);
				});
			} else {

				// processing command and deleting it from the list synchroniously on the server's thread
				commandHandle();
				commandDelete();
			}
		}


		void sendPrompt(bool sendEOL)
		{
			if (!promptWasSent) {
				if (sendEOL) {
					getSocket()->send(cli::Messages::endOfLine);
				}
				getSocket()->send(getServer()->getPromptMessage());

				promptWasSent = true;
			}
		}


		inline void setCloseCallback(std::function<void (SessionType*, const std::error_code&)> closeCallback) {
		    this->closeCallback = closeCallback;
		}


		inline void setDataCallback(std::function<void (SessionType*, const std::error_code&, const std::size_t&)> dataCallback) {
		    this->dataCallback = dataCallback;
		}


		inline void setOpenCallback(std::function<void (SessionType*)> openCallback) {
		    this->openCallback = openCallback;
		}


		void setProcessor(conwrap::ProcessorQueue<SessionType>* p)
	    {
	    	processorPtr = p;
	    }


	protected:
		virtual std::unique_ptr<Command> createCommand(const char*, std::size_t, std::size_t) = 0;


		void negotiate() {
			getSocket()->send(cli::Messages::negotiateCommands);
		}


		void onCancel() {
			LOG(DEBUG) << LABELS{"cli"} << "Canceling session (id=" << this << ")";

			// iterating through running commands
			for(auto& commandPtr : commands) {

				// obtaining and evaluating cancel handler
				if (auto cancelHandler = commandPtr->getCancelHandler()) {
					LOG(DEBUG) << LABELS{"cli"} << "Canceling command (id=" << commandPtr.get() << ")...";

					// invoking cancel handler
					cancelHandler();

					LOG(DEBUG) << LABELS{"cli"} << "Command was canceled (id=" << commandPtr.get() << ")";
				}
			}

			// waiting for any running action to complete
			processorPtr->flush();

			LOG(DEBUG) << LABELS{"cli"} << "Session was canceled (id=" << this << ")";
		}


		void onClose(const std::error_code error) {
			LOG(DEBUG) << LABELS{"cli"} << "Closing session (id=" << this << ", error='" << error.message() << "')...";

			// canceling any actions run by this session
			cancel();

			// invoking close callback before session is desposed
			if (closeCallback) {
				closeCallback(processorPtr->getResource(), error);
		    }

			LOG(DEBUG) << LABELS{"cli"} << "Session was closed (id=" << this << ")";
		}


		void onData(const std::error_code error, const std::size_t receivedSize) {
			std::unique_ptr<Command> commandPtr;
			std::size_t              commandSize;
			std::size_t              echoSize = receivedSize;

			// if error then closing this session
			if (error) {
				onClose(error);
			} else {

				// invoking onData callback
				if (dataCallback) {
					dataCallback(processorPtr->getResource(), error, receivedSize);
			    }

				if (receivedSize > 0) {

					// moving buffer position pointer by the number of bytes received
					bufferUsed = bufferUsed + receivedSize;
					if (bufferUsed > sizeof(buffer)) {

						// TODO: handling buffer overflow
					}

					while ((commandPtr = createCommand(buffer, sizeof(buffer), bufferUsed)).get()) {

						// figuring out how much data should be echoed
						commandSize = commandPtr->getSize();
						if (commandSize + echoSize > bufferUsed) {
							echoSize = bufferUsed - commandSize;
						}

						// removing data from the buffer leaving everything after the command
						if (commandSize < BUFFER_SIZE) {
							memcpy(buffer, buffer + commandSize, BUFFER_SIZE - commandSize);
							bufferUsed -= commandSize;
						} else {
							bufferUsed = 0;
						}

						// processing command
						process(std::move(commandPtr));
					}

					// echoing everything after the command
					if (echoSize > 0) {
						getSocket()->send(buffer + bufferUsed - echoSize, echoSize);
					}
				}

				// submitting a new task here allows other tasks to progress
				getServer()->getProcessorProxy()->process([=]
				{
					// keep receiving data 'recursivelly' except task processor is used instead of stack
					getSocket()->receive(
						buffer + bufferUsed,
						sizeof(buffer) - bufferUsed,
						[=](const std::error_code error, std::size_t bytes_transferred)
						{
							serverPtr->getProcessorProxy()->wrap([=]
							{
								onData(error, bytes_transferred);
							})();
						}
					);
				});
			}
		}


		void onOpen(const std::error_code error) {

			if (error) {
				onClose(error);
			} else {
				LOG(DEBUG) << LABELS{"cli"} << "Opening session (id=" << this << ")...";

				// invoking open callback
				if (openCallback) {
					openCallback(processorPtr->getResource());
			    }

				// setting parameters for the telnet session
				negotiate();

				// displaying prompt message
				getSocket()->send(getServer()->getPromptMessage());

				// start receiving data
				onData(error, 0);

				LOG(DEBUG) << LABELS{"cli"} << "Session was opened (id=" << this << ")";
		    }
		}


	private:
		conwrap::ProcessorQueue<SessionType>*                                          processorPtr;
		Server*                                                                        serverPtr;
		std::unique_ptr<SocketType>                                                    socketPtr;
        std::function<void (SessionType*, const std::error_code&)>                     closeCallback;
        std::function<void (SessionType*, const std::error_code&, const std::size_t&)> dataCallback;
        std::function<void (SessionType*)>                                             openCallback;
		std::vector<std::unique_ptr<Command>>                                          commands;
    	bool                                                                           promptWasSent;

		// TODO: ...
		enum {
			BUFFER_SIZE = 1024
		};
		char         buffer[BUFFER_SIZE];
        unsigned int bufferUsed;
};

#endif // SessionBase_INCLUDED
