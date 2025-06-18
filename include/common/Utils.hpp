#ifndef UTILS_HPP
#define UTILS_HPP

#include "pch.hpp" // IWYU pragma: keep

extern char **environ;

class Utils {
public:
  static std::vector<std::string> split(std::string const &input,
                                        std::string const &delimiter);

  static void trim(std::string &s);
  static void rtrim(std::string &s);
  static void ltrim(std::string &s);

  static std::string concat(std::vector<std::string> &vec,
                            std::string delimiter = "");

private:
  Utils() = delete;
  Utils(Utils &) = delete;
  Utils(Utils &&) = delete;
  Utils &operator=(Utils &) = delete;
  Utils &operator=(Utils &&) = delete;
};

#endif
