#include "common/Utils.hpp"
#include <algorithm>

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

// trim from start and end
void Utils::trim(std::string &s) {
  ltrim(s);
  rtrim(s);
}

// trim from start (in place)
void Utils::ltrim(std::string &s) {
  s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
            return !std::isspace(ch);
          }));
}

// trim from end (in place)
void Utils::rtrim(std::string &s) {
  s.erase(std::find_if(s.rbegin(), s.rend(),
                       [](unsigned char ch) { return !std::isspace(ch); })
              .base(),
          s.end());
}
