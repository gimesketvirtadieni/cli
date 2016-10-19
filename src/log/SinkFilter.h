#ifndef SinkFilter_INCLUDED
#define SinkFilter_INCLUDED

#include <functional>
#include <g3log/logmessage.hpp>


// TODO: this should be in a formatter file
std::string rightTrim(const std::string&);


class SinkFilter {
	public:
		             SinkFilter(std::function<bool(g3::LogMessage&)> = NULL);
		virtual     ~SinkFilter();
		        bool filter(g3::LogMessage&);

	private:
		std::function<bool(g3::LogMessage&)> _filter;
};

#endif // SinkFilter_INCLUDED
