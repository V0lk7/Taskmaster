#include "Client/Client.hpp"

#include "pch.hpp"

Client &Client::Instance() {
  static Client _instance;

  return _instance;
}

Client::~Client() { cleanUp(); }

bool Client::setupClient(std::string const &conf) {
  std::string socket = extractSocket(conf);

  if (socket.empty()) {
    return false;
  }

  if (!setUpSigaction()) {
    return false;
  }

  if (!this->registerCommands()) {
    return false;
  }
  _state = State::setup;

  if (!_epoll.init(0) || !_epoll.addFd(STDIN_FILENO, true) ||
      !_epoll.addFd(_request.getrcvFd(), false)) {
    return false;
  }
  return true;
}

std::string Client::extractSocket(std::string const &conf) {
  if (conf.empty()) {
    std::cerr
        << "[Error] - Client::extractSocket - No configuration file given!"
        << std::endl;
    return "";
  }
  try {
    YAML::Node config = YAML::LoadFile(conf);
    YAML::Node ctlNode = config["taskmasterctl"];

    if (!ctlNode.IsDefined() || !ctlNode["serverurl"]) {
      std::cerr << "[Error] - Client::extractSocket - "
                << "socket information was not found!" << std::endl;
      return "";
    }

    std::string socket = config["taskmasterctl"]["serverurl"].as<std::string>();

    if (socket.empty()) {
      std::cerr << "[Error] - Client::extractSocket - " << "variable is empty!"
                << std::endl;
      return "";
    }
    return socket;
  } catch (YAML::Exception const &e) {
    std::cerr << "[Error] - Client::extractSocket - " << e.what() << std::endl;
    return "";
  }
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
    (void)args;
    this->cmdReload();
  };
  Console::CommandHandler quit = [this](std::vector<std::string> &args) {
    (void)args;
    this->cmdQuit();
  };

  if (!_console.registerCmd("status", status) ||
      !_console.registerCmd("start", start) ||
      !_console.registerCmd("stop", stop) ||
      !_console.registerCmd("restart", restart) ||
      !_console.registerCmd("reload", reload) ||
      !_console.registerCmd("quit", quit)) {
    return false;
  }

  return true;
}

void Client::cmdStatus(std::vector<std::string> &args) {
  (void)args;
  std::cout << "Status has been called." << std::endl;
}
void Client::cmdStart(std::vector<std::string> &args) {
  (void)args;
  std::cout << "Start has been called." << std::endl;
}
void Client::cmdStop(std::vector<std::string> &args) {
  (void)args;
  std::cout << "stop has been called." << std::endl;
}
void Client::cmdRestart(std::vector<std::string> &args) {
  (void)args;
  std::cout << "Restart has been called." << std::endl;
}
void Client::cmdReload() {
  std::cout << "Reload has been called." << std::endl;
}
void Client::cmdQuit() {
  std::cout << "Quit has been called." << std::endl;
  cleanUp();
  exit(0);
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

  while (_state == State::running || _state == State::waitReply) {
    int nbrFd = _epoll.waitEvents(events, MAX_EVENTS, TIMEOUT);

    for (int i = 0; i < nbrFd; ++i) {
      int fd = events[i].data.fd;

      if (fd == STDIN_FILENO) {
        _console.readCharRead();
      } else if (fd == _request.getrcvFd()){
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
