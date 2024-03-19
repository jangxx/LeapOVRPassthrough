#include "OVROverlayController.h"

#define APPLICATION_KEY "de.literalchaos.leap_motion_ovr_overlay"

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
	  m_ulOverlayHandle( vr::k_ulOverlayHandleInvalid ),
	  m_rTrackedDevicePose()
{
}


OVROverlayController::~OVROverlayController()
{
}

bool OVROverlayController::init()
{
	std::stringstream output;
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
		updateOverlaySizeAndPosition();

		output << "Successfully created overlay" << std::endl;
		outputStringStream(output);
	} else {
		output << "Error creating overlay" << std::endl;
		outputStringStream(output);
	}

	return true;
}

void OVROverlayController::shutdown()
{
	disconnectFromVRRuntime();
}

void OVROverlayController::pollEvents()
{
	if (m_VRSystem == nullptr) return;

	uint8_t eventBuffer[1024];
	vr::VREvent_t* evt = (vr::VREvent_t*)eventBuffer;

	if (m_VRSystem->PollNextEvent(evt, 1024)) {
		if (evt->eventType == vr::VREvent_Quit || evt->eventType == vr::VREvent_ProcessQuit) {
			disconnectFromVRRuntime();
		}
	}
}

bool OVROverlayController::isConnected()
{
	return m_connected;
}

void OVROverlayController::showOverlay()
{
	vr::VROverlayError err = vr::VROverlay()->ShowOverlay(m_ulOverlayHandle);

	if (err != vr::VROverlayError_None) {
		std::stringstream output;
		output << "showOverlay error: " << err << std::endl;
		outputStringStream(output);
	}
}

void OVROverlayController::hideOverlay()
{
	vr::VROverlayError err = vr::VROverlay()->HideOverlay(m_ulOverlayHandle);

	if (err != vr::VROverlayError_None) {
		std::stringstream output;
		output << "hideOverlay error: " << err << std::endl;
		outputStringStream(output);
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
		std::stringstream output;
		output << "setTexture error: " << err << std::endl;
		outputStringStream(output);
	}
}

void OVROverlayController::setOverlayRotation(int rotation)
{
	m_overlayRotation = rotation;
	updateOverlaySizeAndPosition();
}

void OVROverlayController::setOverlayAlpha(float alpha)
{
	vr::VROverlayError err = vr::VROverlay()->SetOverlayAlpha(m_ulOverlayHandle, alpha);

	if (err != vr::VROverlayError_None) {
		std::stringstream output;
		output << "SetOverlayAlpha error: " << err << std::endl;
		outputStringStream(output);
	}
}

void OVROverlayController::setOverlayWidth(float width)
{
	m_overlayWidth = width;
	updateOverlaySizeAndPosition();
}

float OVROverlayController::getOverlayWidth()
{
	return m_overlayWidth;
}

void OVROverlayController::setOverlayDistance(float zDistance)
{
	m_overlayZDistance = zDistance;
	updateOverlaySizeAndPosition();
}

float OVROverlayController::getOverlayDistance()
{
	return m_overlayZDistance;
}

void OVROverlayController::installManifest()
{
	std::stringstream output;

	if (!isManifestInstalled()) {
		output << "Installing manifest" << std::endl;
		outputStringStream(output);

		std::filesystem::path manifest_path = std::filesystem::current_path() / "manifest.vrmanifest";

		vr::EVRApplicationError err = vr::VRApplications()->AddApplicationManifest(manifest_path.string().c_str());
		if (err != vr::VRApplicationError_None) {
			output << "Error while adding manifest: " << vr::VRApplications()->GetApplicationsErrorNameFromEnum(err) << std::endl;
			outputStringStream(output);
			MessageBox(NULL, L"Manifest installation failed!", L"Leap Motion Overlay", MB_OK | MB_ICONERROR);
		} else {
			MessageBox(NULL, L"Application successfully registered!", L"Leap Motion Overlay", MB_OK | MB_ICONINFORMATION);
		}
	}
}

