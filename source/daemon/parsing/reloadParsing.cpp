#include "daemon/parsing/reloadParsing.hpp"

void reloadConf(Daemon *daemon) {
	daemon->sendLogs("Reloading configuration from file: " + daemon->getConfPath(), "INFO");
	YAML::Node config = YAML::LoadFile(daemon->getConfPath());
	if (!config) {
		throw std::runtime_error("Error: Unable to load config file.");
		exit(1);
	}
	std::string socketPath = config["unix_http_server"]["file"].as<std::string>();
	
	// if (socketPath != daemon->getSocketPath()) {
	// 	daemon->sendLogs("Socket path changed, reinitializing daemon." + socketPath + " old : " + daemon->getSocketPath() , "INFO");
	// 	daemon->setSocketPath(socketPath);
	     // Soucis avec l'ajout de ipc:// devant le nom du socket
		// Redemarrer le daemon ?
	// }
	Log newConfLog = parsingTaskmasterd(config["taskmasterd"]);
	if (newConfLog == daemon->getLogs()[0]) {
		daemon->sendLogs("Taskmasterd configuration unchanged, keeping current logs.", "INFO");
	} else {
		daemon->sendLogs("Taskmasterd configuration changed, updating logs.", "INFO");
		for (auto &log : daemon->getLogs()) {
			if (log.getName() == "taskmasterd") {
				log = newConfLog;
			}
		}
		for (auto &prog : daemon->getPrograms()) {
			for (auto &log : prog.getLogs()) {
				if (log.getName() == "taskmasterd") {
					log = newConfLog;
				}
			}
		}
	}
	std::vector<Program> newConfPrograms = parsingPrograms(config["programs"], daemon->getLogs());
	for (auto &program : newConfPrograms) {
		if (daemon->getProgram(program.getName()) == program) {
			daemon->sendLogs("Program " + program.getName() + " unchanged, keeping current configuration.", "INFO");
		} else {
			daemon->sendLogs("Program " + program.getName() + " changed, updating configuration.", "INFO");
			daemon->removeProgram(daemon->getProgram(program.getName()));
			daemon->addProgram(program);
			if (program.getAutostart()) {
				daemon->sendLogs("Starting program " + program.getName() + " after configuration change.", "INFO");
				program.start("");
			}
		}
	}
}
