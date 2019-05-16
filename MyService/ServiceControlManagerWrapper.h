#pragma once
#include <Windows.h>
enum class ServiceStateSCM : DWORD
{
	CONTINUE_PENDING = SERVICE_CONTINUE_PENDING,
	PAUSE_PENDING = SERVICE_PAUSE_PENDING,
	PAUSED = SERVICE_PAUSED,
	RUNNING = SERVICE_RUNNING,
	START_PENDING = SERVICE_START_PENDING,
	STOP_PENDING = SERVICE_STOP_PENDING,
	STOPPED = SERVICE_STOPPED
};

/*
 * This class works only inside StartServiceCtrlDispatcher() - otherwise RegisterServiceCtrlHandler() will always be nullptr
 */
class ServiceControlManagerWrapper
{
	//https://docs.microsoft.com/en-us/windows/desktop/api/winsvc/ns-winsvc-_service_status
	

	SERVICE_STATUS_HANDLE m_statusHandle = nullptr;
	SERVICE_STATUS m_serviceStatus = { 0 };

	void setContinuePending();
	void setRunning();
	void setStartPending();
	void setStopPending();
	void setStopped();

	bool updateServiceControlManagerWithOurState();
public:
	ServiceControlManagerWrapper(wchar_t* serviceName, LPHANDLER_FUNCTION serviceCtrlHandler);
	ServiceControlManagerWrapper(SERVICE_STATUS_HANDLE handle);
	~ServiceControlManagerWrapper();

	bool isConnectedToSCM();
	bool sendServiceStatusToSCM(ServiceStateSCM newState);
};

