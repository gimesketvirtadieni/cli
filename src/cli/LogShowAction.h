#ifndef LogShowAction_INCLUDED
#define LogShowAction_INCLUDED

#include <cli/Action.h>
#include <cli/Context.h>
#include <log/log.h>
#include <string>
#include <vector>


class LogShowAction : public Action
{
	public:
		             LogShowAction();
		virtual void main(Context&) override;

	protected:
		static bool containsAny(g3::Labels, std::vector<std::string>);
};

#endif // LogShowAction_INCLUDED
