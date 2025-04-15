#include "parsing.hpp"

void send_syslog(int log_level, const std::string& message) {
	openlog("taskmasterd", LOG_PID | LOG_CONS, LOG_DAEMON);
	syslog(log_level, "%s", message.c_str());
	closelog();
}

std::string parsingSocket(YAML::Node unix_http_server)
{
	YAML::Node path = unix_http_server["file"];
	if (!path) {
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
	// else
	//   throw std::runtime_error("Error: Invalid 'loglevel' value.");

}

std::vector<Process> parsingProcess(YAML::Node process) {
	YAML::Node name = process["name"];
	if (!name) {
	  syslog(LOG_ERR, "Error: 'name' not found in 'process'.");
	  throw std::runtime_error("Error: 'name' not found in 'process'.");
	}
	std::string process_name = name.as<std::string>();
	if (process_name.empty()) {
	  syslog(LOG_ERR, "Error: 'name' is empty.");
	  throw std::runtime_error("Error: 'name' is empty.");
	}

	YAML::Node command = process["command"];
	if (!command) {
	  syslog(LOG_ERR, "Error: 'command' not found in 'process'.");
	  throw std::runtime_error("Error: 'command' not found in 'process'.");
	}
	std::string process_command = command.as<std::string>();
	if (process_command.empty()) {
	  syslog(LOG_ERR, "Error: 'command' is empty.");
	  throw std::runtime_error("Error: 'command' is empty.");
	}

	YAML::Node directory = process["directory"];
	if (!directory) {
	  syslog(LOG_ERR, "Error: 'directory' not found in 'process'.");
	  throw std::runtime_error("Error: 'directory' not found in 'process'.");
	}
	std::string process_directory = directory.as<std::string>();
	if (process_directory.empty()) {
	  syslog(LOG_ERR, "Error: 'directory' is empty.");
	  throw std::runtime_error("Error: 'directory' is empty.");
	}

	YAML::Node numprocs = process["numprocs"];
	if (!numprocs) {
	  syslog(LOG_ERR, "Error: 'numprocs' not found in 'process'.");
	  throw std::runtime_error("Error: 'numprocs' not found in 'process'.");
	}
	int process_numprocs = numprocs.as<int>();
	if (process_numprocs <= 0) {
	  syslog(LOG_ERR, "Error: 'numprocs' must be greater than 0.");
	  throw std::runtime_error("Error: 'numprocs' must be greater than 0.");
	}

	YAML::Node autostart = process["autostart"];
	if (!autostart) {
	  syslog(LOG_ERR, "Error: 'autostart' not found in 'process'.");
	  throw std::runtime_error("Error: 'autostart' not found in 'process'.");
	}
	bool process_autostart = autostart.as<bool>();

	YAML::Node autorestart = process["autorestart"];
	if (!autorestart) {
	  syslog(LOG_ERR, "Error: 'autorestart' not found in 'process'.");
	  throw std::runtime_error("Error: 'autorestart' not found in 'process'.");
	}
	std::string process_autorestart = autorestart.as<std::string>();

	YAML::Node exitcodes = process["exitcodes"];
	if (!exitcodes) {
	  syslog(LOG_ERR, "Error: 'exitcodes' not found in 'process'.");
	  throw std::runtime_error("Error: 'exitcodes' not found in 'process'.");
	}
	std::vector<int> process_exitcodes;
	for (const auto& exitcode : exitcodes) {
	  process_exitcodes.push_back(exitcode.as<int>());
	}

	YAML::Node starttime = process["starttime"];
	if (!starttime) {
	  syslog(LOG_ERR, "Error: 'starttime' not found in 'process'.");
	  throw std::runtime_error("Error: 'starttime' not found in 'process'.");
	}
	int process_starttime = starttime.as<int>();

	YAML::Node startretries = process["startretries"];
	if (!startretries) {
	  syslog(LOG_ERR, "Error: 'startretries' not found in 'process'.");
	  throw std::runtime_error("Error: 'startretries' not found in 'process'.");
	}
	int process_startretries = startretries.as<int>();

	YAML::Node stopsignal = process["stopsignal"];
	if (!stopsignal) {
	  syslog(LOG_ERR, "Error: 'stopsignal' not found in 'process'.");
	  throw std::runtime_error("Error: 'stopsignal' not found in 'process'.");
	}
	int process_stopsignal = stopsignal.as<int>();

	YAML::Node stopwait = process["stopwait"];
	if (!stopwait) {
	  syslog(LOG_ERR, "Error: 'stopwait' not found in 'process'.");
	  throw std::runtime_error("Error: 'stopwait' not found in 'process'.");
	}
	int process_stopwait = stopwait.as<int>();

	YAML::Node environment = process["environment"];
	if (!environment) {
	  syslog(LOG_ERR, "Error: 'environment' not found in 'process'.");
	  throw std::runtime_error("Error: 'environment' not found in 'process'.");
	}
	std::map<std::string, std::string> process_environment;
	for (const auto& env : environment) {
	  process_environment[env.first.as<std::string>()] = env.second.as<std::string>();
	}

	YAML::Node stdout_logfile = process["stdout_logfile"];
	if (!stdout_logfile) {
	  syslog(LOG_ERR, "Error: 'stdout_logfile' not found in 'process'.");
	  throw std::runtime_error("Error: 'stdout_logfile' not found in 'process'.");
	}
	std::string process_stdout_logfile = stdout_logfile.as<std::string>();

	YAML::Node stderr_logfile = process["stderr_logfile"];
	if (!stderr_logfile) {
	  syslog(LOG_ERR, "Error: 'stderr_logfile' not found in 'process'.");
	  throw std::runtime_error("Error: 'stderr_logfile' not found in 'process'.");
	}
	std::string process_stderr_logfile = stderr_logfile.as<std::string>();
	YAML::Node umask = process["umask"];
	if (!umask) {
	  syslog(LOG_ERR, "Error: 'umask' not found in 'process'.");
	  throw std::runtime_error("Error: 'umask' not found in 'process'.");
	}
	mode_t process_umask = umask.as<mode_t>();
	Process process(process_name, process_command, process_directory,
		process_numprocs, process_autostart, convertStringToRestart(process_autorestart),
		process_exitcodes, process_starttime, process_startretries,
		process_stopsignal, process_stopwait, process_stdout_logfile,
		process_stderr_logfile, process_umask, process_environment);
}

int parsingFile(std::string config_file) {
	YAML::Node config = YAML::LoadFile(config_file);
	if (!config) {
	  std::cerr << "Error: Unable to load config file." << std::endl;
	  return 1;
	}
	try {
		std::string socket_path = parsingSocket(config["unix_http_server"]);
		Log log_info = parsingTaskmasterd(config["taskmasterd"]);
		std::vector<Process> processes = parsingProcess(config["process"]);

	}
	catch (const std::runtime_error& e) {
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	}
	return 0;
  }
