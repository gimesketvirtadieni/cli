#ifndef FileSink_INCLUDED
#define FileSink_INCLUDED

#include <g3log/logmessage.hpp>
#include <functional>
#include <string>
#include <memory>
#include "SinkFilter.h"


class FileSink : public SinkFilter {
	public:
		            FileSink(const std::string &log_prefix, const std::string &log_directory, std::function<bool(g3::LogMessage&)> = NULL);
		            FileSink &operator=(const FileSink &) = delete;
		            FileSink(const FileSink &other) = delete;
		virtual    ~FileSink();
		std::string changeLogFile(const std::string &directory);
		std::string createLogFileNamee(const std::string &verified_prefix);
		std::string fileName();
		void        save(g3::LogMessageMover message);

	protected:
		void           addLogFileHeader();
		std::ofstream& getFileStream();

	private:
		std::string                    _log_file_with_path;
		std::string                    _log_prefix_backup; // needed in case of future log file changes of directory
		std::unique_ptr<std::ofstream> _outptr;
};

#endif // FileSink_INCLUDED
