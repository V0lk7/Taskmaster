#include "daemon.hpp"

Daemon::Daemon(std::string socketPath, Log logInfo)
{
	this->socketPath = socketPath;
	this->socketFd = -1;
	this->loggers.push_back(logInfo);
	Log syslogTaskmasterd("taskmasterd", Log::Type::SYSLOG, logInfo.getLogLevel(), this->socketPath);
	this->loggers.push_back(syslogTaskmasterd);

	// Initialize the socket
	this->socketFd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (this->socketFd == -1) {
		this->loggers[0].doLog("Error creating socket: " + std::string(strerror(errno)));
		throw std::runtime_error("Error creating socket: " + std::string(strerror(errno)));
	}
	if (this->socketPath.empty()) {
		return;
	}
	struct sockaddr_un addr;
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, socketPath.c_str(), sizeof(addr.sun_path) - 1);
	if (bind(socketFd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
		loggers[0].doLog("Error binding socket: " + std::string(strerror(errno)));
		close(socketFd);
		throw std::runtime_error("Error binding socket: " + std::string(strerror(errno)));
	}
}

Daemon::~Daemon()
{
	if (this->socketFd != -1) {
		close(this->socketFd);
	}
	for (auto &logger : this->loggers) {
		logger.doLog("Daemon destroyed.");
	}
}

void Daemon::setSocketPath(std::string socketPath)
{
	this->socketPath = socketPath;
}
std::string Daemon::getSocketPath() const
{
	return this->socketPath;
}
int Daemon::getSocketFd() const
{
	return this->socketFd;
}
void Daemon::setSocketFd(int socketFd)
{
	this->socketFd = socketFd;
}
void Daemon::sendLogs(const std::string &message)
{
	for (auto &logger : this->loggers) {
		logger.doLog(message);
	}
}

int Daemon::startProcess(Process &process)
{
	// Start the process
	int pid = fork();
	if (pid == 0) {
		// Child process
		process.start();
		exit(0);
	} else if (pid < 0) {
		this->loggers[0].doLog("Error starting process: " + std::string(strerror(errno)));
		return -1;
	}
	return pid;
}

void Daemon::stopProcess(Process &process)
{
	// Stop the process
	int pid = process.getPid();
	if (pid > 0) {
		kill(pid, SIGTERM);
	}
}
