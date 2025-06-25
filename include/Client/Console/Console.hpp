#ifndef CONSOLE_HPP
#define CONSOLE_HPP

#include "Client/ProcessInfo.hpp"

#include "pch.hpp" // IWYU pragma: keep

class Console {
  friend class ClientTest;

public:
  enum class State { normal, question };

  using CommandHandler = std::function<void(std::vector<std::string> &args)>;
  using answerFunction = std::function<void(std::string answer)>;

  static Console &Instance();
  ~Console();

  void setReadline();
  void disableHandler();
  void enableHandler();

  void readCharRead(void);
  void resetPrompt();
  void clearPrompt();

  void setQuestionState(std::string const &, answerFunction);

  State const &getState() const;
  std::vector<std::string> const &getProcessList() const;
  void setProcessList(std::map<std::string, std::vector<ProcessInfo>> const &);

  void setCommandHandler(CommandHandler &handler);

  void setEOFHandler(std::function<void()> handler);

private:
  std::string _prompt = "";
  State _state = State::normal;
  answerFunction _ansFuction;
  CommandHandler _cmdHandler;
  std::function<void()> _eofHandler;
  bool _handlerEnabled = false;

  static std::vector<std::string> _processList;
  static const std::vector<std::string> _commands;

  static void handler(char *);
  static void normalState(Console &instance, char *line);
  static void questionState(Console &instance, char *line);
  static char *argGenerator(const char *, int);
  static char *commandGenerator(const char *, int);
  static char **completionHook(const char *, int, int);

  Console();
  Console(Console &) = delete;
  Console(Console &&) = delete;
  Console &operator=(Console &) = delete;
  Console &operator=(Console &&) = delete;
};

#endif
