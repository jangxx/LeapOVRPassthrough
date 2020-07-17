#pragma once

#include <Windows.h>
#include <GL/glew.h>
#include <openvr.h>
#include <iostream>
#include <filesystem>

#define FRAMEBUFFER_WIDTH 640
#define FRAMEBUFFER_HEIGHT 480

class OVROverlayController
{
public:
	static OVROverlayController* getInstance();

	OVROverlayController();
	~OVROverlayController();

	bool init();
	void shutdown();

	void pollEvents();
	bool isConnected();

	void showOverlay();
	void hideOverlay();
	void toggleOverlay();
	void setTexture(GLuint id);

	void installManifest();
	void removeManifest();
	bool isManifestInstalled();

	bool BHMDAvailable();
	//vr::IVRSystem* getVRSystem();
	vr::HmdError getLastHmdError();

private:
	bool connectToVRRuntime();
	void disconnectFromVRRuntime();

	bool m_connected { false };
	vr::IVRSystem* m_VRSystem { nullptr };

	std::string m_strVRDriver { "No Driver" };
	std::string m_strVRDisplay { "No Display" };

	vr::TrackedDevicePose_t m_rTrackedDevicePose[vr::k_unMaxTrackedDeviceCount];
	vr::HmdError m_eLastHmdError;

	vr::HmdError m_eCompositorError;
	vr::HmdError m_eOverlayError;
	vr::VROverlayHandle_t m_ulOverlayHandle;
};

