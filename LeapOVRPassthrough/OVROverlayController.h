#pragma once

#include <GL/glew.h>
//#include <GL/freeglut.h>
#include <openvr.h>
#include <iostream>

class OVROverlayController
{
public:
	static OVROverlayController* getInstance();

	OVROverlayController();
	~OVROverlayController();

	bool init();
	void shutdown();
	//void enableRestart();

	void showOverlay();
	void hideOverlay();
	void setTexture(GLuint* id);

	bool BHMDAvailable();
	//vr::IVRSystem* getVRSystem();
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
};

