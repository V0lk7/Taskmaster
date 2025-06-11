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
  try {
  	Daemon *daemon = parsingFile(config_file);
  	daemon->initialStart();
  	std::cout << "Daemon started." << std::endl;
	sleep(10);
	std::cout << daemon->stringStatusAllPrograms() << std::endl;;
	std::cout << "Daemon stopped." << std::endl;
	delete daemon;
  }
  catch (const std::exception &e) {
  	std::cerr << "Error: " << e.what() << std::endl;
  	return 1;
  }
  return 0;
}
