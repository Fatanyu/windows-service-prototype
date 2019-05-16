#include "pch.h"
#include "ServiceControlManagerWrapper.h"

ServiceControlManagerWrapper::ServiceControlManagerWrapper(wchar_t *serviceName, LPHANDLER_FUNCTION serviceCtrlHandler) : ServiceControlManagerWrapper(RegisterServiceCtrlHandler(serviceName, serviceCtrlHandler))
{
}
ServiceControlManagerWrapper::ServiceControlManagerWrapper(SERVICE_STATUS_HANDLE handle)
{
	m_statusHandle = handle;
	SecureZeroMemory(&m_serviceStatus, sizeof(m_serviceStatus));
}

ServiceControlManagerWrapper::~ServiceControlManagerWrapper()
{
}

bool ServiceControlManagerWrapper::sendServiceStatusToSCM(ServiceStateSCM newState)
{
	switch (newState)
	{
	case ServiceStateSCM::CONTINUE_PENDING:
		this->setContinuePending();
		break;
	case ServiceStateSCM::RUNNING:
		this->setRunning();
		break;
	case ServiceStateSCM::START_PENDING:
		this->setStartPending();
		break;
	case ServiceStateSCM::STOP_PENDING:
		this->setStopPending();
		break;
	case ServiceStateSCM::STOPPED:
		this->setStopped();
		break;
	default:
		//action is not needed
		break;
	}
	return this->updateServiceControlManagerWithOurState();
}

void ServiceControlManagerWrapper::setContinuePending()
{
}

void ServiceControlManagerWrapper::setRunning()
{
	// Tell the service controller we are started
	m_serviceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
	m_serviceStatus.dwCurrentState = SERVICE_RUNNING;
	m_serviceStatus.dwWin32ExitCode = 0;
	m_serviceStatus.dwCheckPoint = 0;
}

void ServiceControlManagerWrapper::setStartPending()
{
	// Tell the service controller we are starting
	m_serviceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	m_serviceStatus.dwControlsAccepted = 0;
	m_serviceStatus.dwCurrentState = SERVICE_START_PENDING;
	m_serviceStatus.dwWin32ExitCode = 0;
	m_serviceStatus.dwServiceSpecificExitCode = 0;
	m_serviceStatus.dwCheckPoint = 0;
}

void ServiceControlManagerWrapper::setStopPending()
{
	m_serviceStatus.dwControlsAccepted = 0;
	m_serviceStatus.dwCurrentState = SERVICE_STOP_PENDING;
	m_serviceStatus.dwWin32ExitCode = 0;
	m_serviceStatus.dwCheckPoint = 4;
}

void ServiceControlManagerWrapper::setStopped()
{
	// Tell the service controller we are stopped
	m_serviceStatus.dwControlsAccepted = 0;
	m_serviceStatus.dwCurrentState = SERVICE_STOPPED;
	m_serviceStatus.dwWin32ExitCode = 0;
	m_serviceStatus.dwCheckPoint = 3;
}

bool ServiceControlManagerWrapper::updateServiceControlManagerWithOurState()
{
	return SetServiceStatus(m_statusHandle, &m_serviceStatus);
}

bool ServiceControlManagerWrapper::isConnectedToSCM()
{
	return m_statusHandle != nullptr;
}
