#pragma once
#include <Windows.h>
#include <Tchar.h> // for _T macro

#include "IServiceWrapper.h"

class WindowsServiceWrapper : public IServiceWrapper
{
private:
	//
	// Microsoft contants for services
	//
	inline static SERVICE_STATUS s_serviceStatus = { 0 };
	inline static SERVICE_STATUS_HANDLE s_statusHandle = NULL;
	inline static HANDLE s_serviceStopEvent = INVALID_HANDLE_VALUE;

	//original definition was like #define serviceName "ServiceName"
	inline static wchar_t s_serviceName[1024] = _T("MyService"); //there were errors with size < 1024 (TODO confirm error with lower length)

	// macro which defines const array
	SERVICE_TABLE_ENTRY m_serviceTable[2] =
	{
		{s_serviceName, serviceMain},
		{NULL, NULL}
	};

	static void WINAPI serviceMain(DWORD argc, LPTSTR *argv);
	static void WINAPI serviceCtrlHandler(DWORD CtrlCode);
	static DWORD WINAPI serviceWorkerThread(LPVOID lpParam);
public:
	WindowsServiceWrapper();
	~WindowsServiceWrapper();

	// Inherited via IServiceWrapper
	virtual DWORD start() override;
};

