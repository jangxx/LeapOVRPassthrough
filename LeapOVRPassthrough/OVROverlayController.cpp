#include "stdafx.h"
#include "OVROverlayController.h"


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
		vr::VROverlayError overlayError = vr::VROverlay()->CreateDashboardOverlay(key.c_str(), name.c_str(), &m_ulOverlayHandle, &m_ulOverlayThumbnailHandle);

		success &= overlayError == vr::VROverlayError_None;
	}

	if (success) {
		vr::VROverlay()->SetOverlayWidthInMeters(m_ulOverlayHandle, 1.5f);
	}

	return true;
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
