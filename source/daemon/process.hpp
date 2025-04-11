#ifndef PROCESS_HPP
#define PROCESS_HPP

#include <string>
#include <map>
#include <vector>
#include <sys/types.h>
#include <stdexcept>

class Process {
	public:
		enum class State {
			STOPPED,
			RUNNING,
			STARTING,
			STOPPING,
			EXITED
		};
		enum class Restart {
			ALWAYS,
			ON_FAILURE,
			NEVER
		};

		Process(const std::string &name, const std::string &command, const std::string &workdir,
				int nbprocess, bool autostart, Restart restart, int exitcode,
				int startdelay, int restartretry, int stopsignal, int stoptimeout,
				bool discardstdout, bool discardstderr,
				const std::string &stdoutfile, const std::string &stderrfile,
				mode_t umask);
		~Process();

		Restart convertStringToRestart(const std::string &str);
		std::string convertRestartToString(Restart restart);
		std::string convertStateToString(State state);

	private:
		std::string _name;
		State _state;
		std::string _command;
		std::string _workdir;
		int _nbprocess; //How many processes to start
		bool _autostart; //Whether to start this program at launch or not
		Restart _restart; //Whether the program should be restarted always, never, or on unexpected exits only
		std::vector<int> _exitcode; //Which return codes represent an "expected" exit status
		int _startdelay; //How long the program should be running after it’s started for it to be considered successfully started
		int _restartretry; //How many times a restart should be attempted before aborting
		int _stopsignal; //The signal to send to the process when stopping it
		int _stoptimeout; //How long to wait for the process to stop before killing it
		bool discardstdout; //Whether to discard the stdout of the process
		bool discardstderr; //Whether to discard the stderr of the process
		std::string _stdoutfile; //Where to redirect the stdout of the process
		std::string _stderrfile; //Where to redirect the stderr of the process
		mode_t _umask; //The umask to set for the process
		std::map<std::string, std::string> _env; //Environment variables to set for the process
	};

#endif
