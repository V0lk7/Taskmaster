#ifndef LOG_HPP
#define LOG_HPP

#include <string>
#include <fstream>
#include <syslog.h>
#include <iostream>
#include <stdexcept>

class Log {
public:
	enum class LogLevel {
		ERR,
		WARNING,
		NOTICE,
		INFO,
		DEBUG
	};

	enum class Type {
		STDOUT,
		STDERR,
		SYSLOG,
		FILE
	};

	Log(std::string name, Type type, LogLevel level, const std::string logfile);
	~Log();
	void doLog(const std::string &message);
	int convertLogLevelToSyslog();

private:
	std::string name;
	Type type;
	LogLevel level;
	std::string logFile;
};

#endif
