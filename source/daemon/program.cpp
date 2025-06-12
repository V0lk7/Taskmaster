#include "daemon/program.hpp"

Program::Program(const std::string &name, const std::string &command) {
	this->_name = name;
	this->_command = setCommand(command);
	this->_args = setArgs(command);
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
	this->_processes = std::vector<Process>();
}

Program::~Program() {
}

char **Program::setArgs(std::string rawCommand) {
	std::string arg;
	std::vector<std::string> args;
	size_t pos = 0;
	while ((pos = rawCommand.find(" ")) != std::string::npos) {
		arg = rawCommand.substr(0, pos);
		args.push_back(arg);
		rawCommand.erase(0, pos + 1);
	}
	args.push_back(rawCommand);
	char **argv = new char *[args.size() + 1];
	for (size_t i = 0; i < args.size(); ++i) {
		argv[i] = new char[args[i].length() + 1];
		strcpy(argv[i], args[i].c_str());
	}
	argv[args.size()] = nullptr;
	return argv;
}

std::string Program::setCommand(std::string rawCommand) {
	std::string command = rawCommand;
	size_t pos = command.find(" ");
	if (pos != std::string::npos) {
		this->_command = command.substr(0, pos);
		command.erase(0, pos + 1);
	} else {
		this->_command = command;
		command.clear();
	}
	this->_args = setArgs(command);
	return this->_command;
}

void Program::setName(const std::string &name) {
	this->_name = name;
}
std::string Program::getName() const {
	return this->_name;
}
std::string Program::getCommand() const {
	return this->_command;
}
void Program::setWorkdir(const std::string &workdir) {
	this->_workdir = workdir;
}
std::string Program::getWorkdir() const {
	return this->_workdir;
}
void Program::setNbprocess(int nbprocess) {
	this->_nbprocess = nbprocess;
	this->addProcess();
}
int Program::getNbprocess() const {
	return this->_nbprocess;
}
void Program::setAutostart(bool autostart) {
	this->_autostart = autostart;
}
bool Program::getAutostart() const {
	return this->_autostart;
}
void Program::setRestart(Restart restart) {
	this->_restart = restart;
}
Program::Restart Program::getRestart() const {
	return this->_restart;
}
void Program::setExitcodes(std::vector<int> exitcodes) {
	this->_exitcodes = exitcodes;
}
std::vector<int> Program::getExitcodes() const {
	return this->_exitcodes;
}
void Program::setStartdelay(int startdelay) {
	this->_startdelay = startdelay;
}
int Program::getStartdelay() const {
	return this->_startdelay;
}
void Program::setRestartretry(int restartretry) {
	this->_restartretry = restartretry;
}
int Program::getRestartretry() const {
	return this->_restartretry;
}
void Program::setStopsignal(int stopsignal) {
	this->_stopsignal = stopsignal;
}
void Program::setStopsignalString(const std::string &stopsignal) {
	this->_stopsignal = convertStringToStopsignal(stopsignal);
}
int Program::getStopsignal() const {
	return this->_stopsignal;
}
void Program::setStoptimeout(int stoptimeout) {
	this->_stoptimeout = stoptimeout;
}
int Program::getStoptimeout() const {
	return this->_stoptimeout;
}
void Program::addEnv(const std::string &key, const std::string &value) {
	this->_env[key] = value;
}
std::map<std::string, std::string> Program::getEnv() const {
	return this->_env;
}
void Program::setStdoutfile(const std::string &stdoutfile) {
	this->_stdoutfile = stdoutfile;
	if (this->_stdoutfile != "AUTO") {
		Log log = Log(this->_name, Log::Type::FILE, Log::LogLevel::INFO, this->_stdoutfile);
		this->_logs.push_back(log);
	}
}
void Program::setStderrfile(const std::string &stderrfile) {
	this->_stderrfile = stderrfile;
	if (this->_stderrfile != "AUTO") {
		Log log = Log(this->_name, Log::Type::FILE, Log::LogLevel::ERR, this->_stderrfile);
		this->_logs.push_back(log);
	}
}
void Program::setUmask(mode_t umask) {
	this->_umask = umask;
}
mode_t Program::getUmask() const {
	return this->_umask;
}

std::string Program::getStates() {
	std::string states;
	for (const auto &process : this->_processes) {
		states += process.getName() + ": " + convertStateToString(process.getState()) + "\n";
	}
	return states;
}

void Program::addProcess() {
	for (int i = 0; i < this->_nbprocess; ++i) {
		Process process(this->_name + "_" + std::to_string(i));
		this->_processes.push_back(process);
	}
}

Process &Program::getProcess(std::string name) {
	for (auto &process : this->_processes) {
		if (process.getName() == name) {
			return process;
		}
	}
	throw std::runtime_error("Process " + name + " not found in program " + this->_name);
}

