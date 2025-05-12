#include <iostream>
#include "parsing/parsing.hpp"

int main(int argc, char **argv) {
  if (argc < 2) {
	std::cerr << "Usage: " << argv[0] << " <config_file>" << std::endl;
	return 1;
  }
  std::string config_file = argv[1];
  parsingFile(config_file);
  std::cout << "End." << std::endl;
  return 0;
}
