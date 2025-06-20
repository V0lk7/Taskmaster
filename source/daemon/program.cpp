#include "daemon/program.hpp"

Program::Program(const std::string &name, const std::string &command) {
	this->_name = name;
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
	this->_processes = std::vector<Process>();
}

Program::~Program() {
}

bool Program::operator==(const Program &other) const {
	return this->_name == other._name && this->_command == other._command &&
	       this->_workdir == other._workdir && this->_nbprocess == other._nbprocess &&
	       this->_autostart == other._autostart && this->_restart == other._restart &&
	       this->_exitcodes == other._exitcodes && this->_startdelay == other._startdelay &&
	       this->_restartretry == other._restartretry && this->_stopsignal == other._stopsignal &&
	       this->_stoptimeout == other._stoptimeout && this->_stdoutfile == other._stdoutfile &&
	       this->_stderrfile == other._stderrfile && this->_umask == other._umask &&
	       this->_env == other._env;
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
}
void Program::setStderrfile(const std::string &stderrfile) {
	this->_stderrfile = stderrfile;
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

void Program::addLog(const Log &log) {
	this->_logs.push_back(log);
}

void Program::doLog(const std::string &message, Log::LogLevel level, std::string name_process) {
	time_t now = time(nullptr);
	std::tm *ltm = std::localtime(&now);
	char time_buf[20];
	std::strftime(time_buf, sizeof(time_buf), "%d:%m:%Y %H:%M:%S", ltm);
	std::string time_str(time_buf);
	if (name_process.empty()) {
		name_process = "main";
	}
	std::string full_message = time_str + " - " + convertLogLeveltoString(level) + " - [" + this->_name + ":" + name_process + "] " + message;
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
    if (!name_process.empty()) {
        try {
            Process &process = this->getProcess(name_process);
            if (process.getState() == Process::State::STOPPED || process.getState() == Process::State::EXITED) {
                this->doLog("Starting process", Log::LogLevel::INFO, process.getName());
                process.start(this->_umask, this->_workdir, this->_stdoutfile, this->_stderrfile, this->_env, this->_command);
                std::cout << "Process " << process.getName() << " started with PID " << process.getPid() << std::endl;
            } else {
                this->doLog("Process is already running or starting", Log::LogLevel::WARNING, process.getName());
            }
        } catch (const std::runtime_error &e) {
            this->doLog(e.what(), Log::LogLevel::ERR, name_process);
            return;
        }
    } else {
        this->doLog("Starting all processes", Log::LogLevel::INFO, "");
        for (Process &process : this->_processes) {
            if (process.getState() == Process::State::STOPPED || process.getState() == Process::State::EXITED) {
                try {
                    this->doLog("Starting process", Log::LogLevel::INFO, process.getName());
                    process.start(this->_umask, this->_workdir, this->_stdoutfile, this->_stderrfile, this->_env, this->_command);
                } catch (const std::exception &e) {
                    this->doLog(std::string("Error starting process: ") + e.what(), Log::LogLevel::ERR, process.getName());
                    process.setState(Process::State::FATAL);
                    continue;
                }
            } else {
                this->doLog("Process is already running or starting", Log::LogLevel::WARNING, process.getName());
            }
        }
    }
}

void Program::stop(std::string name_process) {
	if (!name_process.empty()) {
		try {
			Process &process = this->getProcess(name_process);
			if (process.getPid() == -1) {
				this->doLog("Process is not running", Log::LogLevel::WARNING, name_process);
				return;
			}
			this->doLog("Stopping process", Log::LogLevel::INFO, process.getName());
			process.stop(this->_stopsignal);
		} catch (const std::runtime_error &e) {
			this->doLog(e.what(), Log::LogLevel::ERR, name_process);
			return;
		}
	} else {
		this->doLog("Stopping all processes", Log::LogLevel::INFO, "");
		for (auto &process : this->_processes) {
			try {
				if (process.getPid() == -1) {
					continue;
				}
				this->doLog("Stopping process", Log::LogLevel::INFO, process.getName());
				process.stop(this->_stopsignal);
			} catch (const std::exception &e) {
				this->doLog(std::string("Error stopping process: ") + e.what(), Log::LogLevel::ERR, process.getName());
				continue;
			}
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

void Program::superviseProcesses() {
	if (this->_processes.empty()) {
		this->doLog("No processes to supervise", Log::LogLevel::WARNING, "");
		return;
	}
	bool unexpected_exit;
	pid_t pid;
	int status;
	for (auto &process : this->_processes) {
		unexpected_exit = false;
		if (process.getPid() == -1) {
			continue;
		}
		pid = waitpid(process.getPid(), &status, WNOHANG);
		if (pid == -1) {
			// Error occurred while waiting for the process
			this->doLog("Error waiting for process " + process.getName() + ": " + std::strerror(errno), Log::LogLevel::ERR, process.getName());
			continue;
		} else if (pid == 0) {
			// Process is still running
			if (process.getState() == Process::State::STARTING) {
				if (!process.diffTime(this->_startdelay)) {
					process.setState(Process::State::RUNNING);
					process.setTime();
					this->doLog("Process " + process.getName() + " is now RUNNING", Log::LogLevel::INFO, process.getName());
				}
			} else if (process.getState() == Process::State::STOPPING) {
				// Process is stopping, check if it has stopped
				if (!process.diffTime(this->_stoptimeout)) {
					this->doLog("Process " + process.getName() + " did not stop in time, sending KILL signal", Log::LogLevel::WARNING, process.getName());
					kill(process.getPid(), SIGKILL);
				}
			}
		} else {
			// Process has exited
			int exit_status = WEXITSTATUS(status);
			if (process.getState() == Process::State::STARTING) {
				process.setInfoMsg("Exited too quickly (process log may have details)");
			}
			if (std::find(this->_exitcodes.begin(), this->_exitcodes.end(), exit_status) != this->_exitcodes.end()) {
				this->doLog("Process " + process.getName() + " exited with expected code: " + std::to_string(exit_status), Log::LogLevel::NOTICE, process.getName());
			} else {
				this->doLog("Process " + process.getName() + " exited with unexpected code: " + std::to_string(exit_status), Log::LogLevel::ERR, process.getName());
				unexpected_exit = true;
			}
			if (this->_restart == Restart::TRUE || (this->_restart == Restart::UNEXPECTED && unexpected_exit)) {
				if (process.getNbRestart() < this->_restartretry) {
					process.incrementNbRestart();
					this->doLog("Restarting process " + process.getName() + ", attempt: " + std::to_string(process.getNbRestart()) + " of " + std::to_string(this->_restartretry), Log::LogLevel::INFO, process.getName());
					process.start(this->_umask, this->_workdir, this->_stdoutfile, this->_stderrfile, this->_env, this->_command);
				} else {
					this->doLog("Process " + process.getName() + " exceeded restart retry limit", Log::LogLevel::ERR, process.getName());
					process.setPid(-1);
					process.setState(Process::State::FATAL);
				}
			} else {
				process.setState(Process::State::EXITED);
				process.setPid(-1);
				process.setInfoMsgFormattedTime();
				this->doLog("Process " + process.getName() + " has exited and will not be restarted", Log::LogLevel::NOTICE, process.getName());
			}
		}
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

