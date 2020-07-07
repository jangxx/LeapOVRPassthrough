#pragma once

#include "include/openvr.h"

class OVROverlayController
{
public:
	OVROverlayController();
	~OVROverlayController();

	bool init();
	void shutdown();
	void enableRestart();

	bool BHMDAvailable();
	vr::IVRSystem* getVRSystem();
	vr::HmdError getLastHmdError();

private:
	bool connectToVRRuntime();
	void disconnectFromVRRuntime();

	std::string m_strVRDriver { "No Driver" };
	std::string m_strVRDisplay { "No Display" };

	vr::TrackedDevicePose_t m_rTrackedDevicePose[vr::k_unMaxTrackedDeviceCount];
	vr::HmdError m_eLastHmdError;

	vr::HmdError m_eCompositorError;
	vr::HmdError m_eOverlayError;
	vr::VROverlayHandle_t m_ulOverlayHandle;
	vr::VROverlayHandle_t m_ulOverlayThumbnailHandle;
};

