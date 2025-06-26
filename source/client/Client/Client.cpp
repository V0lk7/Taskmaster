#include "Client/Client.hpp"
#include "Client/ProcessInfo.hpp"

#include "common/Commands.hpp"
#include "common/Utils.hpp"
#include "pch.hpp" // IWYU pragma: keep

Client &Client::Instance() {
  static Client _instance;

  return _instance;
}

Client::~Client() {}

bool Client::setupClient(std::string const &conf) {
  bool err = false;
  std::string socket = extractSocket(conf, err);

  if (err) {
    return false;
  }

  _request.setSockFile(socket);

  _request.setAddFdToEpoll([this]() {
    int rcvFd = _request.getrcvFd();
    if (rcvFd <= 0) {
      logError("setupClient() - Request socket not connected.");
      return;
    }
    if (!_epoll.addFd(rcvFd, false)) {
      logError("setupClient() - Failed to add request socket to epoll.");
    }
  });
  _request.setRemoveFdFromEpoll([this]() {
    int rcvFd = _request.getrcvFd();
    if (rcvFd <= 0) {
      return;
    }
    if (!_epoll.removeFd(rcvFd)) {
      logError("setupClient() - Failed to remove request socket from epoll.");
    }
  });

  _console.setEOFHandler([this]() {
    _console.disableHandler();
    _state = State::exit;
    std::cout << "Exiting client..." << std::endl;
  });

  if (!setUpSigaction()) {
    return false;
  }

  Console::CommandHandler cmdHandler = [this](std::vector<std::string> &args) {
    addCmdToQueue(args);
  };
  _console.setCommandHandler(cmdHandler);

  _console.setReadline();

  if (!_epoll.init(0) || !_epoll.addFd(STDIN_FILENO, true)) {
    return false;
  }
  _state = State::setup;
  return true;
}

std::string Client::extractSocket(std::string const &conf, bool &err) {
  if (conf.empty()) {
    err = true;
    logError("extractSocket() - No configuration file given!");
    return "";
  }
  try {
    YAML::Node config = YAML::LoadFile(conf);
    YAML::Node ctlNode = config["taskmasterctl"];

    if (!ctlNode.IsDefined()) {
      err = true;
      logError("extractSocket() - configuration file does not include "
               "supervisorctl section!");
      return "";
    }

    std::string socket("");
    if (config["taskmasterctl"]["serverurl"].IsDefined()) {
      socket = config["taskmasterctl"]["serverurl"].as<std::string>();
    }
    return socket;
  } catch (YAML::Exception const &e) {
    logError(std::string("extractSocket() - ") + e.what());
    return "";
  }
}

void Client::addCmdToQueue(std::vector<std::string> &args) {
  Commands::CMD cmd = Commands::getCommand(args[0]);
  args.erase(args.begin());

  if (cmd == Commands::CMD::none) {
    cmdErrorMsg(Commands::CMD::none, args[0]);
    return;
  }

  if (args.size() == 0) {
    _cmdQueue.push({cmd, {}, ""});
  } else if (cmd == Commands::CMD::help) {
    _cmdQueue.push({cmd, args, ""});
  } else {
    std::vector<std::string> formattedArgs = processArgs(args);

    for (auto &arg : formattedArgs) {
      _cmdQueue.push({cmd, {arg}, ""});
    }
  }
}

std::vector<std::string> Client::processArgs(std::vector<std::string> &args) {
  std::vector<std::string> formattedArgs;
  std::map<std::string, std::vector<ProcessInfo>> processMap =
      _console.getProcessMap();

  for (const auto &arg : args) {
    std::vector<std::string> splitArgs = Utils::split(arg, ":");
    if (splitArgs.size() == 1 && splitArgs[0] != "*") {
      // Single program name
      formattedArgs.push_back(splitArgs[0]);
    } else {
      if (splitArgs[1] == "*") {
        // Group name with wildcard
        for (const auto &pair : processMap) {
          if (pair.first == splitArgs[0]) {
            for (const auto &process : pair.second) {
              formattedArgs.push_back(pair.first + ":" + process.name);
              std::cout << "Adding process: " << pair.first + ":" + process.name
                        << std::endl;
            }
          }
        }
        if (formattedArgs.empty()) {
          formattedArgs.push_back(splitArgs[0]);
        }
      } else {
        // Specific program in a group
        std::string fullName = splitArgs[0] + ":" + splitArgs[1];
        formattedArgs.push_back(fullName);
      }
    }
  }

  // std::vector<std::string> processList = _console.getProcessList();

  // for (const auto &arg : args) {
  //   std::vector<std::string> splitArgs = Utils::split(arg, ":");
  //   if (splitArgs.size() == 1 && splitArgs[0] != "*") {
  //     // Single program name
  //     formattedArgs.push_back(splitArgs[0]);
  //   } else {
  //     if (splitArgs[1] == "*") {
  //       // Group name with wildcard
  //       for (const auto &process : processList) {
  //         if (process == splitArgs[0] + ":*") {
  //           continue; // Skip wildcard itself
  //         } else if (process.find(splitArgs[0] + ":") == 0) {
  //           formattedArgs.push_back(process);
  //         }
  //       }
  //       if (formattedArgs.empty()) {
  //         formattedArgs.push_back(splitArgs[0]);
  //       }
  //     } else {
  //       // Specific program in a group
  //       std::string fullName = splitArgs[0] + ":" + splitArgs[1];
  //       formattedArgs.push_back(fullName);
  //     }
  //   }
  // }
  for (auto &arg : formattedArgs) {
    std::cout << "Adding argument: " << arg << std::endl;
  }
  return formattedArgs;
}

