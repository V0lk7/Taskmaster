#include "Client/Console/Console.hpp"
#include "common/Utils.hpp"
#include <iostream>
#include <readline/history.h>
#include <readline/readline.h>
#include <string>
#include <vector>

Console &Console::Instance() {
  static Console _instance;

  return _instance;
}

Console::Console() {
  rl_attempted_completion_function = &Console::completionHook;
  rl_callback_handler_install(">>>", &Console::handler);
}

Console::~Console() {}

void Console::cleanUp() {
  rl_callback_handler_remove();
  clear_history();
  rl_clear_history();
}

void Console::handler(char *line) {
  Console &instance = Console::Instance();
  CommandHandler function;
  CommandMap const &commands = instance.getCommands();
  CommandMap::const_iterator it;
  std::vector<std::string> tokens;

  if (!line) {
    // Do something here
    it = commands.find("quit");

    if (it == commands.cend()) {
      return;
    }
    function = it->second;
    function(tokens);
  } else {
    std::string str(line);
    free(line);
    if (str.back() == '\n') {
      str.pop_back();
    }
    if (!str.empty()) {
      add_history(str.c_str());
    }

    tokens = Utils::split(str, " ");

    if (!tokens.empty()) {
      it = commands.find(tokens[0]);

      if (it == commands.cend()) {
        std::cout << "Command not found!" << std::endl;
      } else {
        it->second(tokens);
      }

      rl_replace_line("", 0);
      rl_on_new_line();
      // Launch CommandHandler function here
    }
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
  static std::vector<std::string> matches;
  static size_t matchIndex;

  if (state == 0) {
    matches.clear();
    matchIndex = 0;

    const auto &commands = Console::Instance().getCommands();
    size_t len = strlen(text);

    for (const auto &pair : commands) {
      if (pair.first.compare(0, len, text) == 0) {
        matches.push_back(pair.first);
      }
    }
  }

  if (matchIndex < matches.size()) {
    // rl_completion_matches attend un strdup() car elle free le résultat
    return strdup(matches[matchIndex++].c_str());
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
