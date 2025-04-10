#ifndef DAEMON_HPP
#define DAEMON_HPP

#include <string>
#include <vector>
#include "./log.hpp"
#include "./process.hpp"

class Daemon {
	public:
		Daemon();
		~Daemon();

		void setupDaemon();
		void runDaemon();

	private :
		std::string sockfile;
		int socketFd;

		Log loggers;
		std::vector<Process> processInfos;
};

#endif
