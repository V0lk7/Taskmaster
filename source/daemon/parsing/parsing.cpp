#include "daemon/parsing/parsing.hpp"

void send_syslog(int log_level, const std::string &message) {
  openlog("taskmasterd", LOG_PID | LOG_CONS, LOG_DAEMON);
  syslog(log_level, "%s", message.c_str());
  closelog();
}

std::string parsingSocket(YAML::Node unix_http_server) {
  YAML::Node path = unix_http_server["file"];
  if (!path) {
    send_syslog(LOG_ERR, "Error: 'file' not found in 'unix_http_server'.");
    throw std::runtime_error("Error: 'file' not found in 'unix_http_server'.");
  }
  std::string socket_path = path.as<std::string>();
  return socket_path;
}

Log parsingTaskmasterd(YAML::Node taskmasterd) {
  YAML::Node logfile = taskmasterd["logfile"];
  if (!logfile) {
    send_syslog(LOG_ERR, "Error: 'logfile' not found in 'taskmasterd'.");
    throw std::runtime_error("Error: 'logfile' not found in 'taskmasterd'.");
  }
  std::string logfile_path = logfile.as<std::string>();
  if (logfile_path.empty()) {
    send_syslog(LOG_ERR, "Error: 'logfile' path is empty.");
    throw std::runtime_error("Error: 'logfile' path is empty.");
  }
  YAML::Node loglevel = taskmasterd["loglevel"];
  if (!loglevel) {
    send_syslog(LOG_ERR, "Error: 'loglevel' not found in 'taskmasterd'.");
    throw std::runtime_error("Error: 'loglevel' not found in 'taskmasterd'.");
  }
  std::string loglevel_str = loglevel.as<std::string>();
  if (loglevel_str.empty()) {
    send_syslog(LOG_ERR, "Error: 'loglevel' is empty.");
    throw std::runtime_error("Error: 'loglevel' is empty.");
  }
  Log::LogLevel level = convertStringToLogLevel(loglevel_str);
  Log log_info("taskmasterd", Log::Type::FILE, level, logfile_path);
  return log_info;
}

Daemon *parsingFile(std::string config_file) {
	std::cout << "Parsing config file..." << std::endl;
	YAML::Node config = YAML::LoadFile(config_file);
	if (!config) {
		throw std::runtime_error("Error: Unable to load config file.");
	}
		std::cout << "Parsing Socket" << std::endl;
		std::string socket_path = parsingSocket(config["unix_http_server"]);
		std::cout << "Parsing Taskmasterd" << std::endl;
		Log log_info = parsingTaskmasterd(config["taskmasterd"]);
		std::cout << "Parsing Process" << std::endl;
		std::vector<Program> programs = parsingPrograms(config["programs"]);
		std::cout << "Daemon creation" << std::endl;
		Daemon *daemon = new Daemon(socket_path, log_info);
		daemon->sendLogs("Daemon started.");
		for (auto &program : programs) {
			daemon->addProgram(program);
		}
		// daemon.printDaemon();
	return daemon;
}
