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
};

#endif // ConsoleSink_INCLUDED
