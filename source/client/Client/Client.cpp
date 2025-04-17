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
  std::cout << "Status has been called." << std::endl;
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
    std::cout << "[start] command doesn\'t take any parameters!" << std::endl;
  } else {
    for (std::string const &arg : args) {
      _epoll.insertMessage(START, arg);
    }
    // call epoll req/rep message;
  }
  std::cout << "Start has been called." << std::endl;
}

void Client::cmdStop(std::vector<std::string> &args) {
  if (args.empty()) {
    std::cout << "[stop] command doesn\'t take any parameters!" << std::endl;
  } else {
    for (std::string const &arg : args) {
      _epoll.insertMessage(STOP, arg);
    }
    // call epoll req/rep message;
  }
  std::cout << "Stop has been called." << std::endl;
}

void Client::cmdRestart(std::vector<std::string> &args) {
  if (args.empty()) {
    std::cout << "[restart] command doesn\'t take any parameters!" << std::endl;
  } else {
    for (std::string const &arg : args) {
      _epoll.insertMessage(RESTART, arg);
    }
    // call epoll req/rep message;
  }
  std::cout << "Restart has been called." << std::endl;
}

void Client::cmdReload(std::vector<std::string> &args) {
  if (!args.empty()) {
    std::cout << "[reload] command doesn\'t take any parameters!" << std::endl;
    return;
  }
  _epoll.insertMessage(RELOAD, "");
  std::cout << "Reload has been called." << std::endl;
}
void Client::cmdQuit(std::vector<std::string> &args) {
  if (!args.empty()) {
    std::cout << "[quit] command doesn\'t take any parameters!" << std::endl;
    return;
  }
  std::cout << "Quit has been called." << std::endl;
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

  while (_state == State::running) {
    int nbrFd = _epoll.waitEvents(events, MAX_EVENTS, TIMEOUT);

    for (int i = 0; i < nbrFd; ++i) {
      int fd = events[i].data.fd;

      if (fd == STDIN_FILENO) {
        _console.readCharRead();
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
}

bool Client::setUpSigaction() {
  struct sigaction sa;

  sa.sa_handler = &Client::signalHandler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;

  if (sigaction(SIGINT, &sa, nullptr) < 0) {
    std::cerr
        << "[Error:1] - Client::setUpSigaction - sigaction failed, errno = "
        << errno << std::endl;
    return false;
  }
  if (sigaction(SIGQUIT, &sa, nullptr) < 0) {
    std::cerr
        << "[Error:2] - Client::setUpSigaction - sigaction failed, errno = "
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
