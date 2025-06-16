#include "daemon/daemon.hpp"

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
	// Verify if the socket file already exists
	if (access(this->socketPath.c_str(), F_OK) == 0) {
		if (unlink(this->socketPath.c_str()) == -1) {
			this->loggers[0].doLog("Error unlinking socket: " + std::string(strerror(errno)));
			close(this->socketFd);
			throw std::runtime_error("Error unlinking socket: " + std::string(strerror(errno)));
		}
		this->loggers[0].doLog("Socket file already exists, unlinking done.");
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
	this->stopAllPrograms();
	if (this->socketFd != -1) {
		close(this->socketFd);
	}
	if (!this->socketPath.empty()) {
		unlink(this->socketPath.c_str());
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
std::vector<Log> Daemon::getLogs() const
{
	return this->loggers;
}

void Daemon::sendLogs(const std::string &message, std::string log_levelmsg)
{
	if (log_levelmsg.empty()) {
		log_levelmsg = "INFO";
	}
	time_t now = time(nullptr);
	std::tm *ltm = std::localtime(&now);
	char time_buf[20];
	std::strftime(time_buf, sizeof(time_buf), "%d:%m:%Y %H:%M:%S", ltm);
	std::string time_str(time_buf);
	std::string new_message = time_str + " - [taskmasterd] " + message;
	Log::LogLevel logLevel = convertStringToLogLevel(log_levelmsg);
	for (auto &logger : this->loggers) {
		if (convertLogLevelToSyslog(logger.getLogLevel()) >= convertLogLevelToSyslog(logLevel)) {
			logger.doLog(new_message);
		}
	}
}

void Daemon::initialStart()
{
	for (auto& program : this->programs) {
		if (program.getAutostart()) {
			program.start("");
		}
	}
	this->sendLogs("All processes started.", "INFO");
}

void Daemon::stopAllPrograms()
{
	for (auto &program : this->programs) {
		program.stop("");
		this->sendLogs("Program " + program.getName() + " stopped.", "INFO");
	}
}

void Daemon::addProgram(Program &program)
{
	this->programs.push_back(program);
}

void Daemon::removeProgram(Program &program)
{
	for (auto it = this->programs.begin(); it != this->programs.end(); ++it) {
		if (it->getName() == program.getName()) {
			this->programs.erase(it);
			break;
		}
	}
}

void Daemon::printDaemon()
{
	std::cout << "Daemon socket path: " << this->socketPath << std::endl;
	std::cout << "Daemon socket fd: " << this->socketFd << std::endl;
	std::cout << "Daemon processes:" << std::endl;
	for (auto &process : this->programs) {
		std::cout << "============================" << std::endl;
		process.printProgram();
	}
}

std::string Daemon::stringStatusProgram(std::string name)
{
	for (auto &program : this->programs) {
		if (program.getName() == name) {
			return "Program " + program.getName() + " is : \n" + program.getStates();
		}
	}
	throw std::runtime_error("Process " + name + " not found.");
}

std::string Daemon::stringStatusAllPrograms()
{
	std::string status = "Programs status:\n";
	for (auto &program : this->programs) {
		status += "Program " + program.getName() + " is : \n" + program.getStates() + "\n";
	}
	return status;
}

std::vector<Program> Daemon::getPrograms()
{
	return this->programs;
}
