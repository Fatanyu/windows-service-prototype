#include "pch.h"
#include "WindowsServiceWrapper.h"



WindowsServiceWrapper::WindowsServiceWrapper()
{
}


WindowsServiceWrapper::~WindowsServiceWrapper()
{
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
	s_statusHandle = RegisterServiceCtrlHandler(s_serviceName, serviceCtrlHandler);

	if (s_statusHandle == NULL)
	{
		//goto EXIT;
		return;
	}

	// Tell the service controller we are starting
	ZeroMemory(&s_serviceStatus, sizeof(s_serviceStatus));
	s_serviceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	s_serviceStatus.dwControlsAccepted = 0;
	s_serviceStatus.dwCurrentState = SERVICE_START_PENDING;
	s_serviceStatus.dwWin32ExitCode = 0;
	s_serviceStatus.dwServiceSpecificExitCode = 0;
	s_serviceStatus.dwCheckPoint = 0;

	if (SetServiceStatus(s_statusHandle, &s_serviceStatus) == FALSE)
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
		s_serviceStatus.dwControlsAccepted = 0;
		s_serviceStatus.dwCurrentState = SERVICE_STOPPED;
		s_serviceStatus.dwWin32ExitCode = GetLastError();
		s_serviceStatus.dwCheckPoint = 1;

		if (SetServiceStatus(s_statusHandle, &s_serviceStatus) == FALSE)
		{
			OutputDebugString(_T(
				"My Sample Service: ServiceMain: SetServiceStatus returned error"));
		}
		return;
	}

	// Tell the service controller we are started
	s_serviceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
	s_serviceStatus.dwCurrentState = SERVICE_RUNNING;
	s_serviceStatus.dwWin32ExitCode = 0;
	s_serviceStatus.dwCheckPoint = 0;

	if (SetServiceStatus(s_statusHandle, &s_serviceStatus) == FALSE)
	{
		OutputDebugString(_T(
			"My Sample Service: ServiceMain: SetServiceStatus returned error"));
	}

	// Start a thread that will perform the main task of the service
	HANDLE hThread = CreateThread(NULL, 0, WindowsServiceWrapper::serviceWorkerThread, NULL, 0, NULL);

	// Wait until our worker thread exits signaling that the service needs to stop
	WaitForSingleObject(hThread, INFINITE);


	/*
	 * Perform any cleanup tasks
	 */

	CloseHandle(s_serviceStopEvent);

	// Tell the service controller we are stopped
	s_serviceStatus.dwControlsAccepted = 0;
	s_serviceStatus.dwCurrentState = SERVICE_STOPPED;
	s_serviceStatus.dwWin32ExitCode = 0;
	s_serviceStatus.dwCheckPoint = 3;

	if (SetServiceStatus(s_statusHandle, &s_serviceStatus) == FALSE)
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

		if (s_serviceStatus.dwCurrentState != SERVICE_RUNNING)
			break;

		/*
		 * Perform tasks necessary to stop the service here
		 */

		s_serviceStatus.dwControlsAccepted = 0;
		s_serviceStatus.dwCurrentState = SERVICE_STOP_PENDING;
		s_serviceStatus.dwWin32ExitCode = 0;
		s_serviceStatus.dwCheckPoint = 4;

		if (SetServiceStatus(s_statusHandle, &s_serviceStatus) == FALSE)
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
