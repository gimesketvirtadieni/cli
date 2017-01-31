#include <cli/Action.h>
#include <cli/Actions.h>
#include <cli/Context.h>
#include <cli/Session.h>
#include <cli/Server.h>
#include <cli/Socket.h>
#include <log/log.h>
#include <log/SocketSink.h>

//#include <iostream>


Actions::Actions() {
	LOG(DEBUG) << "CLI: Actions object was created (id=" << this << ")";
}


Actions::~Actions() {
	LOG(DEBUG) << "CLI: Actions object was deleted (id=" << this << ")";
}


const char* usageString = "Usage: <category> <action> [options...]\r\nType 'help show' to display help message\r\n";


//auto helpActionVersion = [](Command* commandPtr, std::shared_ptr<std::vector<std::string>>) {
//	commandPtr->getSession()->getSocket()->send("Hello 1\r\n");
//};


void Actions::addAction(Action action) {
	// validating input parameters
	// TODO: ...

	// seaching for action's category
	auto iterator1 = categories.find(*action.getCategoryName());
	if (iterator1 == categories.end()) {

		// creating a new category with the first action in it
		iterator1 = categories.emplace(
			std::make_pair(
				*action.getCategoryName(),
				std::map<std::string, Action> {
					std::make_pair(
						*action.getActionName(),
						action
					)
				}
			)
		).first;
	} else {

		// searching for an action within this category
		auto iterator2 = (*iterator1).second.find(*action.getActionName());
		if (iterator2 == (*iterator1).second.end()) {

			// adding a new action into found category
			iterator2 = (*iterator1).second.emplace(
				std::make_pair(
					*action.getActionName(),
					action
				)
			).first;
		} else {

			// updating found action
			(*iterator2).second = action;
		}
	}
}


//Action helpActions[] = {
//	{"help", "version", helpActionVersion}
//};


void Actions::addDefaultCLIActions() {
	LOG(DEBUG) << "CLI: Adding default CLI actions (id=" << this << ")...";

	// defining 'cli list' action
	std::function<void(Context&)> cliActionList = [this](auto& context) {
		for (auto& category : categories) {
			for (auto& action : category.second) {
				context.getOutput() << category.first.c_str() << " " << action.first.c_str() << "\r\n";
			}
		}
	};

	// adding 'cli list' action
	addAction({"cli", "list", cliActionList});

	LOG(DEBUG) << "CLI: Default CLI actions were added (id=" << this << ")";
}


void Actions::addDefaultLogActions() {

	// defining 'log show' action
	auto logActionShow = [](auto& context) {
		auto stopping = false;

		// TODO: this is very dangerous, refactoring is required:
		context.setCancelHandler([&] {
			stopping = true;
		});

		// defining filter for log entries
		// TODO: find a way to reuse from snapClient.cpp
		auto filter = [](g3::LogMessage& logMessage) {
			auto filter = false;

			// filtering one noicy line
			if (logMessage._level == DEBUG && !logMessage._file.compare("alsaPlayer.cpp") && logMessage._line == 243) {
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
		for (int i = 0; !stopping; i++) {
			usleep(100000);
		}

		// removing socket sink from the logger
		context.getLogger().removeSink(
			std::move(
				socketSinkPtr
			)
		);
	};

	// adding 'log show' action
	addAction({"log", "show", logActionShow});
}


Action Actions::findAction(std::shared_ptr<std::vector<std::string>> actionTokensPtr) {

	// defining default action which is an empty action
	Action action = Action([](auto&) {});

	if (actionTokensPtr->size() == 1) {

		// error message action is returned if there are too few parameters
		action = Action([](auto& context)
		{
			// TODO: correct message
			context.getOutput() << "Invalid action command\r\n" << usageString;
		});
	} else if (actionTokensPtr->size() > 1) {

		// seaching for action's category
		auto iterator1 = categories.find((*actionTokensPtr)[0]);
		if (iterator1 == categories.end()) {

			// error message action is returned if no category was found
			action = Action(
				[](auto& context) {
					// TODO: correct message
					context.getOutput() << "Action category was not found\r\nType 'cli show...'\r\n";
				}
			);
		} else {

			// searching for an action within this category
			auto iterator2 = (*iterator1).second.find((*actionTokensPtr)[1]);
			if (iterator2 == (*iterator1).second.end()) {

				// error message action is returned if no action was found within this category
				action = Action(
					[](auto& context) {
						// TODO: correct message
						context.getOutput() << "Action was not found within ... category\r\nType...\r\n";
					}
				);
			} else {

				// required action was found
				action = (*iterator2).second;
			}
		}
	}
	return action;
}
