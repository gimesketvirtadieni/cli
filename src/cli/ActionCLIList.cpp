#include <cli/ActionCLIList.h>
#include <cli/Actions.h>
#include <cli/Messages.h>


ActionCLIList::ActionCLIList()
	: Action("cli", "list") {}


void ActionCLIList::main(Context& context)
{
	for (auto& category : context.getActions().getCategories())
	{
		for (auto& action : category.second)
		{
			context.getOutput()
				<< category.first
				<< " "
				<< action.first.c_str()
				<< cli::Messages::endOfLine;
		}
	}
}
