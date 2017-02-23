#include <cli/Actions.h>
#include <cli/LogShowAction.h>
#include <log/log.h>

//#include <iostream>


Actions::Actions() {
	LOG(DEBUG) << LABELS{"cli"} << "Actions object was created (id=" << this << ")";
}


Actions::~Actions() {
	LOG(DEBUG) << LABELS{"cli"} << "Actions object was deleted (id=" << this << ")";
}


// TODO: move to Messages
const char* usageString = "Usage: <category> <action> [options...]\r\nType 'help show' to display help message\r\n";


void Actions::addAction(std::string categoryName, std::string actionName, std::function<void(Context&)> action) {

	// seaching for action's category
	auto iterator1 = categories.find(categoryName);
	if (iterator1 == categories.end()) {

		// creating a new category with the first action in it
		iterator1 = categories.emplace(
			std::make_pair(
				categoryName,
				std::map<std::string, std::function<void(Context&)>> {
					std::make_pair(
						actionName,
						action
					)
				}
			)
		).first;
	} else {

		// searching for an action within this category
		auto iterator2 = (*iterator1).second.find(actionName);
		if (iterator2 == (*iterator1).second.end()) {

			// adding a new action into found category
			iterator2 = (*iterator1).second.emplace(
				std::make_pair(
					actionName,
					action
				)
			).first;
		} else {

			// updating found action
			(*iterator2).second = action;
		}
	}
}


void Actions::addDefaultCLIActions() {
	LOG(DEBUG) << LABELS{"cli"} << "Adding default CLI actions (id=" << this << ")...";

	// defining 'cli list' action
	std::function<void(Context&)> cliActionList = [this](auto& context) {
		for (auto& category : categories) {
			for (auto& action : category.second) {
				context.getOutput() << category.first << " " << action.first.c_str() << "\r\n";
			}
		}
	};

	// adding 'cli list' action
	addAction({"cli"}, {"list"}, cliActionList);

	LOG(DEBUG) << LABELS{"cli"} << "Default CLI actions were added (id=" << this << ")";
}


void Actions::addDefaultLogActions() {

	// registering 'log show' action
	addAction(LogShowAction());
}


std::function<void(Context&)> Actions::findAction(std::vector<std::string> parameters) {
	std::function<void(Context&)> result = [](auto& context) {};

	// defining default action which is an empty action
	if (parameters.size() == 1) {

		// error message action is returned if no action was found within this category
		result = [](auto& context) {
			// TODO: correct message
			context.getOutput() << "Too few parameters were provided...\r\n";
		};
	} else if (parameters.size() > 1) {
		auto categoryName = parameters[0];
		auto actionName   = parameters[1];

		// searching for a category
		auto iterator1 = categories.find(categoryName);
		if (iterator1 == categories.end()) {

			// error message action is returned if no category was found
			result = [](auto& context) {
				// TODO: correct message
				context.getOutput() << "Category was not found\r\nType 'cli show...'\r\n";
			};
		} else {

			// searching for an action within this category
			auto iterator2 = (*iterator1).second.find(actionName);
			if (iterator2 == (*iterator1).second.end()) {

				// error message action is returned if no action was found within this category
				result = [](auto& context) {
					// TODO: correct message
					context.getOutput() << "Action was not found within ... category\r\nType...\r\n";
				};
			} else {

				// required action was found
				result = (*iterator2).second;
			}
		}
	}

	return result;
}
