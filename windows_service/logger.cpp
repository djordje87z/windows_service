#include "logger.h"

#include <time.h>

namespace Util
{
	// Get current date/time, format is YYYY-MM-DD.HH:mm:ss
	const std::string CurrentDateTime()
	{
		time_t     now = time(NULL);
		struct tm  tstruct;
		char       buf[80];
		localtime_s(&tstruct, &now);
		strftime(buf, sizeof(buf), "%Y-%m-%d %X", &tstruct);
		return buf;
	}
}

Logger::~Logger() {
	m_mtx.lock();
	for (auto it = m_map_files.begin(); it != m_map_files.end(); ++it) {
		ofstream* pFile = &it->second;
		pFile->close();
	}
	m_map_files.clear();
	m_mtx.unlock();
}

void Logger::ConsoleLog(const char* format, ...) {
	char* sMessage = NULL;
	int nLength = 0;
	va_list args;
	va_start(args, format);
	//  Return the number of characters in the string referenced the list of arguments.
	// _vscprintf doesn't count terminating '\0' (that's why +1)
	nLength = _vscprintf(format, args) + 1;
	sMessage = new char[nLength];
	vsprintf_s(sMessage, nLength, format, args);
	//vsprintf(sMessage, format, args);
	va_end(args);
	
	string date = Util::CurrentDateTime();
	std::cout << date << " " << sMessage << std::endl;

	delete[] sMessage;
}

void Logger::Log(const string fname, const char* format, ...) {

	char* sMessage = NULL;
	int nLength = 0;
	va_list args;
	va_start(args, format);
	//  Return the number of characters in the string referenced the list of arguments.
	// _vscprintf doesn't count terminating '\0' (that's why +1)
	nLength = _vscprintf(format, args) + 1;
	sMessage = new char[nLength];
	vsprintf_s(sMessage, nLength, format, args);
	va_end(args);

	m_mtx.lock();
	if (!m_map_files.count(fname))
	{
		try
		{
			m_map_files[fname] = ofstream();
			m_map_files[fname].open(fname);
		}
		catch (...)
		{
			m_map_files.erase(fname);
			throw;
		}
	}
	ofstream& log_file = m_map_files[fname];
	//vsprintf(sMessage, format, args);
	string date = Util::CurrentDateTime();
	log_file << date << " " << sMessage << std::endl;
	m_mtx.unlock();

	delete[] sMessage;
}