#ifndef ActionCLIList_INCLUDED
#define ActionCLIList_INCLUDED

#include <cli/Action.h>
#include <cli/Context.h>


class ActionCLIList : public Action
{
	public:
		             ActionCLIList();
		virtual void main(Context&) override;
};

#endif // ActionCLIList_INCLUDED
