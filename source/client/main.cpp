#include "pch.hpp" // IWYU pragma: keep

#include "Client/Client.hpp"

int main(int ac, char *av[]) {
  if (ac != 2) {
    std::cerr << "[Error] - Configuration file path not given!" << std::endl;
    return 1;
  }
  Client &instance = Client::Instance();

  if (!instance.setupClient(av[1])) {
    return 1;
  }
  if (!instance.run()) {
    return 1;
  }
  return 0;
}