bool Client::sendCmd(const std::string &cmd, std::vector<std::string> &args) {
  args.insert(args.begin(), cmd);
  return _request.sendMsg(args);
}

void Client::cmdQuit(CmdRequest &request) {
  (void)request; // Unused parameter
  _console.disableHandler();
  _state = State::exit;
  std::cout << "Exiting client..." << std::endl;
  exit(0);
}

void Client::cmdExit(CmdRequest &request) {
  (void)request; // Unused parameter
  _console.disableHandler();
  _state = State::exit;
  std::cout << "Exiting client..." << std::endl;
  exit(0);
}

bool Client::cmdStatus(CmdRequest &request) {
  _console.disableHandler();
  _console.clearPrompt();
  if (sendCmd(Commands::cmdToString(request.cmd), request.args)) {
    _state = State::waitingReply;
    return true;
  }
  _console.enableHandler();
  return false;
}

bool Client::cmdStart(CmdRequest &request) {
  _console.disableHandler();
  _console.clearPrompt();
  if (request.args.size() <= 0) {
    cmdErrorMsg(Commands::CMD::start);
    _console.enableHandler();
    return false;
  }
  if (sendCmd(Commands::cmdToString(request.cmd), request.args)) {
    _state = State::waitingReply;
    return true;
  }
  _console.enableHandler();
  return false;
}

bool Client::cmdStop(CmdRequest &request) {
  _console.disableHandler();
  _console.clearPrompt();
  if (request.args.size() <= 0) {
    cmdErrorMsg(Commands::CMD::stop);
    _console.enableHandler();
    return false;
  }
  if (sendCmd(Commands::cmdToString(request.cmd), request.args)) {
    _state = State::waitingReply;
    return true;
  }
  _console.enableHandler();
  return false;
}

bool Client::cmdRestart(CmdRequest &request) {
  _console.disableHandler();
  _console.clearPrompt();
  if (request.args.size() <= 0) {
    cmdErrorMsg(Commands::CMD::restart);
    _console.enableHandler();
    return false;
  }
  if (sendCmd(Commands::cmdToString(request.cmd), request.args)) {
    _state = State::waitingReply;
    return true;
  }
  _console.enableHandler();
  return false;
}

bool Client::cmdReload(CmdRequest &request) {
  if (request.args.size() != 0) {
    _console.disableHandler();
    _console.clearPrompt();
    cmdErrorMsg(Commands::CMD::reload);
    _console.enableHandler();
    return false;
  }

  std::string prompt = "Really restart the remote taskmasterd process Y/n? ";

  _console.setQuestionState(prompt, [this](std::string arg) {
    if (arg.empty() || arg == "y" || arg == "Y" || arg == "yes" ||
        arg == "YES") {
      //_console.disableHandler();
      std::cout << "Restarting remote taskmasterd..." << std::endl;
      if (sendCmd(Commands::cmdToString(_currentCmdRequest.cmd),
                  _currentCmdRequest.args)) {
        _state = State::waitingReply;
        return;
      }
    }
    _state = State::running;
    _console.enableHandler();
  });
  _state = State::asking;
  return true;
}

