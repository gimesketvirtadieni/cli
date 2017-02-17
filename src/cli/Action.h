#ifndef Action_INCLUDED
#define Action_INCLUDED

#include <cli/Context.h>
#include <functional>
#include <string>


class Action
{
	public:
		                                      Action(std::string, std::string);
		                                      Action(std::string, std::string, std::function<void(Context&)>);
		virtual void                          operator()(Context&);
		virtual std::string                   getCategoryName();
		virtual std::function<void(Context&)> getHandler();
		virtual std::string                   getName();
		virtual void                          main(Context&);

	private:
		std::string                   actionName;
		std::string                   categoryName;
		std::function<void(Context&)> actionHandler;
};

#endif // Action_INCLUDED
