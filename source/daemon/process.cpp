#include "daemon/process.hpp"

Process::Process(std::string name) : _name(name) {
  this->_state = State::STOPPED;
  this->_pid = -1;
}

Process::~Process() {}

void Process::setState(State state) { this->_state = state; }

Process::State Process::getState() const { return this->_state; }

void Process::setPid(int pid) { this->_pid = pid; }

int Process::getPid() const { return this->_pid; }

std::string Process::getName() const { return this->_name; }

void Process::start(mode_t umask_process, const std::string &workdir,
                    const std::string &stdoutfile,
                    const std::string &stderrfile,
                    const std::map<std::string, std::string> &env,
                    std::string command) {
  this->setState(State::STARTING);
  int pid = fork();
  char *completeCommand[4];
  completeCommand[0] = const_cast<char *>("/bin/sh");
  completeCommand[1] = const_cast<char *>("-c");
  completeCommand[2] = const_cast<char *>(command.c_str());
  completeCommand[3] = nullptr;
  if (pid == -1) {
    throw std::runtime_error("Error forking process: " +
                             std::string(strerror(errno)));
    return;
  } else if (pid == 0) {
    if (umask_process != (mode_t)-1) {
      umask(umask_process);
    }
    if (env.size() > 0) {
      for (const auto &pair : env) {
        setenv(pair.first.c_str(), pair.second.c_str(), 1);
      }
    }
    if (stdoutfile != "AUTO") {
      dup2(open(stdoutfile.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0644),
           STDOUT_FILENO);
    } else {
      dup2(open("/dev/null", O_WRONLY | O_CREAT | O_APPEND, 0644),
           STDOUT_FILENO);
    }
    if (stderrfile != "AUTO") {
      dup2(open(stderrfile.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0644),
           STDERR_FILENO);
    } else {
      dup2(open("/dev/null", O_WRONLY | O_CREAT | O_APPEND, 0644),
           STDERR_FILENO);
    }
    if (workdir != ".") {
      if (chdir(workdir.c_str()) != 0) {
        throw std::runtime_error("Error changing directory to " + workdir +
                                 ": " + std::string(strerror(errno)));
        return;
      }
    }
    if (execve("/bin/sh", completeCommand, environ) == -1) {
      throw std::runtime_error("Error executing process: " +
                               std::string(strerror(errno)));
    }
  }
}

void Process::stop(int stopsignal, int stoptimeout) {
  (void)stoptimeout;
  if (this->_state != State::RUNNING && this->_state != State::STARTING) {
    throw std::runtime_error("Process is not running or starting.");
  }
  if (kill(this->_pid, stopsignal) == -1) {
    throw std::runtime_error("Error sending stop signal to process: " +
                             std::string(strerror(errno)));
  }
  this->_pid = -1; // Reset PID to indicate process is stopped
  this->setState(State::STOPPED);
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
  default:
    throw std::invalid_argument("Invalid state");
  }
}
