#include "Client/Client.hpp"
#include "Client/Console/Console.hpp"
#include "common/Commands.hpp"

Client &Client::Instance() {
  static Client _instance;

  return _instance;
}

Client::~Client() { cleanUp(); }

bool Client::setupClient(std::string const &conf) {
  if (conf.empty()) {
    std::cerr << "[Error] - Client::setupClient - No conf file given!"
              << std::endl;
    return false;
  }
  (void)conf;
  if (!setUpSigaction()) {
    return false;
  }

  if (!_epoll.init(0) || !_epoll.addFd(STDIN_FILENO)) {
    return false;
  }

  if (!this->registerCommands()) {
    return false;
  }
  _state = State::setup;
  return true;
}

bool Client::registerCommands() {
  Console::CommandHandler status = [this](std::vector<std::string> &args) {
    this->cmdStatus(args);
  };
  Console::CommandHandler start = [this](std::vector<std::string> &args) {
    this->cmdStart(args);
  };
  Console::CommandHandler stop = [this](std::vector<std::string> &args) {
    this->cmdStop(args);
  };
  Console::CommandHandler restart = [this](std::vector<std::string> &args) {
    this->cmdRestart(args);
  };
  Console::CommandHandler reload = [this](std::vector<std::string> &args) {
    this->cmdReload(args);
  };
  Console::CommandHandler quit = [this](std::vector<std::string> &args) {
    this->cmdQuit(args);
  };

  if (!_console.registerCmd(STATUS, status) ||
      !_console.registerCmd(START, start) ||
      !_console.registerCmd(STOP, stop) ||
      !_console.registerCmd(RESTART, restart) ||
      !_console.registerCmd(RELOAD, reload) ||
      !_console.registerCmd(QUIT, quit)) {
    return false;
  }

  return true;
}

void Client::cmdStatus(std::vector<std::string> &args) {
  if (args.empty()) {
    _epoll.insertMessage(STATUS, "");
  } else {
    for (std::string const &arg : args) {
      _epoll.insertMessage(STATUS, arg);
    }
  }
  // call epoll req/rep message
}

void Client::cmdStart(std::vector<std::string> &args) {
  if (args.empty()) {
    std::cout << "Error: start requires a process name\n"
              << "start <name>            Start a process\n"
              << "start <name> <name>     Start multiple processes or groups."
              << std::endl;
  } else {
    for (std::string const &arg : args) {
      _epoll.insertMessage(START, arg);
    }
    // call epoll req/rep message;
  }
}

void Client::cmdStop(std::vector<std::string> &args) {
  if (args.empty()) {
    std::cout << "Error: stop requires a process name\n"
              << "stop <name>            Stop a process\n"
              << "stop <name> <name>     Stop multiple processes or groups."
              << std::endl;
  } else {
    for (std::string const &arg : args) {
      _epoll.insertMessage(STOP, arg);
    }
    // call epoll req/rep message;
  }
}

void Client::cmdRestart(std::vector<std::string> &args) {
  if (args.empty()) {
    std::cout
        << "Error: restart requires a process name\n"
        << "restart <name>            Restart a process\n"
        << "restart <name> <name>     Restart multiple processes or groups."
        << std::endl;
  } else {
    for (std::string const &arg : args) {
      _epoll.insertMessage(RESTART, arg);
    }
    // call epoll req/rep message;
  }
}

void Client::cmdReload(std::vector<std::string> &args) {
  if (!args.empty()) {
    std::cout << "Error: reload accepts no arguments\n"
              << "reload          Restart the remote supervisord." << std::endl;
    return;
  }
  if (askUserConfirmation("Really restart the remote supervisord process ")) {
    _console.disableHandler();
    _epoll.insertMessage(RELOAD, "");
    _console.enableHandler();
  }
}

bool Client::askUserConfirmation(std::string const &question) {
  std::string fullPrompt = question + "y/N? ";

  _state = State::asking;

  std::cout << "before readline" << std::endl;
  char *line = readline(fullPrompt.c_str());
  std::cout << "after readline" << std::endl;

  _state = State::running;
  if (!line) {
    return false;
  }

  std::string input(line);
  free(line);

  std::transform(input.begin(), input.end(), input.begin(), ::tolower);
  return (input == "y" || input == "yes");
}

void Client::cmdQuit(std::vector<std::string> &args) {
  (void)args;
  _state = State::exit;
}

bool Client::run() {
  if (_state == State::idle) {
    _state = State::error;
    std::cerr
        << "[Debug] - Client::run - Cannot run before use of setupClient()"
        << std::endl;
    return false;
  } else if (_state == State::setup) {
    _state = State::running;
  } else if (_state == State::running) {
    std::cerr << "[Debug] - Client::run - Client already running!" << std::endl;
    return false;
  }

  struct epoll_event events[MAX_EVENTS];

  while (_state == State::running || _state == State::asking) {
    int nbrFd = _epoll.waitEvents(events, MAX_EVENTS, TIMEOUT);

    for (int i = 0; i < nbrFd; ++i) {
      int fd = events[i].data.fd;

      if (fd == STDIN_FILENO) {
        if (_state != State::asking) {
          _console.readCharRead();
        }
      } else {
        continue;
      }
    }
  }
  return true;
}

void Client::cleanUp() {
  _console.cleanUp();
  _epoll.cleanUp();
}

bool Client::setUpSigaction() {
  struct sigaction sa;

  sa.sa_handler = &Client::signalHandler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;

  if (sigaction(SIGINT, &sa, nullptr) < 0) {
    std::cerr
        << "[Debug] - Client::setUpSigaction -1- sigaction failed, errno = "
        << errno << std::endl;
    return false;
  }
  if (sigaction(SIGQUIT, &sa, nullptr) < 0) {
    std::cerr
        << "[Debug] - Client::setUpSigaction -2- sigaction failed, errno = "
        << errno << std::endl;
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
