#ifndef PROCESSINFO_HPP
#define PROCESSINFO_HPP

#include "pch.hpp" // IWYU pragma: keep

struct ProcessInfo {
  ProcessInfo(std::string const &name, std::string const &status,
              std::string const &message)
      : name(name), status(status), message(message) {}
  std::string name;
  std::string status;
  std::string message;
};

#endif
