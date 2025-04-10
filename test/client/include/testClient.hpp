#include "Client/Client.hpp"

class TestClient {
public:
  static bool callSetupEpoll(Client &client) { return client.setupEpoll(); }

  static bool callSetupReadline(Client &client) {
    return client.setupReadline();
  }

  static bool callAddFdToEpoll(Client &client, int fd, uint32_t events) {
    return client.addFdToEpoll(fd, events);
  }

  static bool callMakeFdNonBlocking(Client &client, int fd) {
    return client.makeFdNonBlocking(fd);
  }

  static void callReadlineHandler(Client &client, const std::string &input) {
    client.readlineHandler(input);
  }

  static bool callSetUpSigaction(Client &client) {
    return client.setUpSigaction();
  }

  static void callSetIsRunning(Client &client, bool val) {
    return client.setIsRunning(val);
  }
};
