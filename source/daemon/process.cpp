#include "process.hpp"

Process::Process(const std::string &name, const std::string &command, const std::string &workdir,
	int nbprocess, bool autostart, Restart restart, std::vector<int> exitcodes,
	int startdelay, int restartretry, int stopsignal, int stoptimeout,
	const std::string &stdoutfile, const std::string &stderrfile,
	mode_t umask, std::map<std::string, std::string> env) {
	this->_name = name;
	this->_state = State::STOPPED;
	this->_command = command;
	this->_workdir = workdir;
	this->_nbprocess = nbprocess;
	this->_autostart = autostart;
	this->_restart = restart;
	this->_exitcodes = exitcodes;
	this->_startdelay = startdelay;
	this->_restartretry = restartretry;
	this->_stopsignal = stopsignal;
	this->_stoptimeout = stoptimeout;
	this->_stdoutfile = stdoutfile;
	this->_stderrfile = stderrfile;
	this->_umask = umask;
	this->_env = env;
	this->_logs = std::vector<Log>();
	if (!this->_stdoutfile.empty()) {
		Log log = Log(name, Log::Type::FILE, Log::LogLevel::INFO, this->_stdoutfile);
		this->_logs.push_back(log);
	}
	if (!this->_stderrfile.empty()) {
		Log log = Log(name, Log::Type::FILE, Log::LogLevel::ERR, this->_stderrfile);
		this->_logs.push_back(log);
	}
}

Process::~Process() {
}

void Process::setState(State state) {
	this->_state = state;
}

Process::State Process::getState() const {
	return this->_state;
}
std::string Process::getName() const {
	return this->_name;
}
std::string Process::getCommand() const {
	return this->_command;
}
std::string Process::getWorkdir() const {
	return this->_workdir;
}
int Process::getNbprocess() const {
	return this->_nbprocess;
}
bool Process::getAutostart() const {
	return this->_autostart;
}
Process::Restart Process::getRestart() const {
	return this->_restart;
}
std::vector<int> Process::getExitcodes() const {
	return this->_exitcodes;
}
int Process::getStartdelay() const {
	return this->_startdelay;
}
int Process::getRestartretry() const {
	return this->_restartretry;
}
int Process::getStopsignal() const {
	return this->_stopsignal;
}
int Process::getStoptimeout() const {
	return this->_stoptimeout;
}
std::map<std::string, std::string> Process::getEnv() const {
	return this->_env;
}

void Process::doLog(const std::string &message, Log::LogLevel level) {
	for (auto &log : this->_logs) {
		if (log.getLogLevel() == level) {
			log.doLog(message);
		}
	}
}

std::string Process::convertRestartToString(Restart restart) {
	switch (restart) {
		case Restart::ALWAYS:
			return "always";
		case Restart::ON_FAILURE:
			return "on-failure";
		case Restart::NEVER:
			return "never";
		default:
			throw std::invalid_argument("Invalid restart option");
	}
}

std::string Process::convertStateToString(State state) {
	switch (state) {
		case State::STOPPED:
			return "STOPPED";
		case State::RUNNING:
			return "RUNNING";
		case State::STARTING:
			return "STARTING";
		case State::STOPPING:
			return "STOPPING";
		case State::EXITED:
			return "EXITED";
		default:
			throw std::invalid_argument("Invalid state");
	}
}

Process::Restart convertStringToRestart(const std::string &str) {
	if (str == "always") {
		return Process::Restart::ALWAYS;
	} else if (str == "on-failure") {
		return Process::Restart::ON_FAILURE;
	} else if (str == "never") {
		return Process::Restart::NEVER;
	} else {
		throw std::invalid_argument("Invalid restart option");
	}
}
