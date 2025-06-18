#include "daemon/daemon.hpp"
#include "common/Commands.hpp"
#include "common/Utils.hpp"
#include "nanomsg/nn.h"

constexpr char Daemon::IPC[];
constexpr char Daemon::UNIX[];

Daemon::Daemon(std::string socketPath, Log logInfo) {
  this->socketPath = socketPath;
  this->socketFd = -1;
  this->sockEndPoint = -1;
  this->loggers.push_back(logInfo);
  Log syslogTaskmasterd("taskmasterd", Log::Type::SYSLOG, logInfo.getLogLevel(),
                        this->socketPath);
  this->loggers.push_back(syslogTaskmasterd);

  this->setIpc();
}

Daemon::~Daemon() {
  this->stopAllPrograms();
  if (this->sockEndPoint > -1) {
    (void)nn_shutdown(this->socketFd, 0);
  }
  if (this->socketFd != -1) {
    (void)nn_close(this->socketFd);
  }
  if (!this->socketPath.empty()) {
    (void)unlink(this->socketPath.c_str());
  }
}

void Daemon::setIpc() {
  checkSocketFile();
  // Initialize the socket
  this->socketFd = nn_socket(AF_SP, NN_REP);
  if (this->socketFd == -1) {
    this->loggers[0].doLog("Error creating socket: " +
                           std::string(strerror(errno)));
    throw std::runtime_error("Error creating socket: " +
                             std::string(strerror(errno)));
  }

  this->formatSocketFileName();

  this->sockEndPoint = nn_bind(this->socketFd, this->socketPath.c_str());

  if (this->sockEndPoint == -1) {
    loggers[0].doLog("Error binding socket: " + std::string(strerror(errno)));
    throw std::runtime_error("Error binding socket: " +
                             std::string(strerror(errno)));
  }

  this->pfd.fd = this->socketFd;
  this->pfd.events = NN_POLLIN | NN_POLLOUT;
}

void Daemon::formatSocketFileName() {
  const std::string ipc(IPC);
  const std::string _unix(UNIX);

  if (this->socketPath.compare(0, ipc.size(), ipc) == 0) {
    return;
  }
  if (this->socketPath.compare(0, _unix.size(), _unix) == 0) {
    this->socketPath = ipc + this->socketPath.substr(_unix.size());
    return;
  }
  this->socketPath = ipc + this->socketPath;
}

void Daemon::checkSocketFile() {
  if (this->socketPath.empty()) {
    throw std::runtime_error("section [unix_http_server] has no file value");
    return;
  }
  // Verify if the socket file already exists
  if (access(this->socketPath.c_str(), F_OK) == 0) {
    if (unlink(this->socketPath.c_str()) == -1) {
      this->loggers[0].doLog("Error unlinking socket: " +
                             std::string(strerror(errno)));
      close(this->socketFd);
      throw std::runtime_error("Unlinking socket failed: " +
                               std::string(strerror(errno)));
    }
    this->loggers[0].doLog("Socket file already exists, unlinking done.");
  }
}

void Daemon::setSocketPath(std::string socketPath) {
  this->socketPath = socketPath;
}
std::string Daemon::getSocketPath() const { return this->socketPath; }
int Daemon::getSocketFd() const { return this->socketFd; }
void Daemon::setSocketFd(int socketFd) { this->socketFd = socketFd; }
std::vector<Log> Daemon::getLogs() const { return this->loggers; }

void Daemon::sendLogs(const std::string &message, std::string log_levelmsg) {
  if (log_levelmsg.empty()) {
    log_levelmsg = "INFO";
  }

  time_t now = time(nullptr);
  std::tm *ltm = std::localtime(&now);
  char time_buf[20];
  std::strftime(time_buf, sizeof(time_buf), "%d:%m:%Y %H:%M:%S", ltm);
  std::string time_str(time_buf);

  std::string new_message = time_str + " - [taskmasterd] " + message;
  Log::LogLevel logLevel = convertStringToLogLevel(log_levelmsg);

  for (auto &logger : this->loggers) {
    if (convertLogLevelToSyslog(logger.getLogLevel()) >=
        convertLogLevelToSyslog(logLevel)) {
      logger.doLog(new_message);
    }
  }
}

