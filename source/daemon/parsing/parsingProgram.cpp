#include "daemon/parsing/parsing.hpp"

std::vector<Program> parsingPrograms(YAML::Node programs, std::vector<Log> loggersDaemon) {
	std::vector<Program> programs_list;
	for (const auto& program : programs) {
		YAML::Node name = program["name"];
		if (!name) {
			throw std::runtime_error("Error: 'name' not found in 'program'.");
		}
		std::string program_name = name.as<std::string>();
		if (program_name.empty()) {
			throw std::runtime_error("Error: 'name' is empty.");
		}

		YAML::Node command = program["command"];
		if (!command) {
			throw std::runtime_error("Error: 'command' not found in 'program'.");
		}
		std::string program_command = command.as<std::string>();
		if (program_command.empty()) {
			throw std::runtime_error("Error: 'command' is empty.");
		}

		Program new_program = Program(program_name, program_command);

		YAML::Node directory = program["directory"];
		if (directory) {
			std::string program_directory = directory.as<std::string>();
			if (program_directory.empty()) {
				throw std::runtime_error("Error: 'directory' is empty.");
			}
			new_program.setWorkdir(program_directory);
		}

		YAML::Node numprocs = program["numprocs"];
		if (numprocs) {
			int program_numprocs = numprocs.as<int>();
			if (program_numprocs <= 0) {
				throw std::runtime_error("Error: 'numprocs' must be greater than 0.");
			}
			new_program.setNbprocess(program_numprocs);
		}

		YAML::Node autostart = program["autostart"];
		if (autostart) {
			bool program_autostart = autostart.as<bool>();
			if (program_autostart != true && program_autostart != false) {
				throw std::runtime_error("Error: 'autostart' must be true or false.");
			}
			new_program.setAutostart(program_autostart);
		}

		YAML::Node autorestart = program["autorestart"];
		if (autorestart) {
			std::string program_autorestart = autorestart.as<std::string>();
			if (program_autorestart != "true" && program_autorestart != "unexpected" && program_autorestart != "false") {
				throw std::runtime_error("Error: 'autorestart' must be 'true', 'unexpected' or 'false'.");
			}
			new_program.setRestart(convertStringToRestart(program_autorestart));
		}

		YAML::Node exitcodes = program["exitcodes"];
		if (exitcodes) {
			std::vector<int> program_exitcodes;
			for (const auto& exitcode : exitcodes) {
				program_exitcodes.push_back(exitcode.as<int>());
			}
			if (program_exitcodes.empty()) {
				throw std::runtime_error("Error: 'exitcodes' is empty.");
			}
			new_program.setExitcodes(program_exitcodes);
		}

		YAML::Node starttime = program["starttime"];
		if (starttime) {
			int program_starttime = starttime.as<int>();
			if (program_starttime < 0) {
				throw std::runtime_error("Error: 'starttime' must be greater than or equal to 0.");
			}
			new_program.setStartdelay(program_starttime);
		}

		YAML::Node startretries = program["startretries"];
		if (startretries) {
			int program_startretries = startretries.as<int>();
			if (program_startretries < 0) {
				throw std::runtime_error("Error: 'startretries' must be greater than or equal to 0.");
			}
			new_program.setRestartretry(program_startretries);
		}

		YAML::Node stopsignal = program["stopsignal"];
		if (stopsignal) {
			int program_stopsignal = convertStringToStopsignal(stopsignal.as<std::string>());
			if (program_stopsignal < 0) {
				throw std::runtime_error("Error: 'stopsignal' must be greater than or equal to 0.");
			}
			new_program.setStopsignal(program_stopsignal);
		}

		YAML::Node stopwait = program["stopwait"];
		if (stopwait) {
			int program_stopwait = stopwait.as<int>();
			if (program_stopwait < 0) {
				throw std::runtime_error("Error: 'stopwait' must be greater than or equal to 0.");
			}
			new_program.setStoptimeout(program_stopwait);
		}

		YAML::Node environment = program["environment"];
		if (environment) {
			std::map<std::string, std::string> program_environment;
			for (const auto& env : environment) {
				new_program.addEnv(env.first.as<std::string>(), env.second.as<std::string>());
			}
		}

		YAML::Node stdout_logfile = program["stdout_logfile"];
		if (stdout_logfile) {
			std::string program_stdout_logfile = stdout_logfile.as<std::string>();
			if (program_stdout_logfile.empty()) {
				throw std::runtime_error("Error: 'stdout_logfile' is empty.");
			}
			new_program.setStdoutfile(program_stdout_logfile);
		}

		YAML::Node stderr_logfile = program["stderr_logfile"];
		if (stderr_logfile) {
			std::string program_stderr_logfile = stderr_logfile.as<std::string>();
			if (program_stderr_logfile.empty()) {
				throw std::runtime_error("Error: 'stderr_logfile' is empty.");
			}
			new_program.setStderrfile(program_stderr_logfile);
		}
		YAML::Node umask = program["umask"];
		if (umask) {
			mode_t program_umask = std::stoi(umask.as<std::string>(), nullptr, 8);
			if (program_umask < 0) {
				throw std::runtime_error("Error: 'umask' must be greater than or equal to 0.");
			}
			new_program.setUmask(program_umask);
		}
		for (const auto& logger : loggersDaemon) {
			new_program.addLog(logger);
		}
		programs_list.push_back(new_program);
	}
	return programs_list;
}
