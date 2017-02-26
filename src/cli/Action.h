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
		virtual                                     ~Action();
		virtual       void                           operator()(Context&);
		virtual const std::string&                   getCategoryName();
		virtual const std::function<void(Context&)>& getHandler();
		virtual const std::string&                   getName();
		virtual       void                           main(Context&);

	private:
		const std::string                   actionName;
		const std::string                   categoryName;
		const std::function<void(Context&)> actionHandler;
};

#endif // Action_INCLUDED
