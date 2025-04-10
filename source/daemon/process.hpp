#ifndef PROCESS_HPP
#define PROCESS_HPP

#include <string>
#include <vector>

class Process {
	public:
		Process();
		~Process();

		void setName(std::string name);
		void setPath(std::string path);

	private:
		std::string _name;
		std::string _command;
		std::string _directory;
		int _nbprocess;
		bool _autostart;
		bool _autorestart;
};

#endif
