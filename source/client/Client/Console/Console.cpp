#include "Client/Console/Console.hpp"
#include "common/Commands.hpp"
#include "common/Utils.hpp"
#include <cstdio>
#include <readline/readline.h>

std::vector<std::string> Console::_processList = {
    "arg1", "arg2",
    "arg3"}; // TODO modify this list when status is first called

Console &Console::Instance() {
  static Console _instance;

  return _instance;
}

Console::Console() {
  rl_attempted_completion_function = &Console::completionHook;

  rl_callback_handler_install(">>> ", &Console::handler);
}

Console::~Console() {}

void Console::disableHandler() { rl_callback_handler_remove(); }

void Console::enableHandler() {
  rl_callback_handler_install(">>> ", &Console::handler);
}

void Console::cleanUp() {
  rl_callback_handler_remove();
  clear_history();
  rl_clear_history();
}

void Console::handler(char *line) {
  Console &instance = Console::Instance();

  switch (instance.getState()) {
  case State::question:
    instance.questionState(instance, line);
    break;
  default:
    instance.normalState(instance, line);
    break;
  }
  if (line) {
    free(line);
  }
}

void Console::normalState(Console &instance, char *line) {
  CommandHandler function;
  CommandMap const &commands = instance.getCommands();
  CommandMap::const_iterator it;
  std::vector<std::string> tokens = {};

  if (!line) {
    it = commands.find("quit");

    if (it == commands.cend()) {
      return;
    }
    function = it->second;
    function(tokens);
    rl_callback_handler_remove();
  } else {
    std::string str(line);

    Utils::trim(str);
    if (!str.empty()) {
      add_history(str.c_str());

      tokens = Utils::split(str, " ");

      if (!tokens.empty()) {
        it = commands.find(tokens[0]);

        if (it == commands.cend()) {
          std::cout << "*** Unknown syntax: " << tokens[0] << std::endl;
        } else {
          tokens.erase(tokens.begin());
          it->second(tokens);
        }
      }
    }
    rl_replace_line("", 0);
    rl_on_new_line();
  }
}

void Console::questionState(Console &instance, char *line) {
  std::string str("");

  if (line) {
    str = line;
    instance._ansFuction(str);
  } else {
    CommandMap const &commands = instance.getCommands();
    auto it = commands.find("quit");
    std::vector<std::string> tokens = {};

    if (it == commands.cend()) {
      return;
    }
    CommandHandler function = it->second;

    function(tokens);
    rl_callback_handler_remove();
  }

  rl_set_prompt(">>> ");
  rl_replace_line("", 0);
  rl_on_new_line();
  instance._state = State::normal;
}

void Console::setQuestionState(std::string const &newPrompt,
                               answerFunction function) {
  Console &instance = Console::Instance();

  instance._state = State::question;
  instance._ansFuction = function;
  rl_set_prompt(newPrompt.c_str());
  rl_replace_line("", 0);
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

char **Console::completionHook(const char *text, int start, int end) {
  (void)end;
  char **matches = nullptr;

  if (start == 0) {
    matches = rl_completion_matches(text, Console::commandGenerator);
  } else {
    std::string buffer(rl_line_buffer);
    std::string cmd = buffer.substr(0, buffer.find(' '));

    if (cmd == RELOAD || cmd == QUIT) {
      matches = nullptr;
    } else if (cmd == STATUS || cmd == START || cmd == STOP || cmd == RESTART) {
      matches = rl_completion_matches(text, Console::argGenerator);
    }
  }
  if (matches == nullptr) {
    matches = (char **)malloc(2 * sizeof(char *));
    matches[0] = strdup("");
    matches[1] = nullptr;
  }

  return matches;
}

char *Console::argGenerator(const char *text, int state) {
  static std::vector<std::string> filteredArgs;
  static size_t matchIndex;
  static size_t len;

  if (state == 0) {
    matchIndex = 0;
    len = strlen(text);
    filteredArgs.clear();

    // Extract already-used args from rl_line_buffer
    std::istringstream iss(rl_line_buffer);
    std::string word, command;
    iss >> command; // Skip command

    std::unordered_set<std::string> usedArgs;
    while (iss >> word) {
      usedArgs.insert(word);
    }

    // Filter out used args
    for (const auto &arg : _processList) {
      if (usedArgs.find(arg) == usedArgs.end()) {
        if (arg.compare(0, len, text) == 0) {
          filteredArgs.push_back(arg);
        }
      }
    }
  }

  if (matchIndex < filteredArgs.size()) {
    return strdup(filteredArgs[matchIndex++].c_str());
  }
  return nullptr;
}

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
    return strdup(matches[matchIndex++].c_str());
  }

  return nullptr;
}

Console::CommandMap const &Console::getCommands() const {
  return Console::Instance()._commands;
}

Console::State const &Console::getState() const {
  return Console::Instance()._state;
}
