#ifndef Actions_INCLUDED
#define Actions_INCLUDED

#include <cli/Action.h>
#include <functional>
#include <map>
#include <memory>
#include <vector>


using ActionsMap    = std::map<std::string, Action>;
using CategoriesMap = std::map<std::string, ActionsMap>;


class Actions {
	public:
		       Actions();
	          ~Actions();
		void   addAction(Action);
		void   addDefaultCLIActions();
		void   addDefaultLogActions();
		Action findAction(std::shared_ptr<std::vector<std::string>>);

	private:
		CategoriesMap categories;
};

#endif // Actions_INCLUDED
