#include <g3log/logworker.hpp>
#include <gtest/gtest.h>
#include <iostream>
#include <log/FileSink.h>
#include <log/log.h>


int main(int argc, char** argv) {

	// initializing log
	auto logWorkerPtr = g3::LogWorker::createLogWorker();
	g3::initializeLogging(logWorkerPtr.get());
	logWorkerPtr->addSink(
		std2::make_unique<FileSink>(
			std::string("UnitTests.log"),
			std::string("")
		),
		&FileSink::save
	);

	// running all tests
	testing::InitGoogleTest(&argc, argv);
	int return_value = RUN_ALL_TESTS();

	std::cout << "Unit testing was finished" << std::endl;
	return return_value;
}
