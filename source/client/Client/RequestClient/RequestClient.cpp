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

  if (!socketFileExists()) {
    logError("connectToDaemon - " + _sockFile + " - No such file!");
    return false;
  }

  _sockFd = nn_socket(AF_SP, NN_REQ);

  if (_sockFd == -1) {
    logError("connectToDaemon - nn_socket() failed", errno);
    return false;
  }
  size_t sz = sizeof(_rcvFd);

  if (nn_getsockopt(_sockFd, NN_SOL_SOCKET, NN_RCVFD, &_rcvFd, &sz) == -1) {
    logError("connectToDaemon - nn_getsockopt() failed", errno);
    return false;
  }

  if (nn_setsockopt(_sockFd, NN_SOL_SOCKET, NN_SNDTIMEO,
                    &RequestClient::TIMEOUT,
                    sizeof(RequestClient::TIMEOUT)) == -1 ||
      nn_setsockopt(_sockFd, NN_SOL_SOCKET, NN_RCVTIMEO,
                    &RequestClient::TIMEOUT,
                    sizeof(RequestClient::TIMEOUT)) == -1) {
    logError("connectToDaemon - nn_setsockopt() failed", errno);
    return false;
  }

  if (nn_connect(_sockFd, _sockFile.c_str()) == -1) {
    switch (errno) {
    case EBADF:
    case EINVAL:
      logError("connectToDaemon - " + _sockFile + " - No such file!");
      break;
    default:
      logError("connectToDaemon - connection failed", errno);
    }
    return false;
  }
  return true;
}

bool RequestClient::socketFileExists() const {
  struct stat buffer;
  std::string name;

  if (_sockFile.find("ipc://") != std::string::npos) {
    name = std::string(_sockFile.begin() + 6, _sockFile.end());
  } else {
    name = _sockFile;
  }
  return stat(name.c_str(), &buffer) == 0;
}

int RequestClient::getrcvFd() const { return _rcvFd; }

void RequestClient::cleanUp() {
  _state = State::idle;
  _rcvFd = -1;

  if (_sockFd > -1) {
    nn_close(_sockFd);
  }
}

bool RequestClient::isConnectionAlive() {
  if (nn_send(_sockFd, "ping", 4, 0) < 4) {
    _state = State::idle;
    logError("isConnectionAlive - Connection refused!");
    return false;
  }
  char buf[1024];
  bzero(buf, 1024);

  if (!tryRecv(buf, sizeof(buf), "pong")) {
    _state = State::idle;
    logError("isConnectionAlive - Connection refused!");
    return false;
  }
  _state = State::connected;
  return true;
}

std::string RequestClient::sendMsg(std::vector<std::string> const &msg,
                                   int &error) {
  if (_state == State::idle) {
    if (!connectToSocket()) {
      error = -1;
      return "";
    }
    if (!isConnectionAlive()) {
      error = -1;
      cleanUp();
      return "";
    }
  }

  std::ostringstream oss;
  for (std::string item : msg) {
    Utils::trim(item);
    oss << " " << item;
  }
  std::string msgContent = oss.str();
  Utils::trim(msgContent);

  ssize_t s = nn_send(_sockFd, msgContent.c_str(), msgContent.size(), 0);

  if (s == -1 || s < static_cast<ssize_t>(msgContent.size())) {
    _state = State::idle;
    error = -1;
    return "";
  }

  char buf[1024];
  bzero(buf, 1024);

  if (!tryRecv(buf, sizeof(buf))) {
    cleanUp();
    error = -1;
    return "";
  }

  return std::string(buf);
}

bool RequestClient::tryRecv(char *buf, size_t size,
                            const std::string &expected) {
  for (int i = 0; i < TRY_RCV; ++i) {
    if (nn_recv(_sockFd, buf, size, 0) > 0) {
      if (expected.empty() || expected == buf) {
        return true;
      }
    }
  }
  return false;
}

void RequestClient::logError(const std::string &msg, const int &error) {
  std::cerr << "[Error] - RequestClient::" << msg;
  if (error != -1) {
    std::cerr << " : errno = " << error << std::endl;
  } else {
    std::cerr << std::endl;
  }
}
