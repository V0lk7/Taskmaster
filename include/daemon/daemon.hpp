#ifndef DAEMON_HPP
#define DAEMON_HPP

#include "pch.hpp" // IWYU pragma: keep

#include "log.hpp"
#include "program.hpp"

class Daemon {
public:
  Daemon(std::string socketPath, Log logInfo, std::string confPath);
  ~Daemon();

  static constexpr char IPC[] = "ipc://";

  void setIpc();
  void checkSocketFile();
  void formatSocketFileName();
  void cleanSocket();

  void start();
  void stop();

  void startAllPrograms(); // initial start

  void stopAllPrograms();

  void addProgram(Program &program);
  void removeProgram(Program &program);

  void signalReceived();

  void setSocketPath(std::string socketPath);
  std::string getSocketPath() const;

  int getSocketFd() const;
  void setSocketFd(int socketFd);

  void sendLogs(const std::string &message, std::string log_levelmsg);
  std::vector<Log> getLogs() const;
  std::string stringStatusProgram(std::string name);
  std::string stringStatusAllPrograms();
  std::vector<Program> getPrograms();
  Program &getProgram(std::string name);
  std::string getConfPath() const;
  void updateLoggers(Log &log);

  void supervisePrograms();

private:
  std::string confPath;
  std::string socketPath;
  int socketFd;
  int sockEndPoint;
  struct nn_pollfd pfd;

  static constexpr int TIMEOUT = 10;
  static constexpr char UNIX[] = "unix://";

  std::vector<Log> loggers;
  std::vector<Program> programs;

  bool listenClients();
  void processMessage(std::string const);

  void cmdStart(std::string, std::string &);
  void cmdStop(std::string, std::string &);
  void cmdReload(std::string &);
  void cmdStatus(std::string, std::string &);
};

#endif
