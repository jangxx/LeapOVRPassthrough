#pragma once
#include <thread>
#include <string>
#include <iostream>
#include <map>

extern "C" {
	#include "include\LeapC.h"
}

class LeapHandler
{
public:
	LeapHandler();
	~LeapHandler();

	bool openConnection();
	void join();

private:
	void pollController();

	std::thread m_pollingThread;
	bool m_started { false };

	LEAP_CONNECTION m_connection;
};

