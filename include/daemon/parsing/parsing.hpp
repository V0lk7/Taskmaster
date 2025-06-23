#ifndef PARSING_HPP
#define PARSING_HPP
#include <iostream>
#include <yaml-cpp/yaml.h>
#include "../daemon.hpp"

Daemon *parsingFile(std::string config_file);
Log parsingTaskmasterd(YAML::Node taskmasterd);
std::vector<Program> parsingPrograms(YAML::Node programs, std::vector<Log> loggersDaemon);

#endif
