// windows_service.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "windows_service.h"
#include "logger.h"

#include <iostream>
#include <windows.h>
#include <wtsapi32.h>
#include <psapi.h>
#include <userenv.h>
#include <string>
#include <tchar.h>
#include <shlwapi.h>
#include < intrin.h >
#include <Lmcons.h>
#include <fstream>
using std::ofstream;
using std::string;

#include <dshow.h>
#include <ks.h>
#include <audioclient.h>
#include <audiopolicy.h>
#include <initguid.h>
#include <mmdeviceapi.h>
#include <endpointvolume.h>
#include <devpkey.h> 

#pragma comment(lib, "strmiids")
#pragma comment(lib, "uuid.lib")

WindowsService::WindowsService(Logger* logger) : m_logger(logger)
{

}

bool LaunchProcess(DWORD SessionId, const char* process_path)
{
    //DWORD SessionId = GetCurrentSessioId();
    if (SessionId == 0)    // no-one logged in
        return false;

    HANDLE hToken;
    BOOL ok = WTSQueryUserToken(SessionId, &hToken);
    if (!ok)
        return false;

    void* environment = NULL;
    ok = CreateEnvironmentBlock(&environment, hToken, TRUE);

    if (!ok)
    {
        CloseHandle(hToken);
        return false;
    }

    LPSTARTUPINFOA si;
    PROCESS_INFORMATION pi;

    memset(&si, 0, sizeof(si));
    memset(&pi, 0, sizeof(pi));

    si->lpDesktop = (char *)"winsta0\\default";


    // Do NOT want to inherit handles here
    DWORD dwCreationFlags = NORMAL_PRIORITY_CLASS | CREATE_UNICODE_ENVIRONMENT;
    ok = CreateProcessAsUserA(hToken, process_path, NULL, NULL, NULL, FALSE,
        dwCreationFlags, environment, NULL, si, &pi);

    DestroyEnvironmentBlock(environment);
    CloseHandle(hToken);

    if (!ok)
        return false;

    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    return true;
}


bool WindowsService::GetCurrentActiveSessions() {
	WTS_SESSION_INFO* pSession = NULL;
	DWORD session_id = -1;
	DWORD session_count = 0;
    string user_name;

	if (WTSEnumerateSessions(WTS_CURRENT_SERVER_HANDLE, 0, 1, &pSession, &session_count))
	{
        this->m_logger->ConsoleLog("WTSEnumerateSessions succcessfully!");
	}
	else
	{
		//log error
        this->m_logger->ConsoleLog("WTSEnumerateSessions failed!");
		return false;
	}

	for (size_t i = 0; i < session_count; i++)
	{
		int nTmpSessionId = pSession[i].SessionId;
		WTS_CONNECTSTATE_CLASS wts_connect_state = WTSDisconnected;
		WTS_CONNECTSTATE_CLASS* ptr_wts_connect_state = NULL;

		DWORD bytes_returned = 0;
		if (::WTSQuerySessionInformation(
			WTS_CURRENT_SERVER_HANDLE,
			nTmpSessionId,
			WTSConnectState,
			reinterpret_cast<LPTSTR*>(&ptr_wts_connect_state),
			&bytes_returned))
		{
			wts_connect_state = *ptr_wts_connect_state;
			::WTSFreeMemory(ptr_wts_connect_state);
			if (wts_connect_state != WTSActive) 
				continue;
		}
		else
		{
			//log error
			continue;
		}
		session_id = pSession[i].SessionId;
        LaunchProcess(session_id, "windows_service.exe");

  //      LPSTR pUserName = NULL;
  //      if (!::WTSQuerySessionInformationA(
  //          WTS_CURRENT_SERVER_HANDLE,
  //          nTmpSessionId,
  //          WTSUserName,
  //          &pUserName,
  //          &bytes_returned))
  //      {
  //          //log error
  //      }
  //      user_name = string(pUserName);
		//break;
	}
	return (session_id != -1);
}

bool WindowsService::ProcessName(DWORD ProcessId, string &filename) {
	bool ret = true;
	HANDLE Handle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, ProcessId);
	if (Handle) {
		char Buffer[MAX_PATH];
		if (GetModuleFileNameExA(Handle, 0, Buffer, MAX_PATH)) {
			filename = PathFindFileNameA(Buffer);
		}
		else {
			// Now would be a good time to call GetLastError()
			this->m_logger->ConsoleLog("Now would be a good time to call GetLastError()");
			ret = false;
		}
		CloseHandle(Handle);
	}

	return ret;
}

bool WindowsService::GetActiveWindowTitle(string &title) {
	//TCHAR wnd_title[256];
	HWND hwnd = GetForegroundWindow(); // get handle of currently active window
	if (hwnd == nullptr) return false;
	//GetWindowText(hwnd, wnd_title, sizeof(wnd_title));
	DWORD process_id;
	GetWindowThreadProcessId(hwnd, &process_id);
	return this->ProcessName(process_id, title);
}

