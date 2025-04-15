#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <vector>

class Utils {
public:
  static std::vector<std::string> split(std::string const &input,
                                        std::string const &delimiter);

private:
  Utils() = delete;
  Utils(Utils &) = delete;
  Utils(Utils &&) = delete;
  Utils &operator=(Utils &) = delete;
  Utils &operator=(Utils &&) = delete;
};

#endif
