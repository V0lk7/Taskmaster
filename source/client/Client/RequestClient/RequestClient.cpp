#include "Client/RequestClient/RequestClient.hpp"
#include "common/Utils.hpp"

constexpr int RequestClient::TIMEOUT;
constexpr int RequestClient::TRY_RCV;

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

bool RequestClient::connectToSocket() {
  if (_state > State::idle) {
    return true;
  }

  struct stat buffer; // Check if file exist
  std::string name(_sockFile.begin() + 6, _sockFile.end());

  if (stat(name.c_str(), &buffer) == -1) {
    std::cerr << "[Error] - no such file" << std::endl;
    return false;
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
                    &RequestClient::TIMEOUT,
                    sizeof(RequestClient::TIMEOUT)) == -1) {
    std::cerr
        << "[Error] - RequestClient::connectToDaemon - nn_setsockopt() failed "
           ": errno = "
        << errno << std::endl;
    return false;
  }

  if (nn_setsockopt(_sockFd, NN_SOL_SOCKET, NN_RCVTIMEO,
                    &RequestClient::TIMEOUT,
                    sizeof(RequestClient::TIMEOUT)) == -1) {
    std::cerr
        << "[Error] - RequestClient::connectToDaemon - nn_setsockopt() failed "
           ": errno = "
        << errno << std::endl;
    return false;
  }

  if (nn_connect(_sockFd, _sockFile.c_str()) == -1) {
    switch (errno) {
    case EBADF:
    case EINVAL:
      std::cerr << _sockFile << " no such file!" << std::endl;
      break;
    default:
      std::cerr
          << "[Error] - RequestClient::connectToDaemon - connection failed "
             ": errno = "
          << errno << std::endl;
    }
    return false;
  }
  return true;
}

int RequestClient::getrcvFd() const { return _rcvFd; }

void RequestClient::clean() {
  _state = State::idle;
  _rcvFd = -1;

  if (_sockFd > -1) {
    nn_close(_sockFd);
  }
}

bool RequestClient::isConnectionAlive() {
  if (_state == State::idle) {
    return false;
  }

  if (nn_send(_sockFd, "ping", 4, 0) < 4) {
    return false;
  }
  char buf[1024];
  bzero(buf, 1024);

  for (int i = 0; i < TRY_RCV; i++) {
    if (nn_recv(_sockFd, buf, sizeof(buf), 0) > 0 && strcmp(buf, "pong") == 0) {
      _state = State::connected;
      return true;
    }
  }
  _state = State::idle;
  return false;
}

bool RequestClient::sendMsg(std::vector<std::string> const &msg) {
  if (_state == State::idle) {
    if (!connectToSocket()) {
      return false;
    }
    if (!isConnectionAlive()) {
      clean();
      std::cout << "[Error] Connection refused!" << std::endl;
      return false;
    }
  }

  std::string msgContent;

  for (std::string item : msg) {
    Utils::trim(item);

    msgContent += " " + item;
  }
  Utils::trim(msgContent);

  ssize_t s = nn_send(_sockFd, msgContent.c_str(), msgContent.size(), 0);

  if (s == -1 || s < static_cast<ssize_t>(msgContent.size())) {
    _state = State::idle;
    return false;
  }

  char buf[1024];
  bzero(buf, 1024);

  int i = 0;
  while (i++ < TRY_RCV) {
    if (nn_recv(_sockFd, buf, sizeof(buf), 0) > 0) {
      break;
    }
  }
  if (i == TRY_RCV) {
    clean();
    return false;
  }

  std::cout << "received = " << buf << std::endl;
  return true;
}
