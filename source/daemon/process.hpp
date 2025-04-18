#ifndef PROCESS_HPP
#define PROCESS_HPP

#include <string>
#include <map>
#include <vector>
#include <sys/types.h>
#include <stdexcept>
#include <sys/signal.h>
#include <sys/stat.h>
#include "log.hpp"

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
			TRUE,
			UNEXPECTED,
			FALSE
		};



		Process(const std::string &name, const std::string &command, const std::string &workdir,
				int nbprocess, bool autostart, Restart restart, std::vector<int> exitcodes,
				int startdelay, int restartretry, int stopsignal, int stoptimeout,
				const std::string &stdoutfile, const std::string &stderrfile,
				mode_t umask, std::map<std::string, std::string> env);
		Process(const std::string &name, const std::string &command);
		~Process();

		void setState(State state);
		State getState() const;
		void setName(const std::string &name);
		std::string getName() const;
		void setPid(int pid);
		int getPid() const;
		void setCommand(const std::string &command);
		std::string getCommand() const;
		void setWorkdir(const std::string &workdir);
		std::string getWorkdir() const;
		void setNbprocess(int nbprocess);
		int getNbprocess() const;
		void setAutostart(bool autostart);
		bool getAutostart() const;
		void setRestart(Restart restart);
		Restart getRestart() const;
		void setExitcodes(std::vector<int> exitcodes);
		std::vector<int> getExitcodes() const;
		void setStartdelay(int startdelay);
		int getStartdelay() const;
		void setRestartretry(int restartretry);
		int getRestartretry() const;
		void setStopsignal(int stopsignal);
		void setStopsignalString(const std::string &stopsignal);
		int getStopsignal() const;
		void setStoptimeout(int stoptimeout);
		int getStoptimeout() const;
		void setStdoutfile(const std::string &stdoutfile);
		void setStderrfile(const std::string &stderrfile);
		std::map<std::string, std::string> getEnv() const;
		void addEnv(const std::string &key, const std::string &value);
		void setUmask(mode_t umask);
		mode_t getUmask() const;

		void doLog(const std::string &message, Log::LogLevel level);
		std::string convertRestartToString(Restart restart);
		std::string convertStateToString(State state);
		std::string convertStopsignalToString(int signal);
		int convertStringToStopsignal(const std::string &str);
		void start();
		void stop();
		void restart();
		void kill();

		private:
		std::string _name;
		int _pid;
		State _state;
		std::string _command;
		std::string _workdir;
		int _nbprocess; //How many processes to start
		bool _autostart; //Whether to start this program at launch or not
		Restart _restart; //Whether the program should be restarted always, never, or on unexpected exits only
		std::vector<int> _exitcodes; //Which return codes represent an "expected" exit status
		int _startdelay; //How long the program should be running after it’s started for it to be considered successfully started
		int _restartretry; //How many times a restart should be attempted before aborting
		int _stopsignal; //The signal to send to the process when stopping it
		int _stoptimeout; //How long to wait for the process to stop before killing it
		std::string _stdoutfile; //Where to redirect the stdout of the process
		std::string _stderrfile; //Where to redirect the stderr of the process
		mode_t _umask; //The umask to set for the process
		std::map<std::string, std::string> _env; //Environment variables to set for the process
		std::vector<Log> _logs; //Logs for the process
	};

Process::Restart convertStringToRestart(const std::string &str);

#endif

