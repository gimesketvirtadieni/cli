#ifndef Action_INCLUDED
#define Action_INCLUDED

#include <functional>
#include <memory>
#include <vector>


// forward declaration
class Command;


using ActionHandler = std::function<void (Command*, std::shared_ptr<std::vector<std::string>>)>;


class Action {
	public:
		                             Action(ActionHandler);
		                             Action(const Action&) = default;
		                             Action(const char*, const char*, ActionHandler);
		                             Action(std::shared_ptr<std::string>, std::shared_ptr<std::string>, ActionHandler);
		std::shared_ptr<std::string> getActionName();
		std::shared_ptr<std::string> getCategoryName();
		ActionHandler                getHandler();

	private:
		std::shared_ptr<std::string> actionNamePtr;
		std::shared_ptr<std::string> categoryNamePtr;
		ActionHandler                actionHandler;
};

#endif // Action_INCLUDED
