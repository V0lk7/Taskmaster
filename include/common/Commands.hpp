#ifndef COMMANDS_HPP
#define COMMANDS_HPP

class Commands {
public:
  enum class CMD { status, start, stop, restart, reload, quit };

  static constexpr auto STATUS = "status";
  static constexpr auto START = "start";
  static constexpr auto STOP = "stop";
  static constexpr auto RESTART = "restart";
  static constexpr auto RELOAD = "reload";
  static constexpr auto QUIT = "quit";
  static constexpr auto PING = "ping";
  static constexpr auto PONG = "pong";
};

#endif
