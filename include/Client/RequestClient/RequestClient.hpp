#ifndef REQUESTCLIENT_HPP
#define REQUESTCLIENT_HPP

#include "pch.hpp" // IWYU pragma: keep

class RequestClient {
public:
  RequestClient();
  ~RequestClient();

  bool sendMsg(std::vector<std::string> const &);
  bool receiveMsg(std::string &);

  int getrcvFd() const;
  void setSockFile(std::string const &);

  void setAddFdToEpoll(std::function<void()> const &);
  void setRemoveFdFromEpoll(std::function<void()> const &);

private:
  enum class State { idle, connected, waitingResponse };

  std::string _sockFile;
  int _sockFd; // nanomsg socket
  int _rcvFd;  // Used to epoll on the nano socket
  State _state;

  std::function<void()> _addFdToEpoll;
  std::function<void()> _removeFdFromEpoll;

  static constexpr int TIMEOUT = 10; // 10ms timeout

  static constexpr char IPC[] = "ipc://";
  static constexpr char UNIX[] = "unix://";

  bool socketFileExists();
  // bool tryRecv(std::string &, const std::string &expected = "");
  void logError(const std::string &msg, const int &error = -1);
  bool connectToSocket();
};

#endif
