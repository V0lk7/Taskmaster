#ifndef DAEMON_HPP
#define DAEMON_HPP

#include <string>
#include <vector>
#include <sys/socket.h>
#include <sys/un.h>
#include <cstring>
#include "log.hpp"
#include "process.hpp"

class Daemon {
	public:
		Daemon(std::string socketPath, Log logInfo);
		~Daemon();

		void setSocketPath(std::string socketPath);
		std::string getSocketPath() const;
		int getSocketFd() const;
		void setSocketFd(int socketFd);
		void sendLogs(const std::string &message);
		int startProcess(Process &process);
		void stopProcess(Process &process);
		void restartProcess(Process &process);
		void killProcess(Process &process);
		void addProcess(Process &process);
		void removeProcess(Process &process);

	private :
		std::string socketPath;
		int socketFd;

		std::vector<Log> loggers;
		std::vector<Process> processes;
};

#endif
