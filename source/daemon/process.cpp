#include "daemon/process.hpp"

Process::Process(std::string name) : _name(name) {
  this->_state = State::STOPPED;
  this->_pid = -1;
  this->_time = 0;
  this->_infoMsg = "Not started";
  this->_nbRestart = 0;
}

Process::~Process() {}

void Process::setState(State state) { this->_state = state; }

Process::State Process::getState() const { return this->_state; }

void Process::setPid(int pid) { this->_pid = pid; }

int Process::getPid() const { return this->_pid; }

void Process::setTime() { this->_time = std::time(nullptr); }

time_t Process::getTime() const { return this->_time; }

std::string Process::getName() const { return this->_name; }

void Process::setInfoMsg(const std::string &infoMsg) {
  this->_infoMsg = infoMsg;
}

std::string Process::getInfoMsg() const { return this->_infoMsg; }

void Process::incrementNbRestart() { this->_nbRestart++; }

int Process::getNbRestart() const { return this->_nbRestart; }

void Process::start(mode_t umask_process, const std::string &workdir, const std::string &stdoutfile, const std::string &stderrfile, const std::map<std::string, std::string> &env, std::string command) {
	this->setState(State::STARTING);
	this->setTime();
	int pid = fork();
	if (pid == -1) {
		throw std::runtime_error("Error forking process: " + std::string(strerror(errno)));
		return ;
	} else if (pid == 0) {
		char *completeCommand[4];
		completeCommand[0] = const_cast<char*>("/bin/sh");
		completeCommand[1] = const_cast<char*>("-c");
		completeCommand[2] = const_cast<char*>(command.c_str());
		completeCommand[3] = nullptr;
		if (umask_process != (mode_t)-1) {
			umask(umask_process);
		}
		if (env.size() > 0) {
			for (const auto &pair : env) {
				setenv(pair.first.c_str(), pair.second.c_str(), 1);
			}
		}
		if (stdoutfile != "AUTO") {
			dup2(open(stdoutfile.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0644), STDOUT_FILENO);
		} else {
			dup2(open("/dev/null", O_WRONLY | O_CREAT | O_APPEND, 0644), STDOUT_FILENO);
		}
		if (stderrfile != "AUTO") {
			dup2(open(stderrfile.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0644), STDERR_FILENO);
		} else {
			dup2(open("/dev/null", O_WRONLY | O_CREAT | O_APPEND, 0644), STDERR_FILENO);
		}
		if (workdir != ".") {
			if (chdir(workdir.c_str()) != 0) {
				throw std::runtime_error("Error changing directory to " + workdir + ": " + std::string(strerror(errno)));
				return;
			}
		}
		if (execve("/bin/sh", completeCommand, environ) == -1) {
			throw std::runtime_error("Error executing process: " + std::string(strerror(errno)));
		}
	}
	this->setPid(pid);
	this->setInfoMsg("Process started with PID " + std::to_string(pid));
}

void Process::stop(int stopsignal) {
  if (this->_state != State::RUNNING && this->_state != State::STARTING) {
    throw std::runtime_error("Process is not running or starting.");
  }
  this->setTime();
  this->setState(State::STOPPING);
  if (kill(this->_pid, stopsignal) == -1) {
    throw std::runtime_error("Error sending stop signal to process: " +
                             std::string(strerror(errno)));
  }
}

bool Process::diffTime(int deltaMax) {
  time_t currentTime = std::time(nullptr);
  double delta = std::difftime(currentTime, this->_time);
  return (delta <= static_cast<double>(deltaMax));
}

std::string Process::convertStateToString(Process::State state) {
	switch (state) {
		case Process::State::STOPPED:
			return "STOPPED";
		case Process::State::RUNNING:
			return "RUNNING";
		case Process::State::STARTING:
			return "STARTING";
		case Process::State::STOPPING:
			return "STOPPING";
		case Process::State::EXITED:
			return "EXITED";
		case Process::State::FATAL:
			return "FATAL";
		default:
			throw std::invalid_argument("Invalid state");
	}
}

void Process::setInfoMsgFormattedTime() {
  std::time_t t = std::time(nullptr);
  std::tm local_tm;
  localtime_r(&t, &local_tm);

  static const char *months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                                 "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

  int hour = local_tm.tm_hour;
  std::string am_pm = "AM";
  if (hour == 0) {
    hour = 12;
  } else if (hour == 12) {
    am_pm = "PM";
  } else if (hour > 12) {
    hour -= 12;
    am_pm = "PM";
  }

  std::ostringstream oss;
  oss << months[local_tm.tm_mon] << ' ' << local_tm.tm_mday << ' '
      << std::setfill('0') << std::setw(2) << hour << ':' << std::setfill('0')
      << std::setw(2) << local_tm.tm_min << ' ' << am_pm;

  this->_infoMsg = oss.str();
}
