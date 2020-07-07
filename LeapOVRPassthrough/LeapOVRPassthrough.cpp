#include "stdafx.h"
#include "windows.h"
#include <iostream>

#include "LeapHandler.h"
#include "OVROverlayController.h"

int APIENTRY WinMain(HINSTANCE /*hInstance*/,
	HINSTANCE /*hPrevInstance*/,
	LPSTR /*lpCmdLine*/,
	int /*cmdShow*/)
{
	std::cout << "Hello World" << std::endl;

	OVROverlayController* vrController = new OVROverlayController();
	LeapHandler* leapHandler = new LeapHandler();

	vrController->init();
	leapHandler->openConnection();
	leapHandler->join();

    return 0;
}

int main()
{
	return WinMain(GetModuleHandle(NULL), NULL, GetCommandLineA(), SW_SHOWNORMAL);
}