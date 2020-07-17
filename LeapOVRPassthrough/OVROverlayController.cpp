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
		vr::VROverlayError err = vr::VROverlay()->SetOverlayWidthInMeters(m_ulOverlayHandle, 0.5f);
		if (err != vr::VROverlayError_None) {
			std::cout << "SetOverlayWidthInMeters error: " << err << std::endl;
		}

		vr::HmdMatrix34_t position = {
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, -0.3f
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

void OVROverlayController::installManifest()
{
	if (!isManifestInstalled()) {
		std::cout << "Installing manifest" << std::endl;

		std::filesystem::path manifest_path = std::filesystem::current_path() / "manifest.vrmanifest";

		vr::EVRApplicationError err = vr::VRApplications()->AddApplicationManifest(manifest_path.string().c_str());
		if (err != vr::VRApplicationError_None) {
			std::cout << "Error while adding manifest: " << vr::VRApplications()->GetApplicationsErrorNameFromEnum(err) << std::endl;
			MessageBox(NULL, L"Manifest installation failed!", L"Leap Motion Overlay", MB_OK | MB_ICONERROR);
		} else {
			MessageBox(NULL, L"Application successfully registered!", L"Leap Motion Overlay", MB_OK | MB_ICONINFORMATION);
		}
	}
}

void OVROverlayController::removeManifest()
{
	if (isManifestInstalled()) {
		std::cout << "Removing manifest" << std::endl;

		//vr::EVRApplicationProperpty

		char manifestPathBase[512] = { 0 };
		uint32_t reqSize = vr::VRApplications()->GetApplicationPropertyString(APPLICATION_KEY, vr::VRApplicationProperty_WorkingDirectory_String, manifestPathBase, 512, nullptr);

		std::string manifestPathBaseStr(manifestPathBase);
		std::filesystem::path manifest_path = std::filesystem::path(manifestPathBaseStr) / "manifest.vrmanifest";

		vr::EVRApplicationError err = vr::VRApplications()->RemoveApplicationManifest(manifest_path.string().c_str());
		if (err != vr::VRApplicationError_None) {
			std::cout << "Error while removing manifest: " << vr::VRApplications()->GetApplicationsErrorNameFromEnum(err) << std::endl;
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
