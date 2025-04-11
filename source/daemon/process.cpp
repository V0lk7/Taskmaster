#include "process.hpp"

Process::Process(const std::string &name, const std::string &command, const std::string &workdir,
		int nbprocess, bool autostart, Restart restart, std::vector<int> exitcode,
		int startdelay, int restartretry, int stopsignal, int stoptimeout,
		bool discardstdout, bool discardstderr,
		const std::string &stdoutfile, const std::string &stderrfile,
		mode_t umask, std::map<std::string, std::string> env)
	: _name(name), _state(State::STOPPED), _command(command), _workdir(workdir),
	  _nbprocess(nbprocess), _autostart(autostart), _restart(restart),
	  _exitcode(exitcode), _startdelay(startdelay), _restartretry(restartretry),
	  _stopsignal(stopsignal), _stoptimeout(stoptimeout),
	  discardstdout(discardstdout), discardstderr(discardstderr),
	  _stdoutfile(stdoutfile), _stderrfile(stderrfile), _umask(umask), _env(env) {}

Process::~Process() {
}

Process::Restart Process::convertStringToRestart(const std::string &str) {
	if (str == "always") {
		return Restart::ALWAYS;
	} else if (str == "on-failure") {
		return Restart::ON_FAILURE;
	} else if (str == "never") {
		return Restart::NEVER;
	} else {
		throw std::invalid_argument("Invalid restart option");
	}
}
std::string Process::convertRestartToString(Restart restart) {
	switch (restart) {
		case Restart::ALWAYS:
			return "always";
		case Restart::ON_FAILURE:
			return "on-failure";
		case Restart::NEVER:
			return "never";
		default:
			throw std::invalid_argument("Invalid restart option");
	}
}
std::string Process::convertStateToString(State state) {
	switch (state) {
		case State::STOPPED:
			return "STOPPED";
		case State::RUNNING:
			return "RUNNING";
		case State::STARTING:
			return "STARTING";
		case State::STOPPING:
			return "STOPPING";
		case State::EXITED:
			return "EXITED";
		default:
			throw std::invalid_argument("Invalid state");
	}
}
