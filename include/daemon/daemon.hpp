#ifndef DAEMON_HPP
#define DAEMON_HPP

#include "pch.hpp" // IWYU pragma: keep

#include "log.hpp"
#include "program.hpp"

class Daemon {
public:
  Daemon(std::string socketPath, Log logInfo);
  ~Daemon();

  void start();
  void stop();
  void clean();

  void startAllPrograms();
  void startProgram(std::string const &);
  void startProgram(std::vector<std::string> const &);

  //  void stopAllPrograms();
  void stopProgram(std::string);
  void stopProgram(std::vector<std::string> const &);

  void signalReceived();

  // void initialStart();    // -> become startAllPrograms()
  void stopAllPrograms(); // -> become stop()

  void setSocketPath(std::string socketPath);
  std::string getSocketPath() const;

  int getSocketFd() const;
  void setSocketFd(int socketFd);

  void sendLogs(const std::string &message);

  void addProgram(Program &program);
  void removeProgram(Program &program);
  void printDaemon();
  std::string stringStatusProgram(std::string name);
  std::string stringStatusAllPrograms();
  std::vector<Program> getPrograms();

private:
  std::string socketPath;
  int socketFd;
  int sockEndPoint;
  struct nn_pollfd pfd;

  static constexpr int TIMEOUT = 10;
  static constexpr int TRY_RCV = 3;
  static constexpr char IPC[] = "ipc://";
  static constexpr char UNIX[] = "unix://";

  std::vector<Log> loggers;
  std::vector<Program> programs;

  void setIpc();
  void checkSocketFile();
  void formatSocketFileName();

  void processMessage(std::string const &);
};

#endif
