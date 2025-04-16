#include "common/Utils.hpp"

std::vector<std::string> Utils::split(std::string const &input,
                                      std::string const &delimiter) {
  std::vector<std::string> tokens;
  size_t start = 0;
  size_t end;

  while ((end = input.find(delimiter, start)) != std::string::npos) {
    if (end != start) {
      tokens.push_back(input.substr(start, end - start));
    }
    start = end + delimiter.length();
  }

  if (start < input.length()) {
    tokens.push_back(input.substr(start));
  }
  return tokens;
}
