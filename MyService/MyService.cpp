// MyService.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "pch.h"
#include <Windows.h>
#include <Tchar.h> // for _T macro
#include <iostream>

#include "WindowsServiceWrapper.h"

#include <chrono> // for sleep
#include <thread> // for sleep

/**
 * App entry point. It is possible to install service via code and run it with argument install (not implemented here)
 * @argc
 * @argv
 * @result 0 when without error
 */
int _tmain(int argc, TCHAR *argv[])
{
	WindowsServiceWrapper* windowsService = new WindowsServiceWrapper();
	auto result = windowsService->start();
	delete windowsService;
	return result;
}

