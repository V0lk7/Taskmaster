#include <iostream>
#include "daemon/parsing/parsing.hpp"

int main(int argc, char **argv) {
  if (argc < 2) {
	std::cerr << "Usage: " << argv[0] << " <config_file>" << std::endl;
	return 1;
  }
  std::string config_file = argv[1];
  if (config_file.empty()) {
	std::cerr << "Error: Config file path is empty." << std::endl;
	return 1;
  }
  if (access(config_file.c_str(), R_OK) == -1) {
	std::cerr << "Error: Unable to access config file." << std::endl;
	return 1;
  }
  int val = parsingFile(config_file);
  std::cout << "End." << val << std::endl;
  return 0;
}
