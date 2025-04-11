#include "Client/Console/Console.hpp"
#include <iostream>
#include <readline/history.h>
#include <readline/readline.h>

Console &Console::Instance() {
  static Console _instance;

  return _instance;
}

Console::Console() { rl_callback_handler_install(">>>", &Console::handler); }

Console::~Console() {}

void Console::cleanUp() {
  rl_callback_handler_remove();
  clear_history();
  rl_clear_history();
}

void Console::handler(char *line) {
  // Console &instance = Console::Instance();

  if (!line) {
    // Do something here
  } else {
    std::string str(line);
    free(line);
    if (!str.empty() && str.back() == '\n') {
      str.pop_back();
    }
    if (!str.empty()) {
      add_history(str.c_str());
    }
    std::cout << str << std::endl;
    rl_replace_line("", 0);
    rl_on_new_line();
    // Launch CommandHandler function here
  }
}

bool Console::registerCmd(const std::string &name, CommandHandler handler) {
  if (_commands.find(name) != _commands.end()) {
    std::cerr << "[Debug] - Console::registerCmd  - Command already exist!"
              << std::endl;
    return false;
  }
  _commands[name] = handler;
  return true;
}

void Console::readCharRead() { rl_callback_read_char(); }

char *Console::commandGenerator(const char *text, int state) {
  static std::map<std::string, CommandHandler>::const_iterator it;
  static std::map<std::string, CommandHandler>::const_iterator end;
  static size_t len;

  auto commands = Console::Instance().getCommands();
  if (state == 0) {
    it = commands.cbegin();
    end = commands.cend();
    len = strlen(text);
  }

  while (it != end) {
    const std::string &cmd = it->first;
    ++it;

    if (cmd.compare(0, len, text) == 0)
      return strdup(cmd.c_str());
  }

  return nullptr;
}

char **Console::completionHook(const char *text, int start, int end) {
  (void)end;
  if (start == 0)
    return rl_completion_matches(text, Console::commandGenerator);
  return nullptr;
}

Console::CommandMap const &Console::getCommands() const {
  return Console::Instance()._commands;
}
