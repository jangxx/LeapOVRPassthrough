#include "LeapHandler.h"

std::map<eLeapRS, std::string> errorMap = {
	{ eLeapRS_Success, "Success" },
	{ eLeapRS_UnknownError, "Unknown Error" },
	{ eLeapRS_InvalidArgument, "Invalid Argument" },
	{ eLeapRS_InsufficientResources, "Insufficient Resources" },
	{ eLeapRS_InsufficientBuffer, "Insufficient Buffer" },
	{ eLeapRS_Timeout, "Timeout" },
	{ eLeapRS_NotConnected, "Not Connected" },
	{ eLeapRS_HandshakeIncomplete, "Handshake Incomplete" },
	{ eLeapRS_BufferSizeOverflow, "Buffer Size Overflow" },
	{ eLeapRS_ProtocolError, "Protocol Error" },
	{ eLeapRS_InvalidClientID, "Invalid Client ID" },
	{ eLeapRS_UnexpectedClosed, "Unexpected Closed" },
	{ eLeapRS_UnknownImageFrameRequest, "Unknown Image Frame Request" },
	{ eLeapRS_UnknownTrackingFrameID, "Unknown Tracking Frame ID" },
	{ eLeapRS_RoutineIsNotSeer, "Routine Is Not Seer" },
	{ eLeapRS_TimestampTooEarly, "Timestamp Too Early" },
	{ eLeapRS_ConcurrentPoll, "Concurrent Poll" },
	{ eLeapRS_NotAvailable, "Not Available" },
	{ eLeapRS_NotStreaming, "Not Streaming" },
	{ eLeapRS_CannotOpenDevice, "Cannot Open Device" },
};

std::map<eLeapEventType, std::string> eventMsgMap = {
	{ eLeapEventType_None, "None" },
	{ eLeapEventType_Connection, "Connection" },
	{ eLeapEventType_ConnectionLost, "Connection Lost" },
	{ eLeapEventType_Device, "Device " },
	{ eLeapEventType_DeviceFailure, "Device Failure" },
	{ eLeapEventType_Policy, "Policy" },
	{ eLeapEventType_Tracking, "Tracking" },
	{ eLeapEventType_ImageRequestError, "Image Request Error" },
	{ eLeapEventType_ImageComplete, "Image Complete" },
	{ eLeapEventType_LogEvent, "Log Event" },
	{ eLeapEventType_DeviceLost, "Device Lost" },
	{ eLeapEventType_ConfigResponse, "Config Response" },
	{ eLeapEventType_ConfigChange, "Config Change" },
	{ eLeapEventType_DeviceStatusChange, "Device Status Change" },
	{ eLeapEventType_Image, "Image" },
	{ eLeapEventType_PointMappingChange, "Point Mapping Change" },
	{ eLeapEventType_LogEvents, "Log Events" },
};

void printLeapRSError(eLeapRS result) {
	std::cout << "LeapC Error: " << errorMap[result] << " " << result << std::endl;
}

LeapHandler* s_sharedInstance = nullptr;

LeapHandler* LeapHandler::getInstance()
{
	if (s_sharedInstance == nullptr) {
		s_sharedInstance = new LeapHandler();
	}

	return s_sharedInstance;
}

LeapHandler::LeapHandler() :
	m_brightUpperPixelsRingbuffer(std::vector<uint32_t>(50)),
	m_lastSwipeDetected( steady_clock::now() )
{
}


LeapHandler::~LeapHandler()
{
}

bool LeapHandler::openConnection()
{
	if (m_started) {
		return false;
	}

	eLeapRS result = LeapCreateConnection(NULL, &m_connection);

	if (result != eLeapRS_Success) {
		printLeapRSError(result);
		m_started = false;
		return false;
	}

	result = LeapOpenConnection(m_connection);
	if (result != eLeapRS_Success) {
		printLeapRSError(result);
		m_started = false;
		return false;
	}

	LEAP_CONNECTION_MESSAGE msg;
	result = LeapPollConnection(m_connection, 1000, &msg);
	if (result != eLeapRS_Success) {
		printLeapRSError(result);
		m_started = false;
		return false;
	}

	result = LeapSetPolicyFlags(m_connection, eLeapPolicyFlag_Images | eLeapPolicyFlag_OptimizeHMD, 0);
	if (result != eLeapRS_Success) {
		printLeapRSError(result);
		m_started = false;
		return false;
	}

	m_started = true;
	m_pollingThread = std::thread([this]() {
		this->pollController();
	});
			
	return true;
}

