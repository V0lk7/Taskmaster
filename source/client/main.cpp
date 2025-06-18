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
  uid_t uid = getuid();

  if (uid == 0) {
    struct passwd *pw = getpwnam("nobody");

    std::cout << "????" << std::endl;

    if (pw) {
      uid = pw->pw_uid;
    } else {
      uid = 65534;
    }
  }

  if (setuid(uid) != 0) {
    std::cout << "aled" << std::endl;
    return false;
  }
  return true;
}
