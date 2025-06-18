#include "Client/RequestClient/RequestClient.hpp"
#include "common/Commands.hpp"
#include "common/Utils.hpp"

constexpr char RequestClient::IPC[];
constexpr char RequestClient::UNIX[];

RequestClient::RequestClient() : _sockFd(-1), _state(State::idle) {}

RequestClient::~RequestClient() {
  if (_sockFd > -1) {
    nn_close(_sockFd);
  }
}

void RequestClient::setsockFile(std::string const &src) {
  if (src != _sockFile) {
    std::cout << "aled 1" << std::endl;
    _sockFile = src;
    std::cout << "aled 2" << std::endl;
  }
}

bool RequestClient::connectToSocket() {
  if (_state > State::idle) {
    return true;
  }

  if (!socketFileExists()) {
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

  int linger = TIMEOUT;
  if (nn_setsockopt(_sockFd, NN_SOL_SOCKET, NN_SNDTIMEO, &linger,
                    sizeof(linger)) == -1 ||
      nn_setsockopt(_sockFd, NN_SOL_SOCKET, NN_RCVTIMEO, &linger,
                    sizeof(linger)) == -1) {
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

bool RequestClient::socketFileExists() {
  struct stat buffer;
  std::string name;

  if (_sockFile.empty()) {
    logError(std::string("connectToDaemon - File Empty!"));
    return false;
  }
  if (_sockFile.find(IPC) == std::string::npos ||
      _sockFile.find(UNIX) == std::string::npos) {
    size_t pos = _sockFile.find(IPC) == std::string::npos ? 7 : 6;
    name = std::string(_sockFile.begin() + pos, _sockFile.end());

    _sockFile = IPC + name;
  } else {
    logError(std::string("connectToDaemon - Unknown protocol for serverurl " +
                         _sockFile));
    return false;
  }
  if (stat(name.c_str(), &buffer) == -1) {
    logError(std::string("connectToDaemon - No such file! - " + _sockFile));
    return false;
  }
  return true;
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
  if (nn_send(_sockFd, Commands::PING, 4, 0) < 4) {
    _state = State::idle;
    logError("isConnectionAlive - Connection refused!");
    return false;
  }

  std::string answer;

  if (!tryRecv(answer, Commands::PONG)) {
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

  std::string answer;

  if (!tryRecv(answer)) {
    cleanUp();
    error = -1;
    return "";
  }

  return answer;
}

bool RequestClient::tryRecv(std::string &answer, const std::string &expected) {

  void *buffer = nullptr;
  int bytes = nn_recv(_sockFd, &buffer, NN_MSG, 0);

  if (bytes >= 0 && buffer != nullptr) {
    answer = std::string(static_cast<char *>(buffer), bytes);
    nn_freemsg(buffer);
    buffer = nullptr;

    if (expected.empty() || expected == answer) {
      return true;
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
