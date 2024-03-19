#pragma once

#include <Windows.h>
#include <GL/glew.h>
#include <openvr.h>
#include <iostream>
#include <filesystem>

#include "utils.h"


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
	void setOverlayRotation(int rotation);
	void setOverlayAlpha(float alpha);
	void setOverlayWidth(float width);
	float getOverlayWidth();
	void setOverlayDistance(float zDistance);
	float getOverlayDistance();

	void installManifest();
	void removeManifest();
	bool isManifestInstalled();

	bool BHMDAvailable();
	//vr::IVRSystem* getVRSystem();
	vr::HmdError getLastHmdError();

private:
	bool connectToVRRuntime();
	void disconnectFromVRRuntime();
	void updateOverlaySizeAndPosition();
	vr::HmdMatrix34_t createOverlayMatrix(float zDistance);

	bool m_connected { false };
	int m_overlayRotation{ 0 };
	float m_overlayWidth{ 0.5f };
	float m_overlayZDistance{ 0.3f };
	vr::IVRSystem* m_VRSystem { nullptr };

	std::string m_strVRDriver { "No Driver" };
	std::string m_strVRDisplay { "No Display" };

	vr::TrackedDevicePose_t m_rTrackedDevicePose[vr::k_unMaxTrackedDeviceCount];
	vr::HmdError m_eLastHmdError;

	vr::HmdError m_eCompositorError;
	vr::HmdError m_eOverlayError;
	vr::VROverlayHandle_t m_ulOverlayHandle;
};

