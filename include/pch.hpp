#ifndef PCH_HPP
#define PCH_HPP

// C++ Standard Library
#include <algorithm> // IWYU pragma: keep
#include <csignal>
#include <functional> // IWYU pragma: keep
#include <iomanip>    // IWYU pragma: keep
#include <iostream>   // IWYU pragma: keep
#include <map>        // IWYU pragma: keep
#include <queue>      // IWYU pragma: keep
#include <sstream>    // IWYU pragma: keep
#include <sstream>
#include <stdexcept>     // IWYU pragma: keep
#include <string>        // IWYU pragma: keep
#include <unordered_set> // IWYU pragma: keep
#include <utility>       // IWYU pragma: keep
#include <vector>        // IWYU pragma: keep

// POSIX / System Headers
#include <fcntl.h>
#include <pwd.h>
#include <sys/epoll.h>
#include <sys/prctl.h>
#include <sys/stat.h>
#include <unistd.h>

#include <sys/signal.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include <cerrno>
#include <strings.h>
#include <sys/types.h>

// Third-Party Libraries
// ======READLINE======
#include <readline/history.h>
#include <readline/readline.h>

// =====YAML-cpp======
#include <yaml-cpp/yaml.h>

// =====Nanomsg======
#include "nanomsg/nn.h"     // IWYU pragma: keep
#include "nanomsg/reqrep.h" // IWYU pragma: keep

#endif // PCH_HPP
