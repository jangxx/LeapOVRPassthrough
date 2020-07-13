#pragma once
#include <thread>
#include <string>
#include <iostream>
#include <map>
#include <vector>
#include <chrono>

#include "GraphicsManager.h"

extern "C" {
	#include <LeapC.h>
}

class LeapHandler
{
	using steady_clock = std::chrono::steady_clock;
	using time_point = steady_clock::time_point;
	using duration = steady_clock::duration;

public:
	static LeapHandler* getInstance();

	LeapHandler();
	~LeapHandler();

	bool openConnection();
	bool swipeDetected();
	void join();

private:
	void pollController();
	void updateBrightUpperPixels(int width, int height, uint8_t* image);
	uint32_t countLastBUPIncreasing();

	std::thread m_pollingThread;
	bool m_started { false };
	bool m_swipeDetected { false };

	LEAP_CONNECTION m_connection;

	std::vector<uint32_t> m_brightUpperPixelsRingbuffer;
	uint32_t m_bupRBNextIndex { 0 };

	time_point m_lastSwipeDetected;
};

