#ifndef PROCESS_HPP
#define PROCESS_HPP

#include <string>
#include <map>
#include <vector>
#include <sys/types.h>
#include <stdexcept>
#include <sys/signal.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <cstring>
#include <fcntl.h>
#include <sstream>
#include <iomanip>

class Process {
	public:
		enum class State {
			STOPPED,
			RUNNING,
			STARTING,
			STOPPING,
			EXITED,
			FATAL
		};

		Process(std::string name);
		~Process();
		void setState(State state);
		void setPid(int pid);
		State getState() const;
		int getPid() const;
		std::string getName() const;

		void start(mode_t umask_process, const std::string &workdir, const std::string &stdoutfile, const std::string &stderrfile, const std::map<std::string, std::string> &env, std::string command);
		void stop(int stopsignal, int stoptimeout);

	private:
		std::string _name;
		State _state;
		int _pid;
	};

std::string convertStateToString(Process::State state);

#endif