bool Client::run() {
  if (_state == State::idle) {
    _state = State::error;
    logError("run() - Can\'t run before being set up, please call "
             "setupClient() before.");
    return false;
  } else if (_state == State::setup) {
    _state = State::running;
  } else if (_state == State::running) {
    logError("run() - Client already in running state!");
    return false;
  }

  struct epoll_event events[MAX_EVENTS];

  while (_state >= State::running && _state <= State::asking) {
    int nbrFd = _epoll.waitEvents(events, MAX_EVENTS, TIMEOUT);

    for (int i = 0; i < nbrFd; ++i) {
      int fd = events[i].data.fd;

      if (fd == STDIN_FILENO && _state != State::waitingReply) {
        _console.readCharRead();
      } else {
        if (_state == State::waitingReply && events[i].events & EPOLLIN) {
          std::string answer;

          if (!_request.receiveMsg(answer)) {
            logError("Internal error! Can\'t receive message from server.");
            _state = State::running;
          } else {
            processReply(_currentCmdRequest.cmd, answer);
          }
          _state = State::running;
        }
      }
    }
    if (_firstRun) {
      _firstRun = false;
      _cmdQueue.push({Commands::CMD::status, {}, ""});
    }
    if (_state == State::running && !_cmdQueue.empty()) {
      if (!processCmd()) {
        _currentCmdRequest = CmdRequest();
        _cmdQueue = std::queue<CmdRequest>();
      }
    }
  }
  return true;
}

bool Client::processCmd() {
  _currentCmdRequest = _cmdQueue.front();
  _cmdQueue.pop();

  switch (_currentCmdRequest.cmd) {
  case Commands::CMD::quit:
    cmdQuit(_currentCmdRequest);
    break;
  case Commands::CMD::exit:
    cmdExit(_currentCmdRequest);
    break;
  case Commands::CMD::status:
    return cmdStatus(_currentCmdRequest);
  case Commands::CMD::start:
    return cmdStart(_currentCmdRequest);
  case Commands::CMD::stop:
    return cmdStop(_currentCmdRequest);
  case Commands::CMD::restart:
    return cmdRestart(_currentCmdRequest);
  case Commands::CMD::reload:
    return cmdReload(_currentCmdRequest);
  case Commands::CMD::help:
    cmdHelp(_currentCmdRequest);
    break;
  default:
    return false;
  }
  return true;
}

void Client::processReply(const Commands::CMD &cmd, const std::string &reply) {
  switch (cmd) {
  case Commands::CMD::status:
    cmdStatusAnswer(reply);
    break;
  case Commands::CMD::start:
    cmdStartAnswer(reply);
    break;
  case Commands::CMD::stop:
    cmdStopAnswer(reply);
    break;
  case Commands::CMD::restart:
    cmdRestartAnswer(reply);
    break;
  case Commands::CMD::reload:
    cmdReloadAnswer(reply);
    break;
  default:
    logError("processReply() - Unknown command received: " + reply);
    break;
  }
  if (_cmdQueue.empty()) {
    _console.enableHandler();
  }
}

void Client::cmdStatusAnswer(std::string const &answer) {
  std::vector<std::string> processList = Utils::split(answer, "\n");
  std::map<std::string, std::vector<ProcessInfo>> processMap;

  if (processList[0] == "error") {
    std::cout << processList[1] << std::endl;
    return;
  }
  processList.erase(processList.begin()); // Remove the first line (header)

  for (std::string &process : processList) {
    std::vector<std::string> processInfo = Utils::split(process, ";");
    auto it = processMap.find(processInfo[0]);

    if (it != processMap.end()) {
      it->second.push_back({processInfo[1], processInfo[2], processInfo[3]});
    } else {
      processMap[processInfo[0]] = std::vector<ProcessInfo>(
          {ProcessInfo(processInfo[1], processInfo[2], processInfo[3])});
    }
  }
  displayProcessList(processMap);
  if (_currentCmdRequest.args.size() ==
      1) { // > 1 means multiple args (except command)
    _console.setProcessList(processMap);
  }
}

void Client::displayProcessList(
    const std::map<std::string, std::vector<ProcessInfo>> &processMap) {

  for (const auto &pair : processMap) {
    const std::string &programName = pair.first;
    const std::vector<ProcessInfo> &processes = pair.second;

    if (pair.second.size() == 1) {
      std::cout << std::left << std::setw(33) << programName << std::left
                << std::setw(10) << pair.second[0].status << std::left
                << std::left << pair.second[0].message << std::endl;
    } else {
      for (const ProcessInfo &process : processes) {
        const std::string name = programName + ":" + process.name;
        std::cout << std::left << std::setw(33) << name << std::left
                  << std::setw(10) << process.status << std::left
                  << process.message << std::endl;
      }
    }
  }
}

void Client::cmdStartAnswer(std::string const &answer) {
  std::cout << answer << std::endl;
}

