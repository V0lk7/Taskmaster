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

  static std::string convertStateToString(Process::State state);
  Process(std::string name);
  ~Process();
  void setState(State state);
  void setPid(int pid);
  void setTime();
  void setInfoMsg(const std::string &infoMsg);
  State getState() const;
  int getPid() const;
  std::string getName() const;
  time_t getTime() const;
  std::string getInfoMsg() const;
  bool diffTime(int deltaMax);
  void incrementNbRestart();
  int getNbRestart() const;

  std::string formatUptime() const;

  void start(mode_t umask_process, const std::string &workdir,
             const std::string &stdoutfile, const std::string &stderrfile,
             const std::map<std::string, std::string> &env,
             std::string command);
  void stop(int stopsignal);

  void setInfoMsgFormattedTime();

private:
  std::string _name;
  State _state;
  int _pid;
  time_t _time;
  std::string _infoMsg;
  int _nbRestart;
};

#endif