bool LeapHandler::swipeDetected()
{
	bool result = m_swipeDetected;
	m_swipeDetected = false;
	return result;
}

void LeapHandler::join()
{
	if (m_started && m_pollingThread.joinable()) {
		m_started = false;
		m_pollingThread.join();
	}
}

void LeapHandler::pollController()
{
	eLeapRS result;
	LEAP_CONNECTION_MESSAGE msg;

	while (m_started) {
		result = LeapPollConnection(m_connection, 1000, &msg);

		//std::cout << eventMsgMap[msg.type] << std::endl;

		switch (msg.type) {
			case eLeapEventType_LogEvents: 
			{
				const LEAP_LOG_EVENTS* events = msg.log_events;
				//std::cout << "Messages (" << events->nEvents << "):" << std::endl;

				for (uint32_t i = 0; i < events->nEvents; i++) {
					LEAP_LOG_EVENT evt = events->events[i];

					std::cout << "[" << evt.timestamp << "] " << evt.message << std::endl;
				}

				break;
			}
			case eLeapEventType_LogEvent:
			{
				const LEAP_LOG_EVENT* evt = msg.log_event;
			
				std::cout << "[" << evt->timestamp << "] " << evt->message << std::endl;

				break;
			}
			case eLeapEventType_Image: 
			{
				const LEAP_IMAGE_EVENT* evt = msg.image_event;
				GraphicsManager* graphicsManager = GraphicsManager::getInstance();

				updateBrightUpperPixels(evt->image[0].properties.width, evt->image[0].properties.height, (uint8_t*)evt->image[0].data + evt->image[0].offset);

				if (countLastBUPIncreasing() >= 5) {
					auto cur_ts = steady_clock::now();

					if (std::chrono::duration_cast<std::chrono::seconds>(cur_ts - m_lastSwipeDetected) > 2 * std::chrono::seconds()) {
						m_lastSwipeDetected = cur_ts;
						m_swipeDetected = true;
					}
				}

				graphicsManager->setFrame(evt->image[0].properties.width, evt->image[0].properties.height, (uint8_t*)evt->image[0].data + evt->image[0].offset);
				break;
			}
		}
	}
}

void LeapHandler::updateBrightUpperPixels(int width, int height, uint8_t * image)
{
	const uint8_t threshold = 100;

	uint32_t count = 0;

	for (int y = 0; y < height / 2; y++) {
		for (int x = 0; x < width; x++) {
			int i = y * width + x;

			if (image[i] >= threshold) {
				count++;
			}
		}
	}

	//std::cout << "count: " << count << std::endl;

	m_brightUpperPixelsRingbuffer[m_bupRBNextIndex] = count;
	m_bupRBNextIndex++;

	if (m_bupRBNextIndex == m_brightUpperPixelsRingbuffer.size()) {
		m_bupRBNextIndex = 0;
	}
}

uint32_t LeapHandler::countLastBUPIncreasing()
{
	const uint32_t threshold = 30000;

	uint32_t result = 0;

	uint32_t last = UINT32_MAX;

	for (int i = 1; i <= m_brightUpperPixelsRingbuffer.size(); i++) {
		int index = m_bupRBNextIndex - i;
		index = (index < 0) ? index + static_cast<int>(m_brightUpperPixelsRingbuffer.size()) : index;

		uint32_t value = m_brightUpperPixelsRingbuffer[index];

		if (value < threshold) {
			break;
		}

		if (value < last) {
			result++;
			last = value;
		} else {
			break;
		}
	}

	return result;
}
