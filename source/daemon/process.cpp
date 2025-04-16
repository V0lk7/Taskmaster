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

Process::Process(const std::string &name, const std::string &command) {
	this->_name = name;
	this->_state = State::STOPPED;
	this->_command = command;
	this->_workdir = ".";
	this->_nbprocess = 1;
	this->_autostart = true;
	this->_restart = Restart::UNEXPECTED;
	this->_exitcodes = std::vector<int>();
	this->_exitcodes.push_back(0);
	this->_startdelay = 1;
	this->_restartretry = 3;
	this->_stopsignal = SIGTERM;
	this->_stoptimeout = 10;
	this->_stdoutfile = "AUTO";
	this->_stderrfile = "AUTO";
	this->_umask = -1;
	this->_env = std::map<std::string, std::string>();
	this->_logs = std::vector<Log>();
}

Process::~Process() {
}

void Process::setState(State state) {
	this->_state = state;
}

Process::State Process::getState() const {
	return this->_state;
}
void Process::setName(const std::string &name) {
	this->_name = name;
}
std::string Process::getName() const {
	return this->_name;
}
void Process::setCommand(const std::string &command) {
	this->_command = command;
}
std::string Process::getCommand() const {
	return this->_command;
}
void Process::setWorkdir(const std::string &workdir) {
	this->_workdir = workdir;
}
std::string Process::getWorkdir() const {
	return this->_workdir;
}
void Process::setNbprocess(int nbprocess) {
	this->_nbprocess = nbprocess;
}
int Process::getNbprocess() const {
	return this->_nbprocess;
}
void Process::setAutostart(bool autostart) {
	this->_autostart = autostart;
}
bool Process::getAutostart() const {
	return this->_autostart;
}
void Process::setRestart(Restart restart) {
	this->_restart = restart;
}
Process::Restart Process::getRestart() const {
	return this->_restart;
}
void Process::setExitcodes(std::vector<int> exitcodes) {
	this->_exitcodes = exitcodes;
}
std::vector<int> Process::getExitcodes() const {
	return this->_exitcodes;
}
void Process::setStartdelay(int startdelay) {
	this->_startdelay = startdelay;
}
int Process::getStartdelay() const {
	return this->_startdelay;
}
void Process::setRestartretry(int restartretry) {
	this->_restartretry = restartretry;
}
int Process::getRestartretry() const {
	return this->_restartretry;
}
void Process::setStopsignal(int stopsignal) {
	this->_stopsignal = stopsignal;
}
void Process::setStopsignalString(const std::string &stopsignal) {
	this->_stopsignal = convertStringToStopsignal(stopsignal);
}
int Process::getStopsignal() const {
	return this->_stopsignal;
}
void Process::setStoptimeout(int stoptimeout) {
	this->_stoptimeout = stoptimeout;
}
int Process::getStoptimeout() const {
	return this->_stoptimeout;
}
void Process::addEnv(const std::string &key, const std::string &value) {
	this->_env[key] = value;
}
std::map<std::string, std::string> Process::getEnv() const {
	return this->_env;
}
void Process::setStdoutfile(const std::string &stdoutfile) {
	this->_stdoutfile = stdoutfile;
}
void Process::setStderrfile(const std::string &stderrfile) {
	this->_stderrfile = stderrfile;
}
void Process::setUmask(mode_t umask) {
	this->_umask = umask;
}
mode_t Process::getUmask() const {
	return this->_umask;
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
		case Restart::TRUE:
			return "true";
		case Restart::UNEXPECTED:
			return "unexpected";
		case Restart::FALSE:
			return "false";
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

std::string Process::convertStopsignalToString(int signal) {
	switch (signal) {
		case SIGTERM:
			return "TERM";
		case SIGKILL:
			return "KILL";
		case SIGINT:
			return "INT";
		case SIGHUP:
			return "HUP";
		default:
			throw std::invalid_argument("Invalid signal");
	}
}

int Process::convertStringToStopsignal(const std::string &str) {
	if (str == "TERM") {
		return SIGTERM;
	} else if (str == "KILL") {
		return SIGKILL;
	} else if (str == "INT") {
		return SIGINT;
	} else if (str == "HUP") {
		return SIGHUP;
	} else {
		throw std::invalid_argument("Invalid signal string");
	}
}

Process::Restart convertStringToRestart(const std::string &str) {
	if (str == "true") {
		return Process::Restart::TRUE;
	} else if (str == "unexpected") {
		return Process::Restart::UNEXPECTED;
	} else if (str == "false") {
		return Process::Restart::FALSE;
	} else {
		throw std::invalid_argument("Invalid restart option");
	}
}
