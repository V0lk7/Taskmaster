#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "pch.hpp" // IWYU pragma: keep

#include "Epoll/Epoll.hpp"

#define MAX_EVENTS 2
#define TIMEOUT 100 // ms

class Console;

class Client {
  friend class TestClient;

public:
  enum class State { idle, setup, running, asking, error, exit };

  static Client &Instance();
  ~Client();

  bool setupClient(std::string const &conf);
  bool run();
  void cleanUp();

private:
  State _state = State::idle;

  Console &_console;
  Epoll _epoll;

  bool setUpSigaction();
  static void signalHandler(int);

  bool registerCommands();

  void cmdStatus(std::vector<std::string> &args);
  void cmdStart(std::vector<std::string> &args);
  void cmdStop(std::vector<std::string> &args);
  void cmdRestart(std::vector<std::string> &args);
  void cmdReload(std::vector<std::string> &args);
  void cmdQuit(std::vector<std::string> &args);

  bool askUserConfirmation(std::string const &);

  Client();
  Client(Client &) = delete;
  Client(Client &&) = delete;
  Client &operator=(Client &) = delete;
  Client &operator=(Client &&) = delete;
};

#endif
