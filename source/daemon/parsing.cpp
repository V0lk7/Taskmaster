#include "parsing.hpp"
#include "process.hpp"

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

	Process program = Process(process_name, process_command);

	YAML::Node directory = process["directory"];
	if (directory) {
		std::string process_directory = directory.as<std::string>();
		if (process_directory.empty()) {
		  syslog(LOG_ERR, "Error: 'directory' is empty.");
		  throw std::runtime_error("Error: 'directory' is empty.");
		}
		program.setWorkdir(process_directory);
	}

	YAML::Node numprocs = process["numprocs"];
	if (numprocs) {
		int process_numprocs = numprocs.as<int>();
		if (process_numprocs <= 0) {
		  syslog(LOG_ERR, "Error: 'numprocs' must be greater than 0.");
		  throw std::runtime_error("Error: 'numprocs' must be greater than 0.");
		}
		program.setNbprocess(process_numprocs);
	}

	YAML::Node autostart = process["autostart"];
	if (autostart) {
		bool process_autostart = autostart.as<bool>();
		if (process_autostart != true && process_autostart != false) {
		  syslog(LOG_ERR, "Error: 'autostart' must be true or false.");
		  throw std::runtime_error("Error: 'autostart' must be true or false.");
		}
		program.setAutostart(process_autostart);
	}

	YAML::Node autorestart = process["autorestart"];
	if (autorestart) {
		std::string process_autorestart = autorestart.as<std::string>();
		if (process_autorestart != "true" && process_autorestart != "unexpected" && process_autorestart != "false") {
		  syslog(LOG_ERR, "Error: 'autorestart' must be 'true', 'unexpected' or 'false'.");
		  throw std::runtime_error("Error: 'autorestart' must be 'true', 'unexpected' or 'false'.");
		}
		program.setRestart(convertStringToRestart(process_autorestart));
	}

	YAML::Node exitcodes = process["exitcodes"];
	if (exitcodes) {
		std::vector<int> process_exitcodes;
		for (const auto& exitcode : exitcodes) {
		  process_exitcodes.push_back(exitcode.as<int>());
		}
		if (process_exitcodes.empty()) {
		  syslog(LOG_ERR, "Error: 'exitcodes' is empty.");
		  throw std::runtime_error("Error: 'exitcodes' is empty.");
		}
		program.setExitcodes(process_exitcodes);
	}

	YAML::Node starttime = process["starttime"];
	if (starttime) {
		int process_starttime = starttime.as<int>();
		if (process_starttime < 0) {
		  syslog(LOG_ERR, "Error: 'starttime' must be greater than or equal to 0.");
		  throw std::runtime_error("Error: 'starttime' must be greater than or equal to 0.");
		}
		program.setStartdelay(process_starttime);
	}

	YAML::Node startretries = process["startretries"];
	if (startretries) {
		int process_startretries = startretries.as<int>();
		if (process_startretries < 0) {
		  syslog(LOG_ERR, "Error: 'startretries' must be greater than or equal to 0.");
		  throw std::runtime_error("Error: 'startretries' must be greater than or equal to 0.");
		}
		program.setRestartretry(process_startretries);
	}

	YAML::Node stopsignal = process["stopsignal"];
	if (stopsignal) {
		int process_stopsignal = stopsignal.as<int>();
		if (process_stopsignal < 0) {
		  syslog(LOG_ERR, "Error: 'stopsignal' must be greater than or equal to 0.");
		  throw std::runtime_error("Error: 'stopsignal' must be greater than or equal to 0.");
		}
		program.setStopsignal(process_stopsignal);
	}

	YAML::Node stopwait = process["stopwait"];
	if (stopwait) {
		int process_stopwait = stopwait.as<int>();
		if (process_stopwait < 0) {
		  syslog(LOG_ERR, "Error: 'stopwait' must be greater than or equal to 0.");
		  throw std::runtime_error("Error: 'stopwait' must be greater than or equal to 0.");
		}
		program.setStoptimeout(process_stopwait);
	}

	YAML::Node environment = process["environment"];
	if (environment) {
		std::map<std::string, std::string> process_environment;
		for (const auto& env : environment) {
			program.addEnv(env.first.as<std::string>(), env.second.as<std::string>());
		}
	}

	YAML::Node stdout_logfile = process["stdout_logfile"];
	if (stdout_logfile) {
		std::string process_stdout_logfile = stdout_logfile.as<std::string>();
		if (process_stdout_logfile.empty()) {
		  syslog(LOG_ERR, "Error: 'stdout_logfile' is empty.");
		  throw std::runtime_error("Error: 'stdout_logfile' is empty.");
		}
		program.setStdoutfile(process_stdout_logfile);
	}

	YAML::Node stderr_logfile = process["stderr_logfile"];
	if (stderr_logfile) {
		std::string process_stderr_logfile = stderr_logfile.as<std::string>();
		if (process_stderr_logfile.empty()) {
		  syslog(LOG_ERR, "Error: 'stderr_logfile' is empty.");
		  throw std::runtime_error("Error: 'stderr_logfile' is empty.");
		}
		program.setStderrfile(process_stderr_logfile);
	}
	YAML::Node umask = process["umask"];
	if (umask) {
		mode_t process_umask = umask.as<mode_t>();
		if (process_umask < 0) {
		  syslog(LOG_ERR, "Error: 'umask' must be greater than or equal to 0.");
		  throw std::runtime_error("Error: 'umask' must be greater than or equal to 0.");
		}
		program.setUmask(process_umask);
	}
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
