#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "Console/Console.hpp"
#include "Epoll/Epoll.hpp"
#include "RequestClient/RequestClient.hpp"
#include "common/Commands.hpp"
#include "pch.hpp" // IWYU pragma: keep

class Client {
  friend class ClientTest;

public:
  enum class State { idle, setup, running, waitingReply, asking, error, exit };

  static Client &Instance();
  ~Client();

  bool setupClient(std::string const &conf);
  bool run();

private:
  struct CmdRequest {
    Commands::CMD cmd;
    std::vector<std::string> args;
    std::string daemonAnswer;
  };

  State _state = State::idle;
  bool _firstRun = true;

  Console &_console;
  Epoll _epoll;
  RequestClient _request;
  std::string _userAnswer;

  Commands::CMD _currentCmd = Commands::CMD::none;

  std::queue<CmdRequest> _cmdQueue;
  CmdRequest _currentCmdRequest;

  static constexpr int TIMEOUT = 100;
  static constexpr int MAX_EVENTS = 2;

  std::string extractSocket(std::string const &, bool &);

  bool setUpSigaction();
  static void signalHandler(int);

  void addCmdToQueue(std::vector<std::string> &);

  bool sendCmd(const std::string &, std::vector<std::string> &);
  std::vector<std::string> processArgs(std::vector<std::string> &);

  bool processCmd();
  void cmdQuit(CmdRequest &);
  void cmdExit(CmdRequest &);
  bool cmdStatus(CmdRequest &);
  bool cmdStart(CmdRequest &);
  bool cmdStop(CmdRequest &);
  bool cmdRestart(CmdRequest &);
  bool cmdReload(CmdRequest &);
  void cmdHelp(CmdRequest &);

  void processReply(const Commands::CMD &, const std::string &);
  void cmdStatusAnswer(std::string const &);
  void cmdStartAnswer(std::string const &);
  void cmdStopAnswer(std::string const &);
  void cmdRestartAnswer(std::string const &);
  void cmdReloadAnswer(std::string const &);

  bool askUserConfirmation(std::string const &);

  void
  displayProcessList(const std::map<std::string, std::vector<ProcessInfo>> &);

  void cmdErrorMsg(Commands::CMD const &,
                   std::string const name = "Error") const;
  void logError(const std::string &msg, const int &error = -1);

  Client();
  Client(Client &) = delete;
  Client(Client &&) = delete;
  Client &operator=(Client &) = delete;
  Client &operator=(Client &&) = delete;
};

#endif