bool WindowsService::IsMicrophoneRecording() {
    
    // #1 Get the audio endpoint associated with the microphone device
    HRESULT hr = S_OK;
    IMMDeviceEnumerator* pEnumerator = NULL;
    IAudioSessionManager2* pSessionManager = NULL;
    bool result = FALSE;

    CoInitialize(0);

    // Create the device enumerator.
    hr = CoCreateInstance(
        __uuidof(MMDeviceEnumerator),
        NULL, CLSCTX_ALL,
        __uuidof(IMMDeviceEnumerator),
        (void**)&pEnumerator);

    IMMDeviceCollection* dCol = NULL;
    hr = pEnumerator->EnumAudioEndpoints(eCapture, DEVICE_STATE_ACTIVE, &dCol);
    UINT dCount;
    hr = dCol->GetCount(&dCount);

    ////////////////////////////////
    static PROPERTYKEY key;

    GUID IDevice_FriendlyName = { 0xa45c254e, 0xdf1c, 0x4efd, { 0x80, 0x20, 0x67, 0xd1, 0x46, 0xa8, 0x50, 0xe0 } };
    key.pid = 14;
    key.fmtid = IDevice_FriendlyName;
    PROPVARIANT varName;
    // Initialize container for property value.
    PropVariantInit(&varName);
    ///////////////////////////////

    for (size_t i = 0; i < dCount; i++)
    {
        IMMDevice* pCaptureDevice = NULL;
        hr = dCol->Item(i, &pCaptureDevice);

        IPropertyStore* pProps = NULL;
        hr = pCaptureDevice->OpenPropertyStore(
            STGM_READ, &pProps);

        PROPVARIANT varName;
        // Initialize container for property value.
        PropVariantInit(&varName);

        // Get the endpoint's friendly-name property.
        hr = pProps->GetValue(
            key, &varName);

        std::wstring nameStr(varName.pwszVal);

        // #2 Determine whether it is the microphone device you are focusing on
        std::size_t found = nameStr.find(L"Microphone");
        if (found != std::string::npos)
        {
            // Print endpoint friendly name.
            //printf("Endpoint friendly name: \"%S\"\n", varName.pwszVal);

            // Get the session manager.
            hr = pCaptureDevice->Activate(
                __uuidof(IAudioSessionManager2), CLSCTX_ALL,
                NULL, (void**)&pSessionManager);
            break;
        }
    }

    // Get session state
    if (!pSessionManager)
    {
        return (result = FALSE);
    }

    int cbSessionCount = 0;
    LPWSTR pswSession = NULL;

    IAudioSessionEnumerator* pSessionList = NULL;
    IAudioSessionControl* pSessionControl = NULL;
    IAudioSessionControl2* pSessionControl2 = NULL;

    // Get the current list of sessions.
    hr = pSessionManager->GetSessionEnumerator(&pSessionList);

    // Get the session count.
    hr = pSessionList->GetCount(&cbSessionCount);
    //wprintf_s(L"Session count: %d\n", cbSessionCount);

    for (int index = 0; index < cbSessionCount; index++)
    {
        CoTaskMemFree(pswSession);
        SAFE_RELEASE(pSessionControl);

        // Get the <n>th session.
        hr = pSessionList->GetSession(index, &pSessionControl);

        hr = pSessionControl->QueryInterface(
            __uuidof(IAudioSessionControl2), (void**)&pSessionControl2);

        // Exclude system sound session
        hr = pSessionControl2->IsSystemSoundsSession();
        if (S_OK == hr)
        {
            //wprintf_s(L" this is a system sound.\n");
            continue;
        }

        // Optional. Determine which application is using Microphone for recording
        LPWSTR instId = NULL;
        hr = pSessionControl2->GetSessionInstanceIdentifier(&instId);
        if (S_OK == hr)
        {
            //wprintf_s(L"SessionInstanceIdentifier: %s\n", instId);
        }

        AudioSessionState state;
        hr = pSessionControl->GetState(&state);
        switch (state)
        {
        case AudioSessionStateInactive:
            //wprintf_s(L"Session state: Inactive\n");
            break;
        case AudioSessionStateActive:
            // #3 Active state indicates it is recording, otherwise is not recording.
            //wprintf_s(L"Session state: Active\n");
            result = TRUE;
            break;
        case AudioSessionStateExpired:
            //wprintf_s(L"Session state: Expired\n");
            break;
        }
    }

    // Clean up.
    SAFE_RELEASE(pSessionControl);
    SAFE_RELEASE(pSessionList);
    SAFE_RELEASE(pSessionControl2);
    SAFE_RELEASE(pSessionManager);
    SAFE_RELEASE(pEnumerator);

    return result;

}


void WindowsService::Run() {
	//GetCurrentActiveSessionId();
	this->m_logger->ConsoleLog("Windows Service app");
	char username[UNLEN + 1];
	DWORD username_len = UNLEN + 1;
	GetUserNameA(username, &username_len);
	string old_title, wnd_title;
    bool prev_mic_status = false;

    //this->GetCurrentActiveSessions();

	while (1)
	{
		if (this->GetActiveWindowTitle(wnd_title))
		{
			if (old_title != wnd_title)
			{
				this->m_logger->Log(LOG_APP, "USER: %s APP: %s", username, wnd_title.c_str());
			}
			old_title = wnd_title;
		}
        bool mic_status = this->IsMicrophoneRecording();
        if (!prev_mic_status && mic_status)
        {
            this->m_logger->Log(LOG_CAM, "USER: %s MIC_ON", username);
        }
        prev_mic_status = mic_status;
		Sleep(100);
	}
}