void OVROverlayController::removeManifest()
{
	std::stringstream output;

	if (isManifestInstalled()) {
		output << "Removing manifest" << std::endl;
		outputStringStream(output);

		//vr::EVRApplicationProperpty

		char manifestPathBase[512] = { 0 };
		uint32_t reqSize = vr::VRApplications()->GetApplicationPropertyString(APPLICATION_KEY, vr::VRApplicationProperty_WorkingDirectory_String, manifestPathBase, 512, nullptr);

		std::string manifestPathBaseStr(manifestPathBase);
		std::filesystem::path manifest_path = std::filesystem::path(manifestPathBaseStr) / "manifest.vrmanifest";

		vr::EVRApplicationError err = vr::VRApplications()->RemoveApplicationManifest(manifest_path.string().c_str());
		if (err != vr::VRApplicationError_None) {
			output << "Error while removing manifest: " << vr::VRApplications()->GetApplicationsErrorNameFromEnum(err) << std::endl;
			outputStringStream(output);
			MessageBox(NULL, L"Manifest removal failed!", L"Leap Motion Overlay", MB_OK | MB_ICONERROR);
		} else {
			MessageBox(NULL, L"Application successfully unregistered!", L"Leap Motion Overlay", MB_OK | MB_ICONINFORMATION);
		}
	}
}

bool OVROverlayController::isManifestInstalled()
{
	return vr::VRApplications()->IsApplicationInstalled(APPLICATION_KEY);
}

bool OVROverlayController::BHMDAvailable()
{
	return vr::VRSystem() != NULL;
}

vr::HmdError OVROverlayController::getLastHmdError()
{
	return m_eLastHmdError;
}

bool OVROverlayController::connectToVRRuntime()
{
	m_eLastHmdError = vr::VRInitError_None;
	m_VRSystem = vr::VR_Init(&m_eLastHmdError, vr::VRApplication_Overlay);

	if (m_eLastHmdError != vr::VRInitError_None) {
		m_strVRDriver = "No Driver";
		m_strVRDisplay = "No Display";
		return false;
	}

	//m_strVRDriver = GetTrackedDeviceString(pVRSystem, vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_TrackingSystemName_String);
	//m_strVRDisplay = GetTrackedDeviceString(pVRSystem, vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_SerialNumber_String);

	m_connected = true;

	return true;
}

void OVROverlayController::disconnectFromVRRuntime()
{
	vr::VR_Shutdown();
	m_connected = false;
}

void OVROverlayController::updateOverlaySizeAndPosition()
{
	std::stringstream output;

	vr::VROverlayError err = vr::VROverlay()->SetOverlayWidthInMeters(m_ulOverlayHandle, m_overlayWidth);
	if (err != vr::VROverlayError_None) {
		output << "SetOverlayWidthInMeters error: " << err << std::endl;
		outputStringStream(output);
	}

	vr::HmdMatrix34_t position = createOverlayMatrix(m_overlayZDistance);
	err = vr::VROverlay()->SetOverlayTransformTrackedDeviceRelative(m_ulOverlayHandle, 0, &position);

	if (err != vr::VROverlayError_None) {
		output << "SetOverlayTransformAbsolute error: " << err << std::endl;
		outputStringStream(output);
	}
}

vr::HmdMatrix34_t OVROverlayController::createOverlayMatrix(float zDistance)
{
	switch (m_overlayRotation) {
	case 1:
		return {
			0.0f, -1.0f, 0.0f, 0.0f,
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, -zDistance
		};
	case 2:
		return {
			-1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, -1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, -zDistance
		};
	case 3:
		return {
			0.0f, 1.0f, 0.0f, 0.0f,
			-1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, -zDistance
		};
	default:
		return {
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, -zDistance
		};
	}
}
