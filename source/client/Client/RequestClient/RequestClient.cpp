#include "Client/RequestClient/RequestClient.hpp"

constexpr int RequestClient::_timeout;

RequestClient::RequestClient() : _sockFd(-1), _state(State::idle) {}

RequestClient::~RequestClient() {
  if (_sockFd > -1) {
    nn_close(_sockFd);
  }
}

void RequestClient::setsockFile(std::string const &src) {
  if (src != _sockFile) {
    _sockFile = src;
  }
}

bool RequestClient::connectToDaemon() {
  if (_state > State::idle) {
    return true;
  }

  struct stat buffer; // Check if file exist

  if (stat(_sockFile.c_str(), &buffer) == -1) {
    std::cerr
        << "[Error] - RequestClient::connectToDaemon - bad file : errno = "
        << errno << std::endl;
    return true;
  }

  _sockFd = nn_socket(AF_SP, NN_REQ);

  if (_sockFd == -1) {
    std::cerr << "[Error] - RequestClient::connectToDaemon - nn_socket() "
                 "failed : errno = "
              << errno << std::endl;
    return false;
  }
  size_t sz = sizeof(_rcvFd);

  if (nn_getsockopt(_sockFd, NN_SOL_SOCKET, NN_RCVFD, &_rcvFd, &sz) == -1) {
    std::cerr
        << "[Error] - RequestClient::connectToDaemon - nn_getsockopt() failed "
           ": errno = "
        << errno << std::endl;
    return false;
  }

  if (nn_setsockopt(_sockFd, NN_SOL_SOCKET, NN_SNDTIMEO,
                    &RequestClient::_timeout,
                    sizeof(RequestClient::_timeout)) == -1) {
    std::cerr
        << "[Error] - RequestClient::connectToDaemon - nn_setsockopt() failed "
           ": errno = "
        << errno << std::endl;
    return false;
  }

  if (nn_connect(_sockFd, _sockFile.c_str()) == -1) {
    std::cerr << "[Error] - RequestClient::connectToDaemon - connection failed "
                 ": errno = "
              << errno << std::endl;
  } else {
    _state = State::connected;
  }

  return true;
}

int RequestClient::getrcvFd() const { return _rcvFd; }

bool RequestClient::sendMsg(std::vector<std::string> const &msg) {
  if (_state == State::idle) {
    if (!connectToDaemon()) {
      return false;
    }
  }
  std::cout << "Envoi cette commande [" << std::endl;
  for (auto it = msg.cbegin(); it != msg.cend(); it++) {
    std::cout << *it << std::endl;
  }
  std::cout << "]" << std::endl;
  return true;
}
