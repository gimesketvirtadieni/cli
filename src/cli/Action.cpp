#include <cli/Action.h>
#include <cli/Context.h>

#include <iostream>


Action::Action(ActionHandler actionHandler) :
	Action("", "", actionHandler) {
}

/*
Action::Action(const Action& action) {
	std::cout << "HELLO" << std::endl;
	actionNamePtr = action.actionNamePtr;
	categoryNamePtr = action.categoryNamePtr;
	actionHandler = action.actionHandler;
}
*/

Action::Action(const char* categoryName, const char* actionName, ActionHandler actionHandler) :
	Action(std::make_shared<std::string>(categoryName), std::make_shared<std::string>(actionName), actionHandler) {
}


Action::Action(std::shared_ptr<std::string> categoryNamePtr, std::shared_ptr<std::string> actionNamePtr, ActionHandler actionHandler) :
	categoryNamePtr(categoryNamePtr),
	actionNamePtr(actionNamePtr),
	actionHandler(actionHandler) {
}


std::shared_ptr<std::string> Action::getActionName() {
	return actionNamePtr;
}


std::shared_ptr<std::string> Action::getCategoryName() {
	return categoryNamePtr;
}


ActionHandler Action::getHandler() {
	return actionHandler;
}
