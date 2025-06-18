#ifndef PROCESS_HPP
#define PROCESS_HPP

#include "pch.hpp" // IWYU pragma: keep

class Process {
public:
  enum class State {
    STOPPED,
    RUNNING,
    STARTING,
    STOPPING,
    EXITED,
    FATAL,
    UNKNOWN,
    BACKOFF
  };

  Process(std::string name);
  ~Process();
  void setState(State state);
  void setPid(int pid);
  State getState() const;
  int getPid() const;
  std::string getName() const;

  void start(mode_t umask_process, const std::string &workdir,
             const std::string &stdoutfile, const std::string &stderrfile,
             const std::map<std::string, std::string> &env,
             std::string command);
  void stop(int stopsignal, int stoptimeout);

  static std::string convertStateToString(Process::State state);

private:
  std::string _name;
  State _state;
  int _pid;
};

#endif
