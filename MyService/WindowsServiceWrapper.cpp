#include "pch.h"
#include "WindowsServiceWrapper.h"



WindowsServiceWrapper::WindowsServiceWrapper()
{
}

WindowsServiceWrapper::~WindowsServiceWrapper()
{
	WindowsServiceWrapper::s_singletonInstance = nullptr;
}

DWORD WindowsServiceWrapper::start()
{
	if (StartServiceCtrlDispatcher(m_serviceTable) == false)
	{
		auto ctrlDispatcherError = GetLastError();
		//TODO error log
		return ctrlDispatcherError;
	}
	return 0;
}



void WINAPI WindowsServiceWrapper::serviceMain(DWORD argc, LPTSTR * argv)
{
	// Register our service control handler with the SCM
	m_statusHandle = RegisterServiceCtrlHandler(s_serviceName, staticServiceCtrlHandler);

	if (m_statusHandle == nullptr)
	{
		//goto EXIT;
		return;
	}

	// Tell the service controller we are starting
	SecureZeroMemory(&m_serviceStatus, sizeof(m_serviceStatus));
	m_serviceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	m_serviceStatus.dwControlsAccepted = 0;
	m_serviceStatus.dwCurrentState = SERVICE_START_PENDING;
	m_serviceStatus.dwWin32ExitCode = 0;
	m_serviceStatus.dwServiceSpecificExitCode = 0;
	m_serviceStatus.dwCheckPoint = 0;

	if (SetServiceStatus(m_statusHandle, &m_serviceStatus) == false)
	{
		OutputDebugString(_T(
			"My Sample Service: ServiceMain: SetServiceStatus returned error"));
	}

	/*
	 * Perform tasks necessary to start the service here
	 */

	 // Create a service stop event to wait on later
	s_serviceStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (s_serviceStopEvent == NULL)
	{
		// Error creating event
		// Tell service controller we are stopped and exit
		m_serviceStatus.dwControlsAccepted = 0;
		m_serviceStatus.dwCurrentState = SERVICE_STOPPED;
		m_serviceStatus.dwWin32ExitCode = GetLastError();
		m_serviceStatus.dwCheckPoint = 1;

		if (SetServiceStatus(m_statusHandle, &m_serviceStatus) == false)
		{
			OutputDebugString(_T(
				"My Sample Service: ServiceMain: SetServiceStatus returned error"));
		}
		return;
	}

	// Tell the service controller we are started
	m_serviceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
	m_serviceStatus.dwCurrentState = SERVICE_RUNNING;
	m_serviceStatus.dwWin32ExitCode = 0;
	m_serviceStatus.dwCheckPoint = 0;

	if (SetServiceStatus(m_statusHandle, &m_serviceStatus) == false)
	{
		OutputDebugString(_T(
			"My Sample Service: ServiceMain: SetServiceStatus returned error"));
	}

	// Start a thread that will perform the main task of the service
	HANDLE hThread = CreateThread(NULL, 0, WindowsServiceWrapper::staticServiceWorkerThread, NULL, 0, NULL);

	// Wait until our worker thread exits signaling that the service needs to stop
	WaitForSingleObject(hThread, INFINITE);


	/*
	 * Perform any cleanup tasks
	 */

	CloseHandle(s_serviceStopEvent);

	// Tell the service controller we are stopped
	m_serviceStatus.dwControlsAccepted = 0;
	m_serviceStatus.dwCurrentState = SERVICE_STOPPED;
	m_serviceStatus.dwWin32ExitCode = 0;
	m_serviceStatus.dwCheckPoint = 3;

	if (SetServiceStatus(m_statusHandle, &m_serviceStatus) == false)
	{
		OutputDebugString(_T(
			"My Sample Service: ServiceMain: SetServiceStatus returned error"));
	}

	//std::cout << "ServiceMain() ended OK" << std::endl;
	return;
}

void WINAPI WindowsServiceWrapper::serviceCtrlHandler(DWORD CtrlCode)
{
	switch (CtrlCode)
	{
	case SERVICE_CONTROL_STOP:

		if (m_serviceStatus.dwCurrentState != SERVICE_RUNNING)
			break;

		/*
		 * Perform tasks necessary to stop the service here
		 */

		m_serviceStatus.dwControlsAccepted = 0;
		m_serviceStatus.dwCurrentState = SERVICE_STOP_PENDING;
		m_serviceStatus.dwWin32ExitCode = 0;
		m_serviceStatus.dwCheckPoint = 4;

		if (SetServiceStatus(m_statusHandle, &m_serviceStatus) == false)
		{
			OutputDebugString(_T(
				"My Sample Service: ServiceCtrlHandler: SetServiceStatus returned error"));
		}

		// This will signal the worker thread to start shutting down
		SetEvent(s_serviceStopEvent);

		break;

	default:
		break;
	}
}

DWORD WINAPI WindowsServiceWrapper::serviceWorkerThread(LPVOID lpParam)
{
	//  Periodically check if the service has been requested to stop
	while (WaitForSingleObject(s_serviceStopEvent, 0) != WAIT_OBJECT_0)
	{
		/*
		 * Perform main service function here
		 */

		 //  Simulate some work by sleeping
		Sleep(3000);
	}

	return ERROR_SUCCESS;
}


//
// static methods
//
void WINAPI WindowsServiceWrapper::staticServiceMain(DWORD argc, LPTSTR * argv)
{
	WindowsServiceWrapper::getInstance()->serviceMain(argc, argv);
}

void WINAPI WindowsServiceWrapper::staticServiceCtrlHandler(DWORD CtrlCode)
{
	WindowsServiceWrapper::getInstance()->serviceCtrlHandler(CtrlCode);
}

DWORD WINAPI WindowsServiceWrapper::staticServiceWorkerThread(LPVOID lpParam)
{
	return (WindowsServiceWrapper::getInstance()->serviceWorkerThread(lpParam));
}

WindowsServiceWrapper* WindowsServiceWrapper::getInstance()
{
	if (WindowsServiceWrapper::s_singletonInstance == nullptr)
	{
		WindowsServiceWrapper::s_singletonInstance = new WindowsServiceWrapper();
	}
	return WindowsServiceWrapper::s_singletonInstance;
}
