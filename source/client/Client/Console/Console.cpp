#include "Client/Console/Console.hpp"
#include "common/Commands.hpp"
#include "common/Utils.hpp"

std::vector<std::string> Console::_processList = {};

const std::vector<std::string> Console::_commands = {
    Commands::RELOAD, Commands::QUIT,    Commands::STATUS, Commands::START,
    Commands::STOP,   Commands::RESTART, Commands::EXIT,   Commands::HELP};

Console &Console::Instance() {
  static Console _instance;

  return _instance;
}

Console::Console() {}

Console::~Console() {
  rl_callback_handler_remove();
  clear_history();
  rl_clear_history();
}

void Console::setProcessList(
    std::map<std::string, std::vector<ProcessInfo>> const &newProcessList) {
  _processList.clear();
  for (const auto &pair : newProcessList) {
    if (pair.second.size() == 1) {
      Console::_processList.push_back(pair.first);
    } else {
      _processList.push_back(pair.first + ":*");
      for (const auto &process : pair.second) {
        Console::_processList.push_back(pair.first + ":" + process.name);
      }
    }
  }
}

void Console::setReadline() {
  rl_attempted_completion_function = &Console::completionHook;

  rl_callback_handler_install(">>> ", &Console::handler);
  _handlerEnabled = true;
}

void Console::disableHandler() {
  if (!_handlerEnabled) {
    return;
  }
  rl_callback_handler_remove();
  _handlerEnabled = false;
}

void Console::enableHandler() {
  if (_handlerEnabled) {
    return;
  }
  rl_callback_handler_install(">>> ", &Console::handler);
  _handlerEnabled = true;
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
  CommandHandler function = instance._cmdHandler;
  std::vector<std::string> tokens = {};

  if (!line) {
    tokens.push_back(Commands::QUIT);
    function(tokens);
    return;
  }
  std::string str(line);

  Utils::trim(str);
  if (!str.empty()) {
    add_history(str.c_str());

    tokens = Utils::split(str, " ");

    function(tokens);
  }
}

void Console::resetPrompt() {
  rl_replace_line("", 0);
  rl_on_new_line();
  rl_redisplay();
}

void Console::clearPrompt() {
  rl_replace_line("", 0);
  rl_crlf();
}

void Console::questionState(Console &instance, char *line) {
  if (line) {
    std::string str(line);
    instance._ansFuction(str);
    rl_set_prompt(">>> ");
    rl_replace_line("", 0);
    rl_on_new_line();
    instance._state = State::normal;
  } else {
    instance._eofHandler();
  }
}

void Console::setQuestionState(std::string const &newPrompt,
                               answerFunction function) {
  Console &instance = Console::Instance();

  instance._state = State::question;
  instance._ansFuction = function;
  rl_set_prompt(newPrompt.c_str());
  rl_replace_line("", 0);
  rl_redisplay();
}

void Console::setCommandHandler(CommandHandler &handler) {
  _cmdHandler = handler;
}

void Console::readCharRead() {
  if (!_handlerEnabled) {
    return;
  }
  rl_callback_read_char();
}

char **Console::completionHook(const char *text, int start, int end) {
  (void)end;
  char **matches = nullptr;

  if (start == 0) {
    matches = rl_completion_matches(text, Console::commandGenerator);
  } else {
    std::string buffer(rl_line_buffer);
    std::string cmd = buffer.substr(0, buffer.find(' '));

    if (cmd == Commands::RELOAD || cmd == Commands::QUIT) {
      matches = nullptr;
    } else if (cmd == Commands::STATUS || cmd == Commands::START ||
               cmd == Commands::STOP || cmd == Commands::RESTART) {
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

    const auto &commands = Console::_commands;
    size_t len = strlen(text);

    for (const auto &cmd : commands) {
      if (cmd.compare(0, len, text) == 0) {
        matches.push_back(cmd);
      }
    }
  }

  if (matchIndex < matches.size()) {
    return strdup(matches[matchIndex++].c_str());
  }

  return nullptr;
}

Console::State const &Console::getState() const {
  return Console::Instance()._state;
}
