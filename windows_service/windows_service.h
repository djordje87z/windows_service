#ifndef windows_service_H
#define windows_service_H

#include <windows.h>
#include <string>
using std::string;

#define LOG_APP "C:\\log-app.txt"
#define LOG_CAM "C:\\log-camera.txt"

#define SAFE_RELEASE(p) { if ( (p) ) { (p)->Release(); (p) = 0; } }

class Logger;

class WindowsService {
	Logger* m_logger;

	WindowsService();
	bool GetActiveWindowTitle(string& title);
	bool ProcessName(DWORD ProcessId, string& filename);
	bool GetCurrentActiveSessions(void);
	bool IsMicrophoneRecording();
public:
	WindowsService(Logger *logger);
	~WindowsService() {};
	void Run();

};


#endif //windows_service_H
