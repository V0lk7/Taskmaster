#include "Client/Client.hpp"
#include <cstdint>
#include <cstdlib>
#include <iostream>

#include <fcntl.h>
#include <readline/history.h>
#include <readline/readline.h>
#include <sys/epoll.h>
#include <unistd.h>

Client &Client::Instance() {
  static Client _instance;

  return _instance;
}

Client::~Client() {
  if (_epoll_fd > -1) {
    (void)close(_epoll_fd);
  }
  clear_history();
  rl_clear_history();
}

bool Client::setupClient(std::string const &conf) {
  // if (conf.empty()) {
  //   std::cerr << "[Error] - Client::setupClient - No conf file given!"
  //             << std::endl;
  //   return false;
  // }
  (void)conf;
  if (!setupEpoll() || !setupReadline()) {
    return false;
  }
  if (!addFdToEpoll(STDIN_FILENO, EPOLLIN)) {
    return false;
  }

  return true;
}

bool Client::run() {
  if (_epoll_fd < 0) {
    std::cerr
        << "[Debug] - Client::run - Cannot run before use of setupClient()"
        << std::endl;
    return false;
  }
  if (_isRunning) {
    std::cerr << "[Debug] - Client::run - Client already running!" << std::endl;
    return false;
  }
  _isRunning = true;

  const int MAX_EVENTS = 10;
  struct epoll_event events[MAX_EVENTS];

  while (_isRunning) {
    int nbrFd = epoll_wait(_epoll_fd, events, MAX_EVENTS, 100);

    for (int i = 0; i < nbrFd; ++i) {
      int fd = events[i].data.fd;

      if (fd == STDIN_FILENO) {
        rl_callback_read_char();
      } else {
        continue;
      }
    }
  }
  rl_callback_handler_remove();
  return true;
}

bool Client::setupEpoll() {
  _epoll_fd = epoll_create1(0);

  if (_epoll_fd < 0) {
    std::cerr
        << "[Error] - Client::setupEpoll - epoll_create1() failed : errno = "
        << errno << std::endl;
    return false;
  }
  return true;
}

bool Client::setupReadline() {
  rl_callback_handler_install(">>> ", &Client::readLineHandlerStatic);
  return true;
}

bool Client::addFdToEpoll(int const &newFd, uint32_t const &eventsType) {
  if (!this->makeFdNonBlocking(newFd)) {
    return false;
  }
  struct epoll_event ev = {};

  ev.events = eventsType;
  ev.data.fd = newFd;

  if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, newFd, &ev) < 0) {
    std::cerr
        << "[Error] - Client::addFdToEpoll - epoll_ctl() failed : errno = "
        << errno << std::endl;
    return false;
  }
  return true;
}

bool Client::makeFdNonBlocking(int const &fd) {
  int flags = fcntl(fd, F_GETFL, 0);

  if (flags < 0) {
    std::cerr
        << "[Error:1] - Client::MakeFdNonBlocking - fcntl failed : errno = "
        << errno << std::endl;
    return false;
  }
  if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0) {
    std::cerr
        << "[Error:2] - Client::MakeFdNonBlocking - fcntl failed : errno = "
        << errno << std::endl;
    return false;
  }
  return true;
}

void Client::readlineHandler(std::string str) {
  if (!str.empty() && str.back() == '\n') {
    str.pop_back();
  }
  if (!str.empty()) {
    add_history(str.c_str());
  }
  rl_replace_line("", 0);
  rl_on_new_line();
}

void Client::readLineHandlerStatic(char *line) {
  Client &instance = Client::Instance();
  if (!line) {
    instance.setIsRunning(false);
  } else {
    std::string strLine(line);

    instance.readlineHandler(strLine);
    free(line);
  }
}

void Client::setIsRunning(bool newVal) {
  if (newVal != _isRunning) {
    _isRunning = newVal;
  }
}

Client::Client() {}
