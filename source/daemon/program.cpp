#include "daemon/program.hpp"

Program::Program(const std::string &name, const std::string &command, const std::string &workdir,
	int nbprocess, bool autostart, Restart restart, std::vector<int> exitcodes,
	int startdelay, int restartretry, int stopsignal, int stoptimeout,
	const std::string &stdoutfile, const std::string &stderrfile,
	mode_t umask, std::map<std::string, std::string> env) {
	this->_name = name;
	this->_pid = -1;
	this->_state = State::STOPPED;
	this->_command = setCommand(command);
	this->_args = setArgs(command);
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

Program::Program(const std::string &name, const std::string &command) {
	this->_name = name;
	this->_pid = -1;
	this->_state = State::STOPPED;
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

void Program::setState(State state) {
	this->_state = state;
}

Program::State Program::getState() const {
	return this->_state;
}
void Program::setName(const std::string &name) {
	this->_name = name;
}
std::string Program::getName() const {
	return this->_name;
}
void Program::setPid(int pid) {
	this->_pid = pid;
}
int Program::getPid() const {
	return this->_pid;
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

void Program::doLog(const std::string &message, Log::LogLevel level) {
	for (auto &log : this->_logs) {
		if (convertLogLevelToSyslog(log.getLogLevel()) >= convertLogLevelToSyslog(level)) {
			log.doLog(message);
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

std::string Program::convertStateToString(State state) {
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

void Program::start() {
	this->setState(State::STARTING);
	this->doLog("Starting program " + this->_name, Log::LogLevel::INFO);
	int pid = fork();
	if (pid == -1) {
		this->doLog("Error forking program " + this->_name, Log::LogLevel::ERR);
		return ;
	} else if (pid == 0) {
		if (this->_umask != (mode_t)-1) {
			umask(this->_umask);
		}
		if (this->_env.size() > 0) {
			for (const auto &pair : this->_env) {
				setenv(pair.first.c_str(), pair.second.c_str(), 1);
			}
		}
		if (this->_stdoutfile != "AUTO") {
			dup2(open(this->_stdoutfile.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0644), STDOUT_FILENO);
		} else {
			printf("Redirecting stdout to /dev/null\n");
			dup2(open("/dev/null", O_WRONLY | O_CREAT | O_APPEND, 0644), STDOUT_FILENO);
		}
		if (this->_stderrfile != "AUTO") {
			dup2(open(this->_stderrfile.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0644), STDERR_FILENO);
		} else {
			dup2(open("/dev/null", O_WRONLY | O_CREAT | O_APPEND, 0644), STDERR_FILENO);
		}
		if (this->_workdir != ".") {
			if (chdir(this->_workdir.c_str()) != 0) {
				this->doLog("Error changing directory to " + this->_workdir, Log::LogLevel::ERR);
				return;
			}
		}
		if (execve(this->_command.c_str(), this->_args, nullptr) == -1) {
			this->doLog("Error executing Program " + this->_name, Log::LogLevel::ERR);
			this->setState(State::FATAL);
			this->setPid(-1);
			exit(1);
		}
	}
	this->_pid = pid;
	this->setState(State::RUNNING);
	this->doLog("Program " + this->_name + " started with PID " + std::to_string(this->_pid), Log::LogLevel::INFO);
}

void Program::stop() {
	this->doLog("Stopping program " + this->_name, Log::LogLevel::INFO);
	if (this->_state != State::RUNNING && this->_state != State::STARTING) {
		this->doLog("Program " + this->_name + " is not running : " + convertStateToString(this->_state), Log::LogLevel::ERR);
		return ;
	}
	if (this->_pid != -1) {
		kill(this->_pid, this->_stopsignal);
		int status;
		waitpid(this->_pid, &status, 0);
		this->doLog("Program " + this->_name + " stopped", Log::LogLevel::INFO);
		if (WIFEXITED(status)) {
			this->doLog("Program " + this->_name + " exited with status " + std::to_string(WEXITSTATUS(status)), Log::LogLevel::INFO);
		} else {
			this->doLog("Program " + this->_name + " terminated abnormally", Log::LogLevel::ERR);
		}
		this->setPid(-1);
	}
	this->setState(State::STOPPED);
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
