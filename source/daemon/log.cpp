#include "log.hpp"

Log::Log(std::string name, Type type, LogLevel level, const std::string logfile) {
	this->name = name;
	this->type = type;
	this->level = level;
	this->logFile = logfile;

	if (type == Type::SYSLOG) {
		openlog(name.c_str(), LOG_PID | LOG_CONS, LOG_DAEMON);
	} else if (type == Type::FILE) {
		std::ofstream logFile(logfile, std::ios_base::app);
		if (!logFile.is_open()) {
			throw std::ios_base::failure("Failed to open log file");
		}
		logFile.close();
	} else {
		throw std::invalid_argument("Invalid log type");
	}
}

Log::~Log() {
	// Destructor implementation
}

void Log::doLog(const std::string &message) {
	switch (this->type) {
		case Type::STDOUT:
			std::cout << message << std::endl;
			break;
		case Type::STDERR:
			std::cerr << message << std::endl;
			break;
		case Type::SYSLOG:
			syslog(convertLogLevelToSyslog(), "%s", message.c_str());
			break;
		case Type::FILE:
			{
				std::ofstream logFile(this->logFile, std::ios_base::app);
				if (logFile.is_open()) {
					logFile << message << std::endl;
					logFile.close();
				} else {
					throw std::ios_base::failure("Failed to open log file");
				}
			}
			break;
	}
}

int Log::convertLogLevelToSyslog() {
	switch (this->level) {
		case LogLevel::ERR:
			return LOG_ERR;
		case LogLevel::WARNING:
			return LOG_WARNING;
		case LogLevel::NOTICE:
			return LOG_NOTICE;
		case LogLevel::INFO:
			return LOG_INFO;
		case LogLevel::DEBUG:
			return LOG_DEBUG;
		default:
			throw std::invalid_argument("Invalid log level");
	}
}
