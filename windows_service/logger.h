#ifndef logger_H
#define logger_H

#include <string>
#include <fstream>
#include <iostream>
#include <cstdarg>
#include <unordered_map>
#include <mutex>
using std::ofstream;
using std::string;

class Logger {
	
	std::unordered_map <string, ofstream> m_map_files;
	std::mutex m_mtx;

	Logger(const Logger&) {};             // copy constructor is private
	Logger& operator=(const Logger&) { return *this; };

public:
	Logger() {};
	~Logger();

	void ConsoleLog(const char* format, ...);
	void Log(const string fname, const char* format, ...);
};


#endif //logger_H
