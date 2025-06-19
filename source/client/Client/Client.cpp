#include "Client/Client.hpp"
#include "Client/ProcessInfo.hpp"

#include "common/Commands.hpp"
#include "common/Utils.hpp"
#include "pch.hpp" // IWYU pragma: keep

Client &Client::Instance() {
  static Client _instance;

  return _instance;
}

Client::~Client() { cleanUp(); }

bool Client::setupClient(std::string const &conf) {
  bool err = false;
  std::string socket = extractSocket(conf, err);

  if (err) {
    return false;
  }

  _request.setsockFile(socket);

  if (!setUpSigaction()) {
    return false;
  }

  if (!this->registerCommands()) {
    return false;
  }

  std::vector<std::string> cmd({Commands::STATUS});
  cmdStatus(cmd);

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

bool Client::registerCommands() {
  Console::CommandHandler status = [this](std::vector<std::string> &args) {
    cmdStatus(args);
  };
  Console::CommandHandler start = [this](std::vector<std::string> &args) {
    cmdStart(args);
  };
  Console::CommandHandler stop = [this](std::vector<std::string> &args) {
    cmdStop(args);
  };
  Console::CommandHandler restart = [this](std::vector<std::string> &args) {
    cmdRestart(args);
  };
  Console::CommandHandler reload = [this](std::vector<std::string> &args) {
    cmdReload(args);
  };
  Console::CommandHandler quit = [this](std::vector<std::string> &args) {
    cmdQuit(args);
  };

  if (!_console.registerCmd(Commands::STATUS, status) ||
      !_console.registerCmd(Commands::START, start) ||
      !_console.registerCmd(Commands::STOP, stop) ||
      !_console.registerCmd(Commands::RESTART, restart) ||
      !_console.registerCmd(Commands::RELOAD, reload) ||
      !_console.registerCmd(Commands::QUIT, quit)) {
    return false;
  }

  return true;
}

void Client::cmdStatus(std::vector<std::string> &args) {
  int error = 0;
  std::string response = _request.sendMsg(args, error);

  if (error == 0) {
    // response format is programName:processName:status:pid:uptime\n...
    std::vector<std::string> processList = Utils::split(response, "\n");
    std::map<std::string, std::vector<ProcessInfo>> processMap;

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
        const std::string name = programName + ":" + pair.second[0].name;
        std::cout << std::left << std::setw(33) << name << std::left
                  << std::setw(10) << process.status << std::left
                  << process.message << std::endl;
      }
    }
  }
}

bool Client::cmdStart(std::vector<std::string> &args) {
  if (args.size() <= 1) {
    cmdErrorMsg(Commands::CMD::start);
  } else {
    int error = 0;
    std::string response = _request.sendMsg(args, error);

    if (error == 0) {
      std::cout << response << std::endl;
    } else {
      return false;
    }
  }
  return true;
}

bool Client::cmdStop(std::vector<std::string> &args) {
  if (args.size() <= 1) {
    cmdErrorMsg(Commands::CMD::stop);
  } else {
    int error = 0;
    std::string response = _request.sendMsg(args, error);

    if (error == 0) {
      std::cout << response << std::endl;
    } else {
      return false;
    }
  }
  return true;
}

void Client::cmdRestart(std::vector<std::string> &args) {
  if (args.size() <= 1) {
    cmdErrorMsg(Commands::CMD::restart);
  } else {
    args[0] = "stop";
    if (!cmdStop(args)) {
      return;
    }
    args[0] = "start";
    cmdStart(args);
  }
}

void Client::cmdReload(std::vector<std::string> &args) {
  if (args.size() > 1) {
    cmdErrorMsg(Commands::CMD::reload);
    return;
  }

  _userAnswer = "Y/n";
  std::string prompt =
      "Really restart the remote taskmasterd process " + _userAnswer + "? ";

  _console.setQuestionState(prompt, [this](std::string arg) {
    if (arg.empty() || arg == "y" || arg == "Y" || arg == "yes") {
      int error = 0;
      std::string response =
          _request.sendMsg(std::vector<std::string>({Commands::RELOAD}), error);

      if (error == 0) {
        std::cout << response << std::endl;
      }
    } else {
      std::cout << "Reload aborted." << std::endl;
    }
  });
}

void Client::cmdQuit(std::vector<std::string> &) { _state = State::exit; }

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

      if (fd == STDIN_FILENO) {
        if (_state != State::asking) {
          _console.readCharRead();
        }
      } else if (fd == _request.getrcvFd()) {
        // read the msg receive and print it to the terminal
        // and pass it to console to print it and reset prompt after it
      } else {
        continue;
      }
    }
  }
  _console.cleanUp();
  return true;
}

void Client::cleanUp() {
  _console.cleanUp();
  _epoll.cleanUp();
  _request.cleanUp();
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
  Client &instance = Client::Instance();

  (void)signal;
  instance.cleanUp();
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

void Client::cmdErrorMsg(Commands::CMD const &cmd) const {
  switch (cmd) {
  case Commands::CMD::start:
    std::cout << "Error: start requires a process name\n"
              << "start <name>            Start a process\n"
              << "start <name> <name>     Start multiple processes or groups."
              << std::endl;
    break;
  case Commands::CMD::stop:
    std::cout << "Error: stop requires a process name\n"
              << "stop <name>            Stop a process\n"
              << "stop <name> <name>     Stop multiple processes or groups."
              << std::endl;

    break;
  case Commands::CMD::restart:
    std::cout
        << "Error: restart requires a process name\n"
        << "restart <name>            Restart a process\n"
        << "restart <name> <name>     Restart multiple processes or groups."
        << std::endl;
    break;
  case Commands::CMD::reload:
    std::cout << "Error: reload accepts no arguments\n"
              << "reload          Restart the remote taskmasterd." << std::endl;
    break;
  default:
    break;
  }
}
