#include "stdafx.h"
#include "OVROverlayController.h"

OVROverlayController* s_shareInstance = nullptr;

OVROverlayController * OVROverlayController::getInstance()
{
	if (s_shareInstance == nullptr) {
		s_shareInstance = new OVROverlayController();
	}

	return s_shareInstance;
}

OVROverlayController::OVROverlayController()
	: m_eLastHmdError( vr::VRInitError_None ),
	  m_eCompositorError ( vr::VRInitError_None ),
	  m_eOverlayError( vr::VRInitError_None ),
	  m_ulOverlayHandle( vr::k_ulOverlayHandleInvalid )
{
}


OVROverlayController::~OVROverlayController()
{
}

bool OVROverlayController::init()
{
	bool success = true;

	success = connectToVRRuntime();

	success &= vr::VRCompositor() != NULL;

	if (vr::VROverlay()) {
		std::string name = "Leap Motion Overlay";
		std::string key = "de.literalchaos.leap_motion_ovr_overlay";
		vr::VROverlayError overlayError = vr::VROverlay()->CreateOverlay(key.c_str(), name.c_str(), &m_ulOverlayHandle);

		success &= overlayError == vr::VROverlayError_None;
	}

	if (success) {
		vr::VROverlayError err = vr::VROverlay()->SetOverlayWidthInMeters(m_ulOverlayHandle, 1.5f);
		if (err != vr::VROverlayError_None) {
			std::cout << "SetOverlayWidthInMeters error: " << err << std::endl;
		}

		vr::HmdMatrix34_t position = {
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, -1.0f
		};

		err = vr::VROverlay()->SetOverlayTransformTrackedDeviceRelative(m_ulOverlayHandle, 0, &position);

		if (err != vr::VROverlayError_None) {
			std::cout << "SetOverlayTransformAbsolute error: " << err << std::endl;
		}

		std::cout << "Successfully created overlay" << std::endl;
	} else {
		std::cout << "Error creating overlay" << std::endl;
	}

	return true;
}

void OVROverlayController::showOverlay()
{
	vr::VROverlayError err = vr::VROverlay()->ShowOverlay(m_ulOverlayHandle);

	if (err != vr::VROverlayError_None) {
		std::cout << "showOverlay error: " << err << std::endl;
	}
}

void OVROverlayController::hideOverlay()
{
	vr::VROverlayError err = vr::VROverlay()->HideOverlay(m_ulOverlayHandle);

	if (err != vr::VROverlayError_None) {
		std::cout << "hideOverlay error: " << err << std::endl;
	}
}

void OVROverlayController::toggleOverlay()
{
	if (vr::VROverlay()->IsOverlayVisible(m_ulOverlayHandle)) {
		vr::VROverlay()->HideOverlay(m_ulOverlayHandle);
	} else {
		vr::VROverlay()->ShowOverlay(m_ulOverlayHandle);
	}
}

void OVROverlayController::setTexture(GLuint id)
{
	vr::Texture_t texture;
	texture.handle = (void*)(uintptr_t)id;
	texture.eType = vr::TextureType_OpenGL;
	texture.eColorSpace = vr::ColorSpace_Auto;

	vr::VROverlayError err = vr::VROverlay()->SetOverlayTexture(m_ulOverlayHandle, &texture);

	if (err != vr::VROverlayError_None) {
		std::cout << "setTexture error: " << err << std::endl;
	}
}

bool OVROverlayController::BHMDAvailable()
{
	return vr::VRSystem() != NULL;
}

bool OVROverlayController::connectToVRRuntime()
{
	m_eLastHmdError = vr::VRInitError_None;
	vr::IVRSystem *pVRSystem = vr::VR_Init(&m_eLastHmdError, vr::VRApplication_Overlay);

	/*if (m_eLastHmdError != vr::VRInitError_None) {
		m_strVRDriver = "No Driver";
		m_strVRDisplay = "No Display";
		return false;
	}

	m_strVRDriver = GetTrackedDeviceString(pVRSystem, vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_TrackingSystemName_String);
	m_strVRDisplay = GetTrackedDeviceString(pVRSystem, vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_SerialNumber_String);*/

	return true;
}

void OVROverlayController::disconnectFromVRRuntime()
{
	vr::VR_Shutdown();
}
