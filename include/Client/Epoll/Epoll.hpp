#ifndef EPOLL_HPP
#define EPOLL_HPP

#include "pch.hpp" // IWYU pragma: keep

class Epoll {
public:
  Epoll();
  ~Epoll();

  bool init(int flags);
  int waitEvents(struct epoll_event *events, int max_events, int timeout);
  bool addFd(int fd, bool modifyIt);
  void cleanUp();

private:
  int _epollFd = -1;

  bool makeFdNonBlocking(int fd);
};

#endif
