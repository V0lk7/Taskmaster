#include "daemon/daemon.hpp"
#include "common/Commands.hpp"
#include "common/Utils.hpp"

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
  for (auto &logger : this->loggers) {
    logger.doLog("Daemon destroyed.");
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

void Daemon::sendLogs(const std::string &message) {
  for (auto &logger : this->loggers) {
    logger.doLog(message);
  }
}

void Daemon::start() {
  this->startAllPrograms();
  std::cout << "Daemon started." << std::endl;
  int ret = 0;
  void *buffer = nullptr;

  while (true) {
    // update program first
    // check message received from socket
    ret = nn_poll(&(this->pfd), 1, TIMEOUT);

    if (ret == -1) {
      // error case
      clean();
    } else if (ret == 0) {
      // timed out, on continue
      continue;
    }
    if (this->pfd.revents & NN_POLLIN) {
      for (int i = 0; i < TRY_RCV; i++) {
        int bytes = nn_recv(this->socketFd, &buffer, NN_MSG, 0);

        if (bytes > 0) {

          std::string message(static_cast<char *>(buffer), bytes);
          nn_freemsg(buffer);
          buffer = nullptr;
          this->processMessage(message);
          break;
        }
      }
    }
  }
}

void Daemon::processMessage(std::string const &message) {
  std::vector<std::string> keys = Utils::split(message, " ");

  std::cout << "Key[0] = |" << keys[0] << "|" << std::endl;
  std::cout << "ping = |" << Commands::PING << "|" << std::endl;
  std::cout << "wtf = " << keys[0].compare(0, keys[0].size(), Commands::PING)
            << std::endl;
  for (size_t i = 0; i < keys[0].size(); ++i) {
    printf("[%02X] ", static_cast<unsigned char>(keys[0][i]));
  }
  printf("\n");
  if (keys[0] == "ping") {
    std::cout << "Command \"PING\" received: sending PONG" << std::endl;
    std::string snd(Commands::PONG);
    if (nn_send(this->socketFd, snd.c_str(), snd.size(), 0) < 1) {
      std::cout << "Et merde" << std::endl;
    }
  } else if (keys[0].compare(0, keys[0].size(), Commands::START) == 0) {
    std::cout << "Command \"START\" received" << std::endl;
  } else if (keys[0].compare(0, keys[0].size(), Commands::STOP) == 0) {
    std::cout << "Command \"STOP\" received" << std::endl;
  } else if (keys[0].compare(0, keys[0].size(), Commands::STATUS) == 0) {
    std::cout << "Command \"STATUS\" received" << std::endl;
  } else if (keys[0].compare(0, keys[0].size(), Commands::RESTART) == 0) {
    std::cout << "Command \"RESTART\" received" << std::endl;
  } else if (keys[0].compare(0, keys[0].size(), Commands::RELOAD) == 0) {
    std::cout << "Command \"RELOAD\" received" << std::endl;
  } else {
    std::cout << "Unknown message : |" << message << "|" << std::endl;
  }
}

void Daemon::startAllPrograms() {
  for (auto &program : this->programs) {
    if (program.getAutostart()) {
      std::cout << "Program " << program.getName() << " will be started."
                << std::endl;
      program.start();
    }
  }
  this->sendLogs("All processes started.");
}

void Daemon::stopAllPrograms() {
  for (auto &program : this->programs) {
    program.stop();
    this->sendLogs("Process " + program.getName() + " stopped.");
  }
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

std::string Daemon::stringStatusProgram(std::string name) {
  for (auto &program : this->programs) {
    if (program.getName() == name) {
      return "Process " + program.getName() + " is " +
             program.convertStateToString(program.getState());
    }
  }
  throw std::runtime_error("Process " + name + " not found.");
}

std::string Daemon::stringStatusAllPrograms() {
  std::string status = "Processes status:\n";
  for (auto &program : this->programs) {
    status += "Process " + program.getName() + " is " +
              program.convertStateToString(program.getState()) + "\n";
  }
  return status;
}

std::vector<Program> Daemon::getPrograms() { return this->programs; }

void Daemon::clean() { stopAllPrograms(); }
