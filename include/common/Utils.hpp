#ifndef UTILS_HPP
#define UTILS_HPP

#include "pch.hpp"

extern char **environ;

class Utils {
public:
  static std::vector<std::string> split(std::string const &input,
                                        std::string const &delimiter);

  static void trim(std::string &s);
  static void rtrim(std::string &s);
  static void ltrim(std::string &s);

private:
  Utils() = delete;
  Utils(Utils &) = delete;
  Utils(Utils &&) = delete;
  Utils &operator=(Utils &) = delete;
  Utils &operator=(Utils &&) = delete;
};

#endif
