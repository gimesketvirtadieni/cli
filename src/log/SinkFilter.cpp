#include "SinkFilter.h"


SinkFilter::SinkFilter(std::function<bool(g3::LogMessage&)> filter) :
	_filter(filter) {
}


SinkFilter::~SinkFilter() {
}


bool SinkFilter::filter(g3::LogMessage& logMessage) {
	return (_filter && _filter(logMessage));
}
