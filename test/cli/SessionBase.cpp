#include "Mocks.hpp"

#include <iostream>


TEST(SessionBase, Constructor1) {

	// creating server instance to be used for create a session mock object
	conwrap::ProcessorAsio<Server> processorServer(
		1234,
		1,
		nullptr
	);

	// creating session mock instance to be tested
	auto socketPtr          = std::make_unique<SocketMock>();
	auto socketRawPtr       = socketPtr.get();
	auto sessionBaseMockPtr = std::make_unique<SessionBaseMock>(
		&processorServer,
		std::move(socketPtr)
	);
	socketRawPtr->setSession(sessionBaseMockPtr.get());
	conwrap::ProcessorQueue<SessionBaseMock> processorSessionMock(std::move(sessionBaseMockPtr));

	// testing SessionBase::getResource, SessionBase::getServer and SessionBase::getSocket which are accessed via session mock object
	ASSERT_TRUE(processorSessionMock.getResource() != nullptr);
	ASSERT_EQ(processorSessionMock.getResource()->getServer(), processorServer.getResource());
	ASSERT_EQ(processorSessionMock.getResource()->getSocket(), socketRawPtr);
}


TEST(SessionBase, Open1) {

	// creating server instance to be used for create a session mock object
	conwrap::ProcessorAsio<Server> processorServer(
		1234,
		1,
		nullptr
	);

	// creating session mock instance to be tested
	auto socketPtr          = std::make_unique<SocketMock>(&processorServer);
	auto socketRawPtr       = socketPtr.get();
	auto sessionBaseMockPtr = std::make_unique<SessionBaseMock>(
		&processorServer,
		std::move(socketPtr)
	);
	socketRawPtr->setSession(sessionBaseMockPtr.get());
	conwrap::ProcessorQueue<SessionBaseMock> processorSessionMock(std::move(sessionBaseMockPtr));

	// setting SessionBaseMock::dataCallback handler by invoking SessionBase::setDataCallback
	processorSessionMock.getResource()->setDataCallback(
		std::bind(
			&SessionBaseMock::dataCallback,
			processorSessionMock.getResource(),
			std::placeholders::_1,
			std::placeholders::_2,
			std::placeholders::_3
		)
	);

	// setting SessionBaseMock::openCallback handler by invoking SessionBase::setOpenCallback
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

	// invoking open method from CLI server processor's thread
	processorServer.process(
		[&] {
			processorSessionMock.getResource()->open();
		}
	);

	// waiting for all tasks to complete
	processorServer.flush();

	// TODO: dataCallback and openCallback parameters should be tested; currently only execution flow and socket output is tested
	// TODO: testing if SocketMock::send was used from the proper thread

	// testing if correct data was sent to the socket
	ASSERT_EQ(socketRawPtr->sentMessages.size(), (unsigned int) 2);
	ASSERT_EQ(socketRawPtr->sentMessages[0], CLI::Messages::negotiateCommands);
	ASSERT_EQ(socketRawPtr->sentMessages[1], *processorServer.getResource()->getPromptMessage());
}


TEST(SessionBase, Close1) {

	// creating server instance to be used for create a session mock object
	conwrap::ProcessorAsio<Server> processorServer(
		1234,
		1,
		nullptr
	);

	// creating session mock instance to be tested
	auto socketPtr          = std::make_unique<SocketMock>(&processorServer);
	auto socketRawPtr       = socketPtr.get();
	auto sessionBaseMockPtr = std::make_unique<SessionBaseMock>(
		&processorServer,
		std::move(socketPtr)
	);
	socketRawPtr->setSession(sessionBaseMockPtr.get());
	conwrap::ProcessorQueue<SessionBaseMock> processorSessionMock(std::move(sessionBaseMockPtr));

	// setting SessionBaseMock::closeCallback handler by invoking SessionBase::setCloseCallback
	processorSessionMock.getResource()->setCloseCallback(
		std::bind(
			&SessionBaseMock::closeCallback,
			processorSessionMock.getResource(),
			std::placeholders::_1,
			std::placeholders::_2
		)
	);

	// defining expectations
	testing::Sequence s;
	EXPECT_CALL(*socketRawPtr, closeHook())
		.Times(1)
		.InSequence(s);
	EXPECT_CALL(*processorSessionMock.getResource(), closeCallbackHook())
		.Times(1)
		.InSequence(s);

	// invoking open method from CLI server processor's thread
	processorServer.process(
		[&] {
			processorSessionMock.getResource()->close();
		}
	);

	// waiting for all tasks to complete
	processorServer.flush();
}
