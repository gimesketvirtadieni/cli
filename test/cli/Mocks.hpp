#ifndef Mocks_INCLUDED
#define Mocks_INCLUDED

#include <atomic>
#include <boost/asio.hpp>
#include <chrono>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>
#include <thread>

#include <iostream>
#include <log/log.h>


// forward declaration
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
	mutable boost::asio::io_service dispatcher;
	mutable bool                    enableResetHook = true;
	mutable bool                    enableRunHook   = true;

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


struct Dummy {
	Dummy() {}
	Dummy(Processor<Dummy>* processorPtr) : processorPtr(processorPtr) {}
	virtual ~Dummy() {}

	Processor<Dummy>* processorPtr;
};

#endif // Mocks_INCLUDED
