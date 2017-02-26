#include <cli/Action.h>


Action::Action(std::string c, std::string n)
	: Action(c, n, nullptr) {}


Action::Action(std::string c, std::string n, std::function<void(Context&)> h)
	: categoryName(c)
	, actionName(n)
	, actionHandler(h) {}


Action::~Action() {}


void Action::operator()(Context& context)
{
	main(context);
}


const std::string& Action::getCategoryName()
{
	return categoryName;
}


const std::function<void(Context&)>& Action::getHandler()
{
	return actionHandler;
}


const std::string& Action::getName()
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
