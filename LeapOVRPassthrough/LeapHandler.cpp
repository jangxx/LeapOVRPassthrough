#include "stdafx.h"
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

LeapHandler::LeapHandler()
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

	if (result == eLeapRS_Success) {
		result = LeapOpenConnection(m_connection);
		if (result == eLeapRS_Success) {
			m_started = true;

			m_pollingThread = std::thread([this]() {
				this->pollController();
			});
			
			return true;
		} else {
			printLeapRSError(result);
			m_started = false;
			return false;
		}
	} else {
		printLeapRSError(result);
		m_started = false;
		return false;
	}
}

void LeapHandler::join()
{
	if (m_started && m_pollingThread.joinable()) {
		m_pollingThread.join();
	}
}

void LeapHandler::pollController()
{
	eLeapRS result;
	LEAP_CONNECTION_MESSAGE msg;

	while (m_started) {
		result = LeapPollConnection(m_connection, 1000, &msg);

		std::cout << eventMsgMap[msg.type] << std::endl;

		switch (msg.type) {
		case eLeapEventType_LogEvents:
			std::cout << "Messages (" << msg.size << "):" << std::endl;
			const LEAP_LOG_EVENTS* events = msg.log_events;

			for (uint32_t i = 0; i < events->nEvents; i++) {
				LEAP_LOG_EVENT evt = events->events[i];

				std::cout << "[" << evt.timestamp << "] " << evt.message << std::endl;
			}

			break;
		}
	}
}