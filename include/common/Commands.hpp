#ifndef COMMANDS_HPP
#define COMMANDS_HPP

#include "pch.hpp" // IWYU pragma: keep

class Commands {
public:
  enum class CMD { status, start, stop, restart, reload, quit, none };

  static constexpr auto STATUS = "status";
  static constexpr auto START = "start";
  static constexpr auto STOP = "stop";
  static constexpr auto RESTART = "restart";
  static constexpr auto RELOAD = "reload";
  static constexpr auto QUIT = "quit";

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
    }
    return CMD::none;
  }
};

#endif
