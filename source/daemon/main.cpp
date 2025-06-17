#include "pch.hpp"

#include "daemon/parsing/parsing.hpp"

static void signalHandler(int signal);
static bool setUpSignalHandler();

std::atomic<Daemon *> daemonHandler{nullptr};

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
  if (!setUpSignalHandler()) {
    std::cerr << "Error: Internal error state! Can\'t configure signals."
              << std::endl;
  }
  try {
    Daemon *daemon = parsingFile(config_file);

    daemonHandler.store(daemon);

    daemon->start();

  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    syslog(LOG_ERR, "Error: %s", e.what());
    return 1;
  }
  return 0;
}

static bool setUpSignalHandler() {
  struct sigaction sa;

  sa.sa_handler = &signalHandler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;

  if (sigaction(SIGINT, &sa, nullptr) < 0 ||
      sigaction(SIGQUIT, &sa, nullptr) < 0 ||
      sigaction(SIGHUP, &sa, nullptr) < 0) {
    return false;
  }

  return true;
}

static void signalHandler(int signal) {
  Daemon *daemon = daemonHandler.load();

  if (signal == SIGINT || signal == SIGQUIT) {
    daemon->sendLogs("Daemon shutting down.", "INFO");
    daemon->clean();
    delete daemon;
    exit(0);
  } else {
    std::cout << "SIGHUP called" << std::endl;
  }
}
