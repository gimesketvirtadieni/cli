#ifndef Mocks_INCLUDED
#define Mocks_INCLUDED

#include <atomic>
#include <asio.hpp>
#include <chrono>
#include <cli/Actions.h>
#include <cli/Messages.h>
#include <cli/Server.h>
#include <cli/SessionBase.hpp>
#include <conwrap/ProcessorAsio.hpp>
#include <conwrap/ProcessorQueue.hpp>
#include <functional>
#include <g3log/logworker.hpp>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>
#include <thread>

#include <system_error>
#include <bits/error_constants.h>
#include <iostream>
#include <log/log.h>


// forward declaration
struct SessionBaseMock;
/*
template <typename ThreadType, typename DispatcherType, typename ResourceType>
class ProcessorImpl;
template <typename ResourceType>
class Processor;


struct LoopSequenceChecker {
	mutable std::string value;

	LoopSequenceChecker(std::string value) : value(value) {}

	void check(std::string currentValue, std::string nextValue) {
		EXPECT_TRUE(value == currentValue);
		value = nextValue;
	}
};


struct ThreadMock {
	mutable std::function<void()>        fun;
	mutable std::unique_ptr<std::thread> backgroundTreadPtr;

	ThreadMock() : fun([]{}) {}

	ThreadMock(std::function<void()> fun) : fun(fun) {}

	virtual ~ThreadMock() {}

	std::thread::id get_id() const noexcept {
		return (backgroundTreadPtr != nullptr ? backgroundTreadPtr->get_id() : std::thread::id());
	}

	void join() {

		if (backgroundTreadPtr) {
			// join hook must be called AFTER this join because it waits for all handlers to complete
			backgroundTreadPtr->join();

			// invoking an empty hook so it can be mocked
			joinHook();
		}
	}
	MOCK_METHOD0(joinHook, void());

	ThreadMock& operator=(ThreadMock&& t) noexcept {
      fun                = t.fun;
      backgroundTreadPtr = std::move(t.backgroundTreadPtr);

      return *this;
    }

	void start() {
		backgroundTreadPtr = std::make_unique<std::thread>(fun);
	}
};


struct DispatcherMock {
	mutable asio::io_service dispatcher;
	mutable bool             enableResetHook = true;
	mutable bool             enableRunHook   = true;

	DispatcherMock() {}

	virtual ~DispatcherMock() {}

	void post(std::function<void()> fun) {
		dispatcher.post(fun);
	}

	void reset() {

		// invoking an empty hook so it can be mocked
		if (enableResetHook) {
			resetHook();
		}

		dispatcher.reset();
	}
	MOCK_METHOD0(resetHook, void());

	bool poll_one() {
		return dispatcher.poll_one();
	}

	void run() {

		// invoking an empty hook so it can be mocked
		if (enableRunHook) {
			runHook();
		}

		dispatcher.run();
	}
	MOCK_METHOD0(runHook, void());

	bool run_one() {

		// invoking an empty hook so it can be mocked
		runOneHook();

		return dispatcher.run_one();
	}
	MOCK_METHOD0(runOneHook, void());

	void stop() {

		// invoking an empty hook so it can be mocked
		stopHook();

		dispatcher.stop();
	}
	MOCK_METHOD0(stopHook, void());
};
*/

struct Dummy {};


struct SocketMock
{
	conwrap::ProcessorAsio<Server>* processorServerPtr;
	SessionBaseMock*                sessionBaseMockPtr;
	std::vector<std::string>        sentMessages;


	SocketMock() {}


	explicit SocketMock(conwrap::ProcessorAsio<Server>* p)
	: processorServerPtr(p) {}


	// implementation is defined after SessionBaseMock declaration
	void close();
	MOCK_METHOD0(closeHook, void());


	void open(std::function<void(const std::error_code&)> openHandler)
	{
		processorServerPtr->process([this, openHandler]
		{
			// this delay simulates that session open is an async event
			std::this_thread::sleep_for(std::chrono::milliseconds{10});

			// invoking an empty hook so it can be mocked
			this->openHook();

			openHandler(std::error_code());
		});
	}
	MOCK_METHOD0(openHook, void());


	void receive(char*, const std::size_t, std::function<void(const std::error_code&, std::size_t)>) {}


	void send(const char* str)
	{
		sentMessages.push_back(std::string(str));
	}


	void send(const char* buffer, const std::size_t size)
	{
		//sentMessages.push_back(std::string(str));
	}


	void send(const std::shared_ptr<std::string> str)
	{
		sentMessages.push_back(std::string(*str));
	}

	void setSession(SessionBaseMock* s)
	{
		sessionBaseMockPtr = s;
	}
};


struct SessionBaseMock : public SessionBase<SessionBaseMock, SocketMock>
{
	explicit SessionBaseMock(conwrap::ProcessorAsio<Server>* processorServerPtr, std::unique_ptr<SocketMock> socketPtr)
	: SessionBase(processorServerPtr->getResource(), std::move(socketPtr)) {}


	virtual std::unique_ptr<Command> createCommand(const char*, std::size_t, std::size_t)
	{
		return std::unique_ptr<Command>();
	}


	// rewriting visibility
	void onClose(const std::error_code error)
	{
		SessionBase::onClose(error);
	}


	// rewriting visibility
	void onOpen(const std::error_code error)
	{
		SessionBase::onOpen(error);
	}


	virtual void process(std::unique_ptr<Command>) {};


	void closeCallback(SessionBaseMock*, const std::error_code error)
	{
		// invoking an empty hook so it can be mocked
		closeCallbackHook();
	}
	MOCK_METHOD0(closeCallbackHook, void());


	void dataCallback(SessionBaseMock*, const std::error_code error, std::size_t receivedSize)
	{
		// invoking an empty hook so it can be mocked
		dataCallbackHook();
	}
	MOCK_METHOD0(dataCallbackHook, void());


	void openCallback(SessionBaseMock*)
	{
		// invoking an empty hook so it can be mocked
		openCallbackHook();
	}
	MOCK_METHOD0(openCallbackHook, void());
};


void SocketMock::close()
{
	processorServerPtr->process([=]
	{
		// this delay simulates that session close is an async event
		std::this_thread::sleep_for(std::chrono::milliseconds{10});

		// to simulate Asio invoking onOpen explicitly
		sessionBaseMockPtr->onOpen(std::make_error_code(std::errc::connection_aborted));
	});

	// invoking an empty hook so it can be mocked
	closeHook();
}

#endif // Mocks_INCLUDED
