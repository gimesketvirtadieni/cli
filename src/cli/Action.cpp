#include <cli/Action.h>


Action::Action(std::string c, std::string n)
	: Action(c, n, nullptr) {}


Action::Action(std::string c, std::string n, std::function<void(Context&)> h)
	: categoryName(c)
	, actionName(n)
	, actionHandler(h) {}


void Action::operator()(Context& context)
{
	main(context);
}


std::string Action::getCategoryName()
{
	return categoryName;
}


std::function<void(Context&)> Action::getHandler()
{
	return actionHandler;
}


std::string Action::getName()
{
	return actionName;
}


void Action::main(Context& context)
{
	if (actionHandler)
	{
		actionHandler(context);
	}
}
