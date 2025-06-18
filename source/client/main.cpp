#include "pch.hpp" // IWYU pragma: keep

#include "Client/Client.hpp"

static bool dropToNobody();

int main(int ac, char *av[]) {
  if (!dropToNobody()) {
    std::cerr << "[Error] - Can\'t drop privileges!" << std::endl;
    return 1;
  }
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

static bool dropToNobody() {
  if (getuid() == 0) {
    if (setgid(getgid()) != 0) {
      return false;
    }
    if (setuid(getuid()) != 0) {
      return false;
    }
  }

  return true;
}