void Daemon::stopAllPrograms() {
  for (auto &program : this->programs) {
    program.stop("");
    this->sendLogs("Program " + program.getName() + " stopped.", "INFO");
  }
}

void Daemon::start() {
  this->startAllPrograms();
  sendLogs("Daemon started successfully.", "INFO");

  while (true) {
    // update program first
    // check message received from socket
    if (!this->listenClients()) {
      clean();
      return;
    }
  }
}

bool Daemon::listenClients() {
  int ret = nn_poll(&(this->pfd), 1, TIMEOUT);

  if (ret == -1) {
    // error case
    return false;
  } else if (ret == 0) {
    // timed out, we continue
    return true;
  }
  if (this->pfd.revents & NN_POLLIN) {
    void *buffer = nullptr;
    int bytes = nn_recv(this->socketFd, &buffer, NN_MSG, 0);

    if (bytes >= 0 && buffer != nullptr) {
      std::string message(static_cast<char *>(buffer), bytes);
      nn_freemsg(buffer);
      buffer = nullptr;
      this->processMessage(message);
    }
  }
  return true;
}

void Daemon::processMessage(std::string const message) {
  std::vector<std::string> keys = Utils::split(message, " ");
  std::string answer;

  if (keys[0] == "ping") {
    std::cout << "Command \"PING\" received" << std::endl;
    answer = Commands::PONG;
  } else if (keys[0] == Commands::START) {
    std::cout << "Command \"START\" received" << std::endl;
    answer = "OK";
  } else if (keys[0] == Commands::STOP) {
    std::cout << "Command \"STOP\" received" << std::endl;
    answer = "OK";
  } else if (keys[0] == Commands::STATUS) {
    for (auto &program : this->programs) {
      std::vector<std::string> status = program.getStatusProcesses();

      answer += Utils::concat(status, "\n");
      answer += "\n";
    }
  } else if (keys[0] == Commands::RELOAD) {
    std::cout << "Command \"RELOAD\" received" << std::endl;
    answer = "OK";
  } else {
    std::cout << "Unknown message : |" << message << "|" << std::endl;
    answer = "KO";
  }
  if (nn_send(this->socketFd, answer.c_str(), answer.size(), 0) < 1) {
    std::cout << "Et merde" << std::endl;
  }
}

std::string Daemon::stringStatusProgram(std::string name) {
  for (auto &program : this->programs) {
    if (program.getName() == name) {
      return "Program " + program.getName() + " is : \n" + program.getStates();
    }
  }
  throw std::runtime_error("Process " + name + " not found.");
}

std::string Daemon::stringStatusAllPrograms() {
  std::string status = "Programs status:\n";
  for (auto &program : this->programs) {
    status += "Program " + program.getName() + " is : \n" +
              program.getStates() + "\n";
  }
  return status;
}

void Daemon::startAllPrograms() {
  for (auto &program : this->programs) {
    if (program.getAutostart()) {
      program.start("");
    }
  }
  this->sendLogs("All programs started.", "INFO");
}

void Daemon::addProgram(Program &program) { this->programs.push_back(program); }

void Daemon::removeProgram(Program &program) {
  for (auto it = this->programs.begin(); it != this->programs.end(); ++it) {
    if (it->getName() == program.getName()) {
      this->programs.erase(it);
      break;
    }
  }
}

void Daemon::printDaemon() {
  std::cout << "Daemon socket path: " << this->socketPath << std::endl;
  std::cout << "Daemon socket fd: " << this->socketFd << std::endl;
  std::cout << "Daemon processes:" << std::endl;
  for (auto &process : this->programs) {
    std::cout << "============================" << std::endl;
    process.printProgram();
  }
}

std::vector<Program> Daemon::getPrograms() { return this->programs; }

void Daemon::clean() { stopAllPrograms(); }
