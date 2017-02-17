#include <algorithm>
#include <cli/LogShowAction.h>
#include <cli/Messages.h>
#include <log/log.h>
#include <log/SocketSink.h>

//#include <iostream>


LogShowAction::LogShowAction()
	: Action("log", "show") {}


bool LogShowAction::containsAny(g3::Labels a, std::vector<std::string> b)
{
	bool found = false;

	for(auto l : b)
	{
		if(a.find(l) != a.end())
		{
			found = true;
			break;
		}
	}

	return found;
}


void LogShowAction::main(Context& context)
{
	auto stopping = false;
	auto labels   = std::vector<std::string>{};

	// setting cancel handler
	context.setCancelHandler([&]
	{
		stopping = true;
	});

	// checking if labels were provided in the parameters
	for(auto l : context.getParameters())
	{
		// TODO: move to Messages
		auto labelsMarker = std::string{"labels="};
		if (l.find(labelsMarker) == 0)
		{
			// there is a list of labels provided
			auto labelsValue = l.substr(labelsMarker.size());

			// saving labels into a set so fileter can use it to filter out the rest
			labels = cli::splitIntoWords(labelsValue, ',');
		}
	}

	// defining filter for log entries
	auto filter = [&](g3::LogMessage& logMessage)
	{
		auto filter = false;

		// filtering out one noicy line
		if (logMessage._level == DEBUG && !logMessage._file.compare("alsaPlayer.cpp") && logMessage._line == 243)
		{
			filter = true;
		}

		// if labels provided then filtering out entries that do not contain required labels
		if (!filter && labels.size() > 0 && !containsAny(logMessage._labels, labels))
		{
			filter = true;
		}

		return filter;
	};

	// adding socket sink to the logger
	auto socketSinkPtr = context.getLogger().addSink(
		std::make_unique<SocketSink>(
			&context.getSocket(),
			filter
		),
		&SocketSink::log
	);

	// waiting for stop signal
	for (int i = 0; !stopping; i++)
	{
		usleep(100000);
	}

	// removing socket sink from the logger
	context.getLogger().removeSink(
		std::move(
			socketSinkPtr
		)
	);
}
