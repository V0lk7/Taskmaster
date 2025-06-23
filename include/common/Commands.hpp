#ifndef COMMANDS_HPP
#define COMMANDS_HPP

#include "pch.hpp" // IWYU pragma: keep

class Commands {
public:
  enum class CMD {
    status,
    start,
    stop,
    restart,
    reload,
    quit,
    exit,
    help,
    none
  };

  static constexpr auto STATUS = "status";
  static constexpr auto START = "start";
  static constexpr auto STOP = "stop";
  static constexpr auto RESTART = "restart";
  static constexpr auto RELOAD = "reload";
  static constexpr auto QUIT = "quit";
  static constexpr auto HELP = "help";
  static constexpr auto EXIT = "exit";

  static CMD getCommand(const std::string &cmd) {
    if (cmd == STATUS) {
      return CMD::status;
    } else if (cmd == START) {
      return CMD::start;
    } else if (cmd == STOP) {
      return CMD::stop;
    } else if (cmd == RESTART) {
      return CMD::restart;
    } else if (cmd == RELOAD) {
      return CMD::reload;
    } else if (cmd == QUIT) {
      return CMD::quit;
    } else if (cmd == EXIT) {
      return CMD::exit;
    } else if (cmd == HELP) {
      return CMD::help;
    }
    return CMD::none;
  }

  static std::string cmdToString(CMD &cmd) {
    switch (cmd) {
    case CMD::status:
      return STATUS;
    case CMD::start:
      return START;
    case CMD::stop:
      return STOP;
    case CMD::restart:
      return RESTART;
    case CMD::reload:
      return RELOAD;
    case CMD::quit:
      return QUIT;
    case CMD::exit:
      return EXIT;
    case CMD::help:
      return HELP;
    default:
      return "";
    }
  }
};

#endif
