#include "Client/RequestClient/RequestClient.hpp"
#include "common/Utils.hpp"

constexpr char RequestClient::IPC[];
constexpr char RequestClient::UNIX[];

RequestClient::RequestClient() : _sockFd(-1), _state(State::idle) {}

RequestClient::~RequestClient() {
  if (_sockFd > -1) {
    nn_close(_sockFd);
  }
}

void RequestClient::setSockFile(std::string const &src) {
  if (src != _sockFile) {
    _sockFile = src;
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
  size_t sz = sizeof(_rcvFd);

  if (nn_getsockopt(_sockFd, NN_SOL_SOCKET, NN_RCVFD, &_rcvFd, &sz) == -1) {
    logError("connectToDaemon - nn_getsockopt() failed", errno);
    return false;
  }
  _state = State::connected;
  if (_addFdToEpoll) {
    _addFdToEpoll();
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

bool RequestClient::sendMsg(std::vector<std::string> const &msg) {
  if (_state == State::idle) {
    if (!connectToSocket()) {
      if (_removeFdFromEpoll) {
        _removeFdFromEpoll();
      }
      return false;
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
    if (_removeFdFromEpoll) {
      _removeFdFromEpoll();
    }
    _state = State::idle;
    logError("Connection refused ! - ", errno);
    return false;
  }

  _state = State::waitingResponse;
  return true;
}

bool RequestClient::receiveMsg(std::string &answer) {
  if (_state == State::idle || _state != State::waitingResponse) {
    logError(
        "receiveMsg - Client not connected or not waiting for a response.");
    return false;
  }

  void *buffer = nullptr;
  int bytes = 0;

  bytes = nn_recv(_sockFd, &buffer, NN_MSG, 0);

  switch (bytes) {
  case -1:
    if (_removeFdFromEpoll) {
      _removeFdFromEpoll();
    }
    return false;
  default:
    answer = std::string(static_cast<char *>(buffer), bytes);
    nn_freemsg(buffer);
    buffer = nullptr;
    _state = State::connected;
    return true;
  }
}

void RequestClient::logError(const std::string &msg, const int &error) {
  std::cerr << "[Error] - RequestClient::" << msg;
  if (error != -1) {
    std::cerr << " : errno = " << error << std::endl;
  } else {
    std::cerr << std::endl;
  }
}

void RequestClient::setAddFdToEpoll(std::function<void()> const &addFdToEpoll) {
  _addFdToEpoll = addFdToEpoll;
}

void RequestClient::setRemoveFdFromEpoll(
    std::function<void()> const &removeFdFromEpoll) {
  _removeFdFromEpoll = removeFdFromEpoll;
}
