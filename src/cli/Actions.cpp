#include <cli/Action.h>
#include <cli/Actions.h>
#include <cli/Command.h>
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


const char* usageString = "Usage: <category> <action> [options...]\n\rType 'help show' to display help message\n\r";


//auto helpActionVersion = [](Command* commandPtr, std::shared_ptr<std::vector<std::string>>) {
//	commandPtr->getSession()->getSocket()->send("Hello 1\n\r");
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
	std::function<void(Command*, std::shared_ptr<std::vector<std::string>>)> cliActionList = [this](Command* commandPtr, std::shared_ptr<std::vector<std::string>>) {
		for (auto& category : categories) {
			for (auto& action : category.second) {
				commandPtr->getSession()->getSocket()->send(category.first.c_str());
				commandPtr->getSession()->getSocket()->send(" ");
				commandPtr->getSession()->getSocket()->send(action.first.c_str());
				commandPtr->getSession()->getSocket()->send("\n\r");
			}
		}
	};

	// adding 'cli list' action
	addAction({"cli", "list", cliActionList});

	LOG(DEBUG) << "CLI: Default CLI actions were added (id=" << this << ")";
}


void Actions::addDefaultLogActions() {

	// defining 'log show' action
	auto logActionShow = [](Command* commandPtr, std::shared_ptr<std::vector<std::string>>) {
		auto stopping = false;

		// TODO: this is very dangerous, refactoring is required:
		commandPtr->setCancelHandler([&](Command* commandPtr) {
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
		auto socketSinkPtr = commandPtr->getSession()->getServer()->getLogger()->addSink(
			std2::make_unique<SocketSink>(
				commandPtr->getSession()->getSocket(),
				filter
			),
			&SocketSink::log
		);

		// waiting for stop signal
		for (int i = 0; !stopping; i++) {
			usleep(100000);
		}

		// removing socket sink from the logger
		commandPtr->getSession()->getServer()->getLogger()->removeSink(
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
	Action action = Action(
		[](Command* commandPtr, std::shared_ptr<std::vector<std::string>>) {}
	);

	if (actionTokensPtr->size() == 1) {

		// error message action is returned if there are too few parameters
		action = Action(
			[](Command* commandPtr, std::shared_ptr<std::vector<std::string>>) {
				// TODO: correct message
				commandPtr->getSession()->getSocket()->send("Invalid action command\n\r");
				commandPtr->getSession()->getSocket()->send(usageString);
			}
		);
	} else if (actionTokensPtr->size() > 1) {

		// seaching for action's category
		auto iterator1 = categories.find((*actionTokensPtr)[0]);
		if (iterator1 == categories.end()) {

			// error message action is returned if no category was found
			action = Action(
				[](Command* commandPtr, std::shared_ptr<std::vector<std::string>>) {
					// TODO: correct message
					commandPtr->getSession()->getSocket()->send("Action category was not found\n\rType 'cli show...'\n\r");
				}
			);
		} else {

			// searching for an action within this category
			auto iterator2 = (*iterator1).second.find((*actionTokensPtr)[1]);
			if (iterator2 == (*iterator1).second.end()) {

				// error message action is returned if no action was found within this category
				action = Action(
					[](Command* commandPtr, std::shared_ptr<std::vector<std::string>>) {
						// TODO: correct message
						commandPtr->getSession()->getSocket()->send("Action was not found within ... category\n\rType...\n\r");
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
