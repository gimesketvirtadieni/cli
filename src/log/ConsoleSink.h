#ifndef ConsoleSink_INCLUDED
#define ConsoleSink_INCLUDED

#include <functional>
#include <g3log/logmessage.hpp>
#include "SinkFilter.h"


class ConsoleSink : public SinkFilter {
	public:
		             ConsoleSink(std::function<bool(g3::LogMessage&)> = NULL);
		             ConsoleSink(const ConsoleSink&) = delete;
		             ConsoleSink& operator=(const ConsoleSink&) = delete;
		virtual     ~ConsoleSink();
		        void print(g3::LogMessageMover);

	protected:
		std::string rightTrim(const std::string&);
};

#endif // ConsoleSink_INCLUDED
