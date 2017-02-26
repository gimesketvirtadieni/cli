#ifndef ActionLogShow_INCLUDED
#define ActionLogShow_INCLUDED

#include <cli/Action.h>
#include <cli/Context.h>
#include <log/log.h>
#include <string>
#include <vector>


class ActionLogShow : public Action
{
	public:
		             ActionLogShow();
		virtual void main(Context&) override;

	protected:
		static bool containsAny(LABELS&, std::vector<std::string>&);
};

#endif // ActionLogShow_INCLUDED
