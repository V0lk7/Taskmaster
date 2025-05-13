#include "Client/RequestClient/RequestClient.hpp"

#include "pch.hpp" // IWYU pragma: keep

RequestClient::RequestClient() {}

int RequestClient::setSocket(std::string const &sockFile) {
  if (sockFile.empty()) {
    return -1;
  }
  _sockFd = nn_socket(AF_SP, NN_REQ);

  if (_sockFd == -1) {
    return -1;
  }
  size_t sz = sizeof(_rcvFd);

  if (nn_getsockopt(_sockFd, NN_SOL_SOCKET, NN_RCVFD, &_rcvFd, &sz) == -1) {
    return -1;
  }

  if (nn_setsockopt(_sockFd, NN_SOL_SOCKET, NN_SNDTIMEO, &_timeout,
                    sizeof(_timeout)) == -1) {
    return -1;
  }

  if (nn_connect(_sockFd, sockFile.c_str()) == -1) {
    return -1;
  }
  return _sockFd;
}

int RequestClient::getrcvFd() const { return _rcvFd; }
