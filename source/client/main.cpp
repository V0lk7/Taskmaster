#include "Client/Client.hpp"

#include <iostream>

int main(int ac, char *av[]) {
  if (ac != 1) {
    std::cerr << "[Error] - Main - Configuration file path not given!"
              << std::endl;
    return 1;
  }
  Client &instance = Client::Instance();

  if (!instance.setupClient(av[0])) {
    return 1;
  }
  if (!instance.run()) {
    return 1;
  }
  return 0;
}
