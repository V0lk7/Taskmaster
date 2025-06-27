#include "pch.hpp"

#include "daemon/parsing/parsing.hpp"
#include "daemon/parsing/reloadParsing.hpp"

static void signalHandler(int signal);
static bool setUpSignalHandler();
static void daemonize();

std::atomic<Daemon *> daemonHandler{nullptr};

int main(int argc, char **argv) {
  daemonize();
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
    delete daemon;
    exit(0);
  } else if (signal == SIGHUP) {
    daemon->sendLogs("Reloading configuration.", "INFO");
    reloadConf(daemon);
  }
}

static void daemonize() {
  pid_t pid = fork();

  if (pid < 0) {
    exit(EXIT_FAILURE);
  }
  if (pid > 0) {
    // Parent exits, child continues
    exit(EXIT_SUCCESS);
  }

  // Child becomes session leader
  if (setsid() < 0) {
    exit(EXIT_FAILURE);
  }

  if (!setUpSignalHandler()) {
    std::cerr << "Error: Internal error state! Can\'t configure signals."
              << std::endl;
    exit(EXIT_FAILURE);
  }

  pid = fork();
  if (pid < 0) {
    exit(EXIT_FAILURE);
  }
  if (pid > 0) {
    // Exit the first child
    exit(EXIT_SUCCESS);
  }

  // Close all open file descriptors
  for (int x = sysconf(_SC_OPEN_MAX); x >= 0; x--) {
    close(x);
  }

  // Redirect stdin, stdout, stderr to /dev/null
  open("/dev/null", O_RDWR); // stdin
  dup(0);                    // stdout
  dup(0);                    // stderr
}
