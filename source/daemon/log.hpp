#ifndef LOG_HPP
#define LOG_HPP

#include <string>
#include <syslog.h>

struct Log {
//   enum class LogLvl { info, warning, error, debug };

  std::string _filename;
  int _logFd;

//   LogLvl _logLvl;
};

#endif
