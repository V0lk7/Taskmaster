#ifndef DAEMON_HPP
#define DAEMON_HPP

#include <string>
#include <vector>
#include <sys/socket.h>
#include <sys/un.h>
#include <cstring>
#include "log.hpp"
#include "program.hpp"

class Daemon {
	public:
		Daemon(std::string socketPath, Log logInfo);
		~Daemon();

		void setSocketPath(std::string socketPath);
		std::string getSocketPath() const;
		int getSocketFd() const;
		void setSocketFd(int socketFd);
		void sendLogs(const std::string &message);
		void addProgram(Program &program);
		void removeProgram(Program &program);
		void initialStart();
		void printDaemon();
		void stopAllPrograms();
		std::string stringStatusProgram(std::string name);
		std::string stringStatusAllPrograms();
		std::vector<Program> getPrograms();

	private :
		std::string socketPath;
		int socketFd;

		std::vector<Log> loggers;
		std::vector<Program> programs;
};

#endif
