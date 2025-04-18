#include "log.hpp"

Log::Log(std::string name, Type type, LogLevel level, const std::string logfile) {
	this->_name = name;
	this->_type = type;
	this->_level = level;
	this->_logFile = logfile;

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
	if (this->_type == Type::SYSLOG) {
		closelog();
	}
	if (this->_type == Type::FILE) {
		std::ofstream logFile(this->_logFile, std::ios_base::app);
		if (logFile.is_open()) {
			logFile.close();
		}
	}
}

Log::LogLevel Log::getLogLevel() {
	return this->_level;
}

void Log::doLog(const std::string &message) {
	switch (this->_type) {
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
				std::ofstream logFile(this->_logFile, std::ios_base::app);
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
	switch (this->_level) {
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

Log::LogLevel convertStringToLogLevel(const std::string &str) {
	if (str == "ERR") {
		return Log::LogLevel::ERR;
	} else if (str == "WARNING") {
		return Log::LogLevel::WARNING;
	} else if (str == "NOTICE") {
		return Log::LogLevel::NOTICE;
	} else if (str == "INFO") {
		return Log::LogLevel::INFO;
	} else if (str == "DEBUG") {
		return Log::LogLevel::DEBUG;
	} else {
		throw std::invalid_argument("Invalid log level string");
	}
}
