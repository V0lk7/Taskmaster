#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <cstdint>
#include <string>

class Client {
public:
  static Client &Instance();
  ~Client();

  bool setupClient(std::string const &conf);
  bool run();

  // static void readLineHandlerStatic(char *);

private:
  int _epoll_fd = -1;
  bool _isRunning = false;

  bool setupEpoll();
  bool setupReadline();
  bool addFdToEpoll(int const &fd, uint32_t const &);
  bool makeFdNonBlocking(int const &fd);

  void readlineHandler(std::string);
  static void readLineHandlerStatic(char *);

  void setIsRunning(bool);

  Client();
  Client(Client &) = delete;
  Client(Client &&) = delete;
  Client &operator=(Client &) = delete;
  Client &operator=(Client &&) = delete;
};

#endif
