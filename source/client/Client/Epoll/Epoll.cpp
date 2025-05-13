#include "Client/Epoll/Epoll.hpp"

#include <fcntl.h>
#include <iostream>
#include <sys/epoll.h>
#include <unistd.h>

Epoll::Epoll() {}

Epoll::~Epoll() { cleanUp(); };

void Epoll::cleanUp() {
  if (_epollFd > -1) {
    close(_epollFd);
    _epollFd = -1;
  }
}

bool Epoll::init(int flags) {
  _epollFd = epoll_create1(flags);

  if (_epollFd < 0) {
    std::cerr << "[Error] - Epoll::init - epoll_create1() failed : errno = "
              << errno << std::endl;
    return false;
  }
  return true;
}

int Epoll::waitEvents(struct epoll_event *events, int max_events, int timeout) {
  return epoll_wait(_epollFd, events, max_events, timeout);
}

bool Epoll::addFd(int fd, bool modifyIt) {
  if (modifyIt) {
    if (!this->makeFdNonBlocking(fd)) {
      return false;
    }
  }
  struct epoll_event ev = {};

  ev.events = EPOLLIN;
  ev.data.fd = fd;

  if (epoll_ctl(_epollFd, EPOLL_CTL_ADD, fd, &ev) < 0) {
    std::cerr << "[Error] - Epoll::addFdToEpoll - epoll_ctl() failed : errno = "
              << errno << std::endl;
    return false;
  }
  return true;
}

bool Epoll::makeFdNonBlocking(int fd) {
  int flags = fcntl(fd, F_GETFL, 0);

  if (flags < 0) {
    std::cerr
        << "[Error:1] - Epoll::MakeFdNonBlocking - fcntl failed : errno = "
        << errno << std::endl;
    return false;
  }
  if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0) {
    std::cerr
        << "[Error:2] - Epoll::MakeFdNonBlocking - fcntl failed : errno = "
        << errno << std::endl;
    return false;
  }
  return true;
}
