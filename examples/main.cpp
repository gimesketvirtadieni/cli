#include <chrono>
#include <cli/Actions.h>
#include <cli/Server.h>
#include <conwrap/ProcessorAsio.hpp>
#include <conwrap/Context.hpp>
#include <g3log/logworker.hpp>
#include <functional>
#include <iostream>
#include <log/ConsoleSink.h>
#include <log/FileSink.h>
#include <log/log.h>


int main(int argc, char *argv[])
{

	// initializing log
	auto logWorkerPtr = g3::LogWorker::createLogWorker();
	g3::initializeLogging(logWorkerPtr.get());

	// defining filter for log entries
	auto filter = [](g3::LogMessage& logMessage) {
		auto filter = false;

		// filtering one noicy line
		//if (logMessage._level == DEBUG && !logMessage._file.compare("alsaPlayer.cpp") && logMessage._line == 205) {
		//	filter = true;
		//}

		return filter;
	};

	// adding custom sinks with predefined filter
	logWorkerPtr->addSink(std2::make_unique<ConsoleSink>(filter), &ConsoleSink::print);
	auto handle = logWorkerPtr->addSink(std2::make_unique<FileSink>("CLI.log", "", filter), &FileSink::save);

	////////////////////////
	// registering custom CLI actions
	auto actionsPtr = std::make_unique<Actions>();

	// registering default actions
	actionsPtr->addDefaultCLIActions();
	actionsPtr->addDefaultLogActions();

	//for (auto action : snapActions) {
	//	actionsPtr->addAction(action);
	//}

	// creating a CLI server
	// TODO: port number should be derived from the settings
	auto processorPtr = std::make_unique<conwrap::ProcessorAsio<Server>>(15673, 2, logWorkerPtr.get(), std::move(actionsPtr));
	processorPtr->process(
		[](auto context) {
			context.getResource()->start();
		}
	);
	//////////////////////////////


    // stoping CLI server
	processorPtr->process(
		[](auto context) {
			context.getResource()->stop();
		}
	);

	// destroying concurrent wrapper which will wait for all pending async operations
	processorPtr.reset();

	return 0;
}
