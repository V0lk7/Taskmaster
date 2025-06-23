#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "Console/Console.hpp"
#include "Epoll/Epoll.hpp"
#include "RequestClient/RequestClient.hpp"
#include "common/Commands.hpp"
#include "pch.hpp" // IWYU pragma: keep

class Client {
public:
  enum class State { idle, setup, running, waitingReply, asking, error, exit };

  static Client &Instance();
  ~Client();

  bool setupClient(std::string const &conf);
  bool run();
  void cleanUp();

private:
  State _state = State::idle;

  Console &_console;
  Epoll _epoll;
  RequestClient _request;
  std::string _userAnswer;

  Commands::CMD _currentCmd = Commands::CMD::none;

  static constexpr int TIMEOUT = 100;
  static constexpr int MAX_EVENTS = 2;

  std::string extractSocket(std::string const &, bool &);

  bool setUpSigaction();
  static void signalHandler(int);

  // bool registerCommands();

  // void cmdStatus(std::vector<std::string> &args);
  // bool cmdStart(std::vector<std::string> &args);
  // bool cmdStop(std::vector<std::string> &args);
  // void cmdRestart(std::vector<std::string> &args);
  // void cmdReload(std::vector<std::string> &args);
  // void cmdQuit(std::vector<std::string> &args);
  
  void processReply(const std::string &);

  void cmdStatusAnswer(std::string const &);
  void cmdStartAnswer(std::string const &);
  void cmdStopAnswer(std::string const &);
  void cmdRestartAnswer(std::string const &);
  void cmdReloadAnswer(std::string const &);

  bool askUserConfirmation(std::string const &);

  void
  displayProcessList(const std::map<std::string, std::vector<ProcessInfo>> &);

  void cmdErrorMsg(Commands::CMD const &) const;
  void logError(const std::string &msg, const int &error = -1);

  Client();
  Client(Client &) = delete;
  Client(Client &&) = delete;
  Client &operator=(Client &) = delete;
  Client &operator=(Client &&) = delete;
};

#endif
