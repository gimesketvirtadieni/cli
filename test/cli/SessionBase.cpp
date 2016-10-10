#include <chrono>
#include <cli/Actions.h>
#include <cli/Messages.h>
#include <cli/Server.h>
#include <cli/SessionBase.hpp>
#include <conwrap/ProcessorAsio.hpp>
#include <conwrap/ProcessorQueue.hpp>
#include <g3log/logworker.hpp>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <functional>

#include <iostream>


struct SocketMock {
	conwrap::ProcessorAsio<Server>* processorServerPtr;
	std::vector<std::string>        sentMessages;

	SocketMock() {}

	SocketMock(conwrap::ProcessorAsio<Server>* processorServerPtr) : processorServerPtr(processorServerPtr) {}

	virtual ~SocketMock() {}

	void open(std::function<void(const std::error_code&)> openHandler) {

		processorServerPtr->process(
			[this, openHandler] {

				// this delay simulates that session open is an async event
				std::chrono::milliseconds wait{100};
				std::this_thread::sleep_for(wait);

				// invoking an empty hook so it can be mocked
				this->openHook();

				openHandler(std::error_code());
			}
		);
	}
	MOCK_METHOD0(openHook, void());

	void receive(char*, const std::size_t, std::function<void(const std::error_code&, std::size_t)>) {
	}

	void send(const char* str) {
		sentMessages.push_back(std::string(str));
	}

	void send(const char* buffer, const std::size_t size) {
		//sentMessages.push_back(std::string(str));
	}

	void send(const std::shared_ptr<std::string> str) {
		sentMessages.push_back(std::string(*str));
	}
};


struct SessionBaseMock : public SessionBase<SessionBaseMock, SocketMock> {
	SessionBaseMock(Server* serverPtr, std::unique_ptr<SocketMock> socketPtr) :
		SessionBase(serverPtr, std::move(socketPtr)) {}

	virtual ~SessionBaseMock() {}

	virtual std::unique_ptr<Command> createCommand(const char*, std::size_t, std::size_t) {
		return std::unique_ptr<Command>();
	}
/*
	void onData(const boost::system::error_code error, std::size_t receivedSize) {

		// invoking onData callback
		//runDataCallback(error, receivedSize);
	}
*/
	virtual void process(std::unique_ptr<Command>) {};
/*
	virtual void runCloseCallback(const boost::system::error_code error) {

		// invoking an empty hook so it can be mocked
		runCloseCallbackHook();
	}
	MOCK_METHOD0(runCloseCallbackHook, void());

*/
	void dataCallback(SessionBaseMock*, const std::error_code error, std::size_t receivedSize) {

		// invoking an empty hook so it can be mocked
		dataCallbackHook();
	}
	MOCK_METHOD0(dataCallbackHook, void());

	void openCallback(SessionBaseMock*) {

		// invoking an empty hook so it can be mocked
		openCallbackHook();
	}
	MOCK_METHOD0(openCallbackHook, void());
};


TEST(SessionBase, Constructor1) {

	conwrap::ProcessorAsio<Server> processorServer(
		1234,
		1,
		nullptr,
		std::move(std::make_unique<Actions>())
	);

	auto socketPtr    = std::make_unique<SocketMock>();
	auto socketRawPtr = socketPtr.get();
	conwrap::ProcessorQueue<SessionBaseMock> processorSessionMock(
		processorServer.getResource(),
		std::move(socketPtr)
	);

	ASSERT_TRUE(processorSessionMock.getResource() != nullptr);
	ASSERT_EQ(processorSessionMock.getResource()->getServer(), processorServer.getResource());
	ASSERT_EQ(processorSessionMock.getResource()->getSocket(), socketRawPtr);
}


TEST(SessionBase, Open1) {

	// creating server instance
	conwrap::ProcessorAsio<Server> processorServer(
		1234,
		1,
		nullptr,
		std::move(std::make_unique<Actions>())
	);

	// creating session instance
	auto socketPtr    = std::make_unique<SocketMock>(&processorServer);
	auto socketRawPtr = socketPtr.get();
	conwrap::ProcessorQueue<SessionBaseMock> processorSessionMock(
		processorServer.getResource(),
		std::move(socketPtr)
	);

	processorSessionMock.getResource()->setDataCallback(
		std::bind(
			&SessionBaseMock::dataCallback,
			processorSessionMock.getResource(),
			std::placeholders::_1,
			std::placeholders::_2,
			std::placeholders::_3
		)
	);

	processorSessionMock.getResource()->setOpenCallback(
		std::bind(
			&SessionBaseMock::openCallback,
			processorSessionMock.getResource(),
			std::placeholders::_1
		)
	);

	// defining expectations
	testing::Sequence s;
	EXPECT_CALL(*socketRawPtr, openHook())
		.Times(1)
		.InSequence(s);
	EXPECT_CALL(*processorSessionMock.getResource(), openCallbackHook())
		.Times(1)
		.InSequence(s);
	EXPECT_CALL(*processorSessionMock.getResource(), dataCallbackHook())
		.Times(1)
		.InSequence(s);

	// invoking open method from CLI server processot thread
	processorServer.process(
		[&] {
			processorSessionMock.getResource()->open();
		}
	);

	// flushing server's processor which will complete all handlers
	processorServer.flush();

	// TODO: parameters should be tested; currently only execution flow and socket output is tested

	// testing if correct data was sent to the socket
	ASSERT_EQ(socketRawPtr->sentMessages.size(), (unsigned int) 2);
	if (socketRawPtr->sentMessages.size() >= 2) {
		ASSERT_EQ(socketRawPtr->sentMessages[0], CLI::Messages::negotiateCommands);
		ASSERT_EQ(socketRawPtr->sentMessages[1], *processorServer.getResource()->getPromptMessage());
	}
}