void Program::doLog(const std::string &message, Log::LogLevel level) {
	std::string full_message = "[" + this->_name + "] " + message;
	for (auto &log : this->_logs) {
		if (convertLogLevelToSyslog(log.getLogLevel()) >= convertLogLevelToSyslog(level)) {
			log.doLog(full_message);
		}
	}
}

std::string Program::convertRestartToString(Restart restart) {
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

std::string Program::convertStopsignalToString(int signal) {
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

void Program::start(std::string name_process) {
	std::vector<Process> processes_to_start;
	if (!name_process.empty()) {
		try {
			processes_to_start.push_back(this->getProcess(name_process));
		} catch (const std::runtime_error &e) {
			this->doLog(e.what(), Log::LogLevel::ERR);
			return;
		}
		this->doLog("Starting process " + name_process, Log::LogLevel::INFO);
	} else {
		this->doLog("Starting all processes", Log::LogLevel::INFO);
		processes_to_start = this->_processes;
	}
	std::cout << "Number of processes to start: " << processes_to_start.size() << std::endl;
	for (auto &process : processes_to_start) {
		if (process.getState() == Process::State::STOPPED || process.getState() == Process::State::EXITED) {
			try {
				this->doLog("Starting process " + process.getName(), Log::LogLevel::INFO);
				process.start(this->_umask, this->_command, this->_workdir, this->_stdoutfile, this->_stderrfile, this->_env, this->_args);
				process.setState(Process::State::RUNNING);
				this->doLog("Process " + process.getName() + " started successfully", Log::LogLevel::INFO);
			} catch (const std::exception &e) {
				this->doLog("Error starting process " + process.getName() + ": " + e.what(), Log::LogLevel::ERR);
				process.setState(Process::State::FATAL);
				continue;
			}
		} else {
			this->doLog("Process " + process.getName() + " is already running or starting", Log::LogLevel::WARNING);
		}
	}
}

void Program::stop(std::string name_process) {
	std::vector<Process> processes_to_stop;
	if (!name_process.empty()) {
		try {
			processes_to_stop.push_back(this->getProcess(name_process));
		} catch (const std::runtime_error &e) {
			this->doLog(e.what(), Log::LogLevel::ERR);
			return;
		}
	} else {
		this->doLog("Stopping all processes", Log::LogLevel::INFO);
		processes_to_stop = this->_processes;
	}
	for (auto &process : processes_to_stop) {
		try {
			this->doLog("Stopping process " + process.getName(), Log::LogLevel::INFO);
			process.stop(this->_stopsignal, this->_stoptimeout);
			this->doLog("Process " + process.getName() + " stopped successfully", Log::LogLevel::INFO);
		} catch (const std::exception &e) {
			this->doLog("Error stopping process " + process.getName() + ": " + e.what(), Log::LogLevel::ERR);
			continue;
		}
	}
}

Program::Restart convertStringToRestart(const std::string &str) {
	if (str == "true") {
		return Program::Restart::TRUE;
	} else if (str == "unexpected") {
		return Program::Restart::UNEXPECTED;
	} else if (str == "false") {
		return Program::Restart::FALSE;
	} else {
		throw std::invalid_argument("Invalid restart option");
	}
}

void Program::printProgram() {
	std::cout << "Program name: " << this->_name << std::endl;
	std::cout << "Program command: " << this->_command << std::endl;
	std::cout << "Program workdir: " << this->_workdir << std::endl;
	std::cout << "Program nbprocess: " << this->_nbprocess << std::endl;
	std::cout << "Program autostart: " << (this->_autostart ? "true" : "false") << std::endl;
	std::cout << "Program restart: " << convertRestartToString(this->_restart) << std::endl;
	std::cout << "Program exitcodes: ";
	for (const auto &exitcode : this->_exitcodes) {
		std::cout << exitcode << " ";
	}
	std::cout << std::endl;
	std::cout << "Program startdelay: " << this->_startdelay << std::endl;
	std::cout << "Program restartretry: " << this->_restartretry << std::endl;
	std::cout << "Program stopsignal: " << convertStopsignalToString(this->_stopsignal) << std::endl;
	std::cout << "Program stoptimeout: " << this->_stoptimeout << std::endl;
	std::cout << "Program stdoutfile: " << this->_stdoutfile << std::endl;
	std::cout << "Program stderrfile: " << this->_stderrfile << std::endl;
	std::cout << "Program umask: " << to_octal_string(this->_umask) << std::endl;
	std::cout << "Program env: " << std::endl;
	for (const auto &pair : this->_env) {
		std::cout << "  " << pair.first << "=" << pair.second << std::endl;
	}
	std::cout << "Logs: " << this->_logs.size() << std::endl;
	for (auto &log : this->_logs) {
		log.printLog();
	}
}

int convertStringToStopsignal(const std::string &str) {
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

std::string to_octal_string(mode_t mode) {
    std::ostringstream oss;
    oss << std::setfill('0') << std::oct << std::setw(3) << (mode & 0777);
    return oss.str();
}