void Client::cmdStopAnswer(std::string const &answer) {
  std::cout << answer << std::endl;
}

void Client::cmdRestartAnswer(std::string const &answer) {
  std::cout << answer << std::endl;
}

void Client::cmdReloadAnswer(std::string const &answer) {
  std::cout << answer << std::endl;
}

bool Client::setUpSigaction() {
  struct sigaction sa;

  sa.sa_handler = &Client::signalHandler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;

  if (sigaction(SIGINT, &sa, nullptr) < 0 ||
      sigaction(SIGQUIT, &sa, nullptr) < 0) {
    logError("sigaction() - internal error", errno);
    return false;
  }

  return true;
}

void Client::signalHandler(int signal) {
  (void)signal;
  std::cout << "\nExiting client..." << std::endl;
  exit(0);
}

Client::Client() : _console(Console::Instance()) {}

void Client::logError(const std::string &msg, const int &error) {
  std::cerr << "[Error] - Client::" << msg;
  if (error != -1) {
    std::cerr << " : errno = " << error << std::endl;
  } else {
    std::cerr << std::endl;
  }
}

void Client::cmdErrorMsg(Commands::CMD const &cmd,
                         std::string const name) const {
  switch (cmd) {
  case Commands::CMD::start:
    std::cout << name << ": start requires a process name\n"
              << "start <name>            Start a process\n"
              << "start <name> <name>     Start multiple processes or groups.\n"
              << "start <gname>:*         Start all processes in a group."
              << std::endl;
    break;
  case Commands::CMD::stop:
    std::cout << name << ": stop requires a process name\n"
              << "stop <name>            Stop a process\n"
              << "stop <name> <name>     Stop multiple processes or groups.\n"
              << "stop <gname>:*         Stop all processes in a group."
              << std::endl;

    break;
  case Commands::CMD::restart:
    std::cout
        << name << ": restart requires a process name\n"
        << "restart <name>            Restart a process\n"
        << "restart <name> <name>     Restart multiple processes or groups.\n"
        << "restart <gname>:*         Restart all processes in a group."
        << std::endl;
    break;
  case Commands::CMD::reload:
    std::cout << name << ": reload accepts no arguments\n"
              << "reload          Restart the remote taskmasterd." << std::endl;
    break;
  case Commands::CMD::status:
    std::cout
        << name << ": status accept optionnal arguments\n"
        << "status <name>           Get status for a single process\n"
        << "status <name> <name>    Get status for multiple named processes\n"
        << "status <gname>:*        Get status for all processes in a group\n"
        << "status                  Get status for all processes." << std::endl;
    break;
  case Commands::CMD::exit:
    std::cout << name << ": exit the client" << std::endl;
    break;
  case Commands::CMD::quit:
    std::cout << name << ": quit the client" << std::endl;
    break;
  case Commands::CMD::none:
    std::cout << "*** Unknown syntax: " << name << "\n"
              << "Type 'help' for help." << std::endl;
    break;
  case Commands::CMD::help:
    std::cout << name << ": help accepts an optional argument\n"
              << "help                   Display this help message\n"
              << "help <topic>           Display help on a specific topic."
              << std::endl;
    break;
  default:
    break;
  }
}

void Client::cmdHelp(CmdRequest &request) {
  _console.disableHandler();
  std::string args = Utils::concat(request.args, " ");

  Utils::trim(args);

  if (args.empty()) {
    std::cout << "default commands (type help <topic>):" << std::endl;
    std::cout << std::setfill('=') << std::setw(37) << "" << std::endl;
    std::cout << "exit\thelp\tquit\treload\nrestart\tstart\tstatus\tstop"
              << std::endl;
  } else if (args == "exit") {
    cmdErrorMsg(Commands::CMD::exit, "exit");
  } else if (args == "quit") {
    cmdErrorMsg(Commands::CMD::quit, "quit");
  } else if (args == "reload") {
    cmdErrorMsg(Commands::CMD::reload, "reload");
  } else if (args == "restart") {
    cmdErrorMsg(Commands::CMD::restart, "restart");
  } else if (args == "start") {
    cmdErrorMsg(Commands::CMD::start, "start");
  } else if (args == "stop") {
    cmdErrorMsg(Commands::CMD::stop, "stop");
  } else if (args == "status") {
    cmdErrorMsg(Commands::CMD::status, "status");
  } else if (args == "help") {
    cmdErrorMsg(Commands::CMD::help, "help");
  } else {
    std::cout << "*** No help on " << args << std::endl;
  }
  _console.enableHandler();
}
