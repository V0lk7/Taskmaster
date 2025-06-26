#ifndef PROGRAM_HPP
#define PROGRAM_HPP

#include "log.hpp"
#include "process.hpp"

class Program {
public:
  enum class Restart { TRUE, UNEXPECTED, FALSE };

  Program(const std::string &name, const std::string &command);
  ~Program();

  bool operator==(const Program &other) const;

  void setName(const std::string &name);
  std::string getName() const;
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
  void addLog(const Log &log);
  std::string getStates();
  std::vector<Log> getLogs() const;

  void doLog(const std::string &message, Log::LogLevel level,
             std::string name_process);
  std::string convertRestartToString(Restart restart);
  std::string convertStopsignalToString(int signal);
  void start(std::string name_process);
  // bool start(std::string name_process, std::string &error_msg);
  void stop(std::string name_process);
  void printProgram();

  std::vector<std::string> getStatusProcesses() const;
  Process &getProcess(std::string name);
  void superviseProcesses();

  void handleExitProcess(Process &, int);

private:
  std::string _name;
  std::string _command;
  // std::vector<char*> _args;
  std::string _workdir;
  int _nbprocess;   // How many processes to start
  bool _autostart;  // Whether to start this program at launch or not
  Restart _restart; // Whether the program should be restarted always, never, or
                    // on unexpected exits only
  std::vector<int>
      _exitcodes;  // Which return codes represent an "expected" exit status
  int _startdelay; // How long the program should be running after it’s started
                   // for it to be considered successfully started
  int _restartretry; // How many times a restart should be attempted before
                     // aborting
  int _stopsignal;   // The signal to send to the process when stopping it
  int _stoptimeout;  // How long to wait for the process to stop before killing
                     // it
  std::string _stdoutfile; // Where to redirect the stdout of the process
  std::string _stderrfile; // Where to redirect the stderr of the process
  mode_t _umask;           // The umask to set for the process
  std::map<std::string, std::string>
      _env;               // Environment variables to set for the process
  std::vector<Log> _logs; // Logs for the process
  std::vector<Process> _processes; // Vector of processes
  // std::vector<char*> setArgs(std::string rawCommand);
  // std::string setCommand(std::string rawCommand);
  void addProcess();
};

int convertStringToStopsignal(const std::string &str);
Program::Restart convertStringToRestart(const std::string &str);
std::string to_octal_string(mode_t mode);

#endif
