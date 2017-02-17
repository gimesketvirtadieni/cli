#ifndef Actions_INCLUDED
#define Actions_INCLUDED

#include <cli/Action.h>
#include <cli/Context.h>
#include <functional>
#include <map>
#include <string>
#include <vector>


using ActionsMap    = std::map<std::string, std::function<void(Context&)>>;
using CategoriesMap = std::map<std::string, ActionsMap>;


class Actions {
	public:
		                              Actions();
	                                 ~Actions();
		void                          addAction(std::string, std::string, std::function<void(Context&)>);
		void                          addDefaultCLIActions();
		void                          addDefaultLogActions();
		std::function<void(Context&)> findAction(std::vector<std::string>);
		template<class T>
		void addAction(T action)
		{
			// compile-time sanity check
			static_assert(std::is_base_of<Action, T>::value, "Provided action is not not derived from Action class");

			// ...
			addAction(action.getCategoryName(), action.getName(), action);
		}

	private:
		CategoriesMap categories;
};

#endif // Actions_INCLUDED
