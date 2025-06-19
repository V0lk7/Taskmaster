#ifndef CONSOLE_HPP
#define CONSOLE_HPP

#include "Client/ProcessInfo.hpp"

#include "pch.hpp" // IWYU pragma: keep

class Console {
public:
  enum class State { normal, question };

  using CommandHandler = std::function<void(std::vector<std::string> &args)>;
  using answerFunction = std::function<void(std::string answer)>;
  using CommandMap = std::map<std::string, CommandHandler>;

  static Console &Instance();
  ~Console();

  void setReadline();
  void disableHandler();
  void enableHandler();
  bool registerCmd(const std::string &, CommandHandler);
  void readCharRead(void);
  void cleanUp();

  CommandMap const &getCommands() const;
  void setQuestionState(std::string const &, answerFunction);
  State const &getState() const;
  void setProcessList(std::map<std::string, std::vector<ProcessInfo>> const &);

private:
  std::string _prompt = "";
  CommandMap _commands = {};
  State _state = State::normal;
  answerFunction _ansFuction;

  static std::vector<std::string> _processList;

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
