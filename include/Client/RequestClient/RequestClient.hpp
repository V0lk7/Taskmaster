#ifndef REQUESTCLIENT_HPP
#define REQUESTCLIENT_HPP

#include "pch.hpp" // IWYU pragma: keep

class RequestClient {
public:
  RequestClient();
  ~RequestClient();

  int getrcvFd() const;
  std::string sendMsg(std::vector<std::string> const &, int &);

  void setsockFile(std::string const &);
  void cleanUp();

private:
  enum class State { idle, connected, waitingResponse };

  std::string _sockFile;
  int _sockFd; // nanomsg socket
  int _rcvFd;  // Used to epoll on the nano socket
  State _state;

  static constexpr int TIMEOUT = 10; // 500ms timeout
  static constexpr int TRY_RCV = 3;

  bool isConnectionAlive();
  bool socketFileExists() const;
  bool tryRecv(char *buf, size_t size, const std::string &expected = "");
  void logError(const std::string &msg, const int &error = -1);
  bool connectToSocket();
};

#endif
