#include <cassert>
#include <g3log/logworker.hpp>
#include "FileSink.h"
#include "FileSinkHelper.hpp"


FileSink::FileSink(const std::string &log_prefix, const std::string &log_directory, std::function<bool(g3::LogMessage&)> filter) :
	_log_file_with_path(log_directory),
	_log_prefix_backup(log_prefix),
	_outptr(new std::ofstream),
	SinkFilter(filter) {

	_log_prefix_backup = prefixSanityFix(log_prefix);
	if (!isValidFilename(_log_prefix_backup)) {
		std::cerr << "g3log: forced abort due to illegal log prefix [" << log_prefix << "]" << std::endl;

		// TODO: WTF
		abort();
	}

	_log_file_with_path = pathSanityFix(_log_file_with_path, _log_prefix_backup);
	_outptr = createLogFile(_log_file_with_path);

	if (!_outptr) {
		std::cerr << "Cannot write log file to location, attempting current directory" << std::endl;
		_log_file_with_path = "./" + _log_prefix_backup;
		_outptr = createLogFile(_log_file_with_path);
	}
	assert(_outptr && "cannot open log file at startup");
	addLogFileHeader();
}


FileSink::~FileSink() {
	std::string exit_msg {"\ng3log g3FileSink shutdown at: "};
	exit_msg.append(g3::localtime_formatted(g3::systemtime_now(), g3::internal::time_formatted));
	getFileStream() << exit_msg << std::flush;

	exit_msg.append({"\nLog file at: ["}).append(_log_file_with_path).append({"]\n\n"});
}


std::string FileSink::createLogFileNamee(const std::string &verified_prefix) {
   std::stringstream oss_name;

   oss_name << verified_prefix << ".log";

   return oss_name.str();
}


std::ofstream& FileSink::getFileStream() {
	return *(_outptr.get());
}


void FileSink::save(g3::LogMessageMover logEntry) {
	auto           logMessage = logEntry.get();
	std::ofstream& out(getFileStream());

	if (!filter(logMessage)) {
		out << logMessage.timestamp() << " "
			<< logMessage.level()
			<< " [" << logMessage.threadID() << "]"
			<< " (" << logMessage.file() << ":" << logMessage.line() << ")"
			<< (logMessage._labels.size() > 0 ? " {" + logMessage.labels() + "}" : "")
			<< " - " << rightTrim(logMessage.message())
			<< std::endl << std::flush;
	}
}


std::string FileSink::changeLogFile(const std::string &directory) {

      auto now = g3::systemtime_now();
      auto now_formatted = g3::localtime_formatted(now, {g3::internal::date_formatted + " " + g3::internal::time_formatted});

      std::string prospect_log = directory + _log_prefix_backup;
      std::unique_ptr<std::ofstream> log_stream = createLogFile(prospect_log);
      if (nullptr == log_stream) {
    	  getFileStream() << "\n" << now_formatted << " Unable to change log file. Illegal filename or busy? Unsuccessful log name was: " << prospect_log;
         return {}; // no success
      }

      addLogFileHeader();
      std::ostringstream ss_change;
      ss_change << "\n\tChanging log file from : " << _log_file_with_path;
      ss_change << "\n\tto new location: " << prospect_log << "\n";
      getFileStream() << now_formatted << ss_change.str();
      ss_change.str("");

      std::string old_log = _log_file_with_path;
      _log_file_with_path = prospect_log;
      _outptr = std::move(log_stream);
      ss_change << "\n\tNew log file. The previous log file was at: ";
      ss_change << old_log;
      getFileStream() << now_formatted << ss_change.str();
      return _log_file_with_path;
   }
   std::string FileSink::fileName() {
      return _log_file_with_path;
   }
   void FileSink::addLogFileHeader() {
	   getFileStream() << header();
   }
