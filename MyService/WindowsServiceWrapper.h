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
	SERVICE_STATUS m_serviceStatus = { 0 };
	SERVICE_STATUS_HANDLE m_statusHandle = nullptr;
	inline static HANDLE s_serviceStopEvent = INVALID_HANDLE_VALUE;
	inline static WindowsServiceWrapper* s_singletonInstance = nullptr;


	//original definition was like #define serviceName "ServiceName"
	inline static wchar_t s_serviceName[1024] = _T("MyService"); //there were errors with size < 1024 (TODO confirm error with lower length)

	// macro which defines const array
	SERVICE_TABLE_ENTRY m_serviceTable[2] =
	{
		{s_serviceName, staticServiceMain},
		{nullptr, nullptr}
	};

	static void WINAPI staticServiceMain(DWORD argc, LPTSTR *argv);
	static void WINAPI staticServiceCtrlHandler(DWORD CtrlCode);
	static DWORD WINAPI staticServiceWorkerThread(LPVOID lpParam);

	void WINAPI serviceMain(DWORD argc, LPTSTR *argv);
	void WINAPI serviceCtrlHandler(DWORD CtrlCode);
	DWORD WINAPI serviceWorkerThread(LPVOID lpParam);
	
	WindowsServiceWrapper(); 
public:

	~WindowsServiceWrapper();

	// Inherited via IServiceWrapper
	virtual DWORD start() override;
	static WindowsServiceWrapper* getInstance();
};

