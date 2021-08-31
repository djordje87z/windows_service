#include "windows_service.h"
#include "logger.h"

#include <stdexcept>

using namespace std;



int main()
{
	Logger* logger = new Logger;
	WindowsService* ws = new WindowsService(logger);

	try
	{
		ws->Run();
	}

	catch (exception& e) 
	{
		cerr << "Exception: " << e.what() << endl;
	}

	//HANDLE hDir = CreateFileA("C", // pointer to the file name
	//	FILE_LIST_DIRECTORY,                // access (read/write) mode
	//	FILE_SHARE_READ | FILE_SHARE_DELETE,  // share mode
	//	NULL,                               // security descriptor
	//	OPEN_EXISTING,                      // how to create
	//	FILE_FLAG_BACKUP_SEMANTICS,         // file attributes
	//	NULL                                // file with attributes to copy
	//);

	//FILE_NOTIFY_INFORMATION Buffer[1024];
	//DWORD BytesReturned;
	//while (ReadDirectoryChangesW(
	//	hDir,                                  // handle to directory
	//	&Buffer,                                    // read results buffer
	//	sizeof(Buffer),                                // length of buffer
	//	TRUE,                                 // monitoring option
	//	FILE_NOTIFY_CHANGE_SECURITY |
	//	FILE_NOTIFY_CHANGE_CREATION |
	//	FILE_NOTIFY_CHANGE_LAST_ACCESS |
	//	FILE_NOTIFY_CHANGE_LAST_WRITE |
	//	FILE_NOTIFY_CHANGE_SIZE |
	//	FILE_NOTIFY_CHANGE_ATTRIBUTES |
	//	FILE_NOTIFY_CHANGE_DIR_NAME |
	//	FILE_NOTIFY_CHANGE_FILE_NAME,             // filter conditions
	//	&BytesReturned,              // bytes returned
	//	NULL,                          // overlapped buffer
	//	NULL// completion routine
	//))
	//{

	//}

	//return 0;

}
