#ifndef CONSOLE_HPP
#define CONSOLE_HPP

#include <functional>
#include <map>
#include <string>
#include <vector>

class Console {
public:
  using CommandHandler = std::function<void(std::vector<std::string> &args)>;
  using CommandMap = std::map<std::string, CommandHandler>;

  static Console &Instance();
  ~Console();

  bool registerCmd(const std::string &, CommandHandler);
  void readCharRead(void);
  void cleanUp();

  CommandMap const &getCommands() const;

private:
  std::string _prompt;
  CommandMap _commands;

  static void handler(char *);
  static char *commandGenerator(const char *, int);
  static char **completionHook(const char *, int, int);

  Console();
  Console(Console &) = delete;
  Console(Console &&) = delete;
  Console &operator=(Console &) = delete;
  Console &operator=(Console &&) = delete;
};

#endif
