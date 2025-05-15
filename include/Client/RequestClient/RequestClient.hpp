#ifndef REQUESTCLIENT_HPP
#define REQUESTCLIENT_HPP

#include "pch.hpp" // IWYU pragma: keep

class RequestClient {
public:
  RequestClient();
  ~RequestClient();

  bool connectToDaemon();
  int getrcvFd() const;
  bool sendMsg(std::vector<std::string> const &);
  std::string readMsg();

  void setsockFile(std::string const &);

private:
  enum class State { idle, connected, waitingResponse };

  std::string _sockFile;
  int _sockFd; // nanomsg socket
  int _rcvFd;  // Used to epoll on the nano socket
  State _state;

  static constexpr int _timeout = 500; // 500ms timeout
};

#endif
