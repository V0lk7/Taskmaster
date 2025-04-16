#ifndef EPOLL_HPP
#define EPOLL_HPP

#include <queue>
#include <string>
#include <sys/epoll.h>
#include <utility>

class Epoll {
public:
  using messageQueue = std::queue<std::pair<std::string, std::string>>;

  Epoll();
  ~Epoll();

  bool init(int flags);
  int waitEvents(struct epoll_event *events, int max_events, int timeout);
  bool addFd(int fd);
  void cleanUp();

  void insertMessage(std::string const &, std::string const &);

private:
  int _epollFd = -1;
  messageQueue _messageQueue;

  bool makeFdNonBlocking(int fd);
};

#endif
