#include "daemon/parsing/reloadParsing.hpp"

void reloadConf(Daemon *daemon) {
	daemon->sendLogs("Reloading configuration from file: " + daemon->getConfPath(), "INFO");
	std::vector<Program> newConfPrograms;
	try {
		YAML::Node config = YAML::LoadFile(daemon->getConfPath());
		if (!config) {
			daemon->sendLogs("Error loading configuration file: " + daemon->getConfPath(), "ERROR");
			daemon->sendLogs("Reloading configuration failed, keeping current configuration.", "ERROR");
		}
		std::string socketPath = config["unix_http_server"]["file"].as<std::string>();

		// if (socketPath != daemon->getSocketPath()) {
		// 	daemon->sendLogs("Socket path changed, reinitializing daemon." + socketPath + " old : " + daemon->getSocketPath() , "INFO");
		// 	daemon->setSocketPath(socketPath);
		// 	 // Soucis avec l'ajout de ipc:// devant le nom du socket
		// 	// Redemarrer le daemon ?
		// }
		Log newConfLog = parsingTaskmasterd(config["taskmasterd"]);
		newConfPrograms = parsingPrograms(config["programs"], daemon->getLogs());
		if (newConfLog == daemon->getLogs()[0]) {
			daemon->sendLogs("Taskmasterd configuration unchanged, keeping current logs.", "INFO");
		} else {
			daemon->sendLogs("Taskmasterd configuration changed, updating logs.", "INFO");
			daemon->updateLoggers(newConfLog);
		}
	}
	catch (const std::runtime_error &e) {
		daemon->sendLogs("Error parsing: " + std::string(e.what()), "ERROR");
		daemon->sendLogs("Reloading configuration failed, keeping current configuration.", "ERROR");
		return;
	}
	// Check if the number of programs has changed
	std::vector<Program> currentPrograms = daemon->getPrograms();
	for (const auto &program : currentPrograms) {
		bool found = false;
		for (const auto &newProgram : newConfPrograms) {
			if (program.getName() == newProgram.getName()) {
				found = true;
				break;
			}
		}
		if (!found) {
			daemon->sendLogs("Program " + program.getName() + " not found in the new configuration, removing it.", "INFO");
			daemon->removeProgram(daemon->getProgram(program.getName()));
		}
	}
	for (auto &program : newConfPrograms) {
		try {
			Program &currentProgram = daemon->getProgram(program.getName());
			if (currentProgram == program) {
				daemon->sendLogs("Program " + program.getName() + " unchanged, keeping current configuration.", "INFO");
			} else {
				daemon->sendLogs("Program " + program.getName() + " changed, updating configuration.", "INFO");
				daemon->removeProgram(daemon->getProgram(program.getName()));
				daemon->addProgram(program);
			}
		} catch (const std::runtime_error &e) {
			daemon->sendLogs("New program " + program.getName() + " not found in the current configuration, adding it.", "INFO");
			daemon->addProgram(program);
		}
	}
}
