#ifndef PCH_HPP
#define PCH_HPP

// C++ Standard Library
#include <algorithm> // IWYU pragma: keep
#include <csignal>
#include <functional>    // IWYU pragma: keep
#include <iostream>      // IWYU pragma: keep
#include <map>           // IWYU pragma: keep
#include <queue>         // IWYU pragma: keep
#include <sstream>       // IWYU pragma: keep
#include <string>        // IWYU pragma: keep
#include <unordered_set> // IWYU pragma: keep
#include <utility>       // IWYU pragma: keep
#include <vector>        // IWYU pragma: keep

// POSIX / System Headers
#include <fcntl.h>
#include <sys/epoll.h>
#include <unistd.h>

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
