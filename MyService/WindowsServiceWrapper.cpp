#include "pch.h"
#include "WindowsServiceWrapper.h"



WindowsServiceWrapper::WindowsServiceWrapper()
{
}

WindowsServiceWrapper::~WindowsServiceWrapper()
{
	WindowsServiceWrapper::s_singletonInstance = nullptr;
	delete m_serviceControlManagementWrapper;
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
	m_serviceControlManagementWrapper = new ServiceControlManagerWrapper(RegisterServiceCtrlHandler(s_serviceName, staticServiceCtrlHandler));

//	if (m_statusHandle == nullptr)
	if (!this->m_serviceControlManagementWrapper->isConnectedToSCM())
	{
		//TODO error
		return;
	}

	// Tell the service controller we are starting
	if (!this->m_serviceControlManagementWrapper->sendServiceStatusToSCM(ServiceStateSCM::START_PENDING))
	{
		//TODO error
	}

	/*
	 * Perform tasks necessary to start the service here
	 */

	 // Create a service stop event to wait on later
	s_serviceStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (s_serviceStopEvent == nullptr)
	{
		// Error creating event
		// Tell service controller we are stopped and exit
		this->m_serviceControlManagementWrapper->sendServiceStatusToSCM(ServiceStateSCM::STOPPED);
	}

	// Tell the service controller we are started
	if (!this->m_serviceControlManagementWrapper->sendServiceStatusToSCM(ServiceStateSCM::RUNNING))
	{
		//TODO error
	}


	// Start a thread that will perform the main task of the service
	std::thread workerThread(&WindowsServiceWrapper::serviceWorkerThread, this, nullptr);
	// Wait until our worker thread exits signaling that the service needs to stop

	workerThread.join();

	/*
	 * Perform any cleanup tasks
	 */

	CloseHandle(s_serviceStopEvent);

	// Tell the service controller we are stopped
	if (!this->m_serviceControlManagementWrapper->sendServiceStatusToSCM(ServiceStateSCM::STOPPED))
	{
		//TODO error
	}

	return;
}

void WINAPI WindowsServiceWrapper::serviceCtrlHandler(DWORD CtrlCode)
{
	switch (CtrlCode)
	{
	case SERVICE_CONTROL_STOP:
		/*
		 * Perform tasks necessary to stop the service here
		 */

		if (!this->m_serviceControlManagementWrapper->sendServiceStatusToSCM(ServiceStateSCM::STOP_PENDING))
		{
			//TODO error
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
