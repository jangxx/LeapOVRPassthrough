#define GLFW_EXPOSE_NATIVE_WIN32

#include "windows.h"
#include "resource.h"
#include <CommCtrl.h>
#include <iostream>
#include <vector>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <lodepng.h>

#include "LeapHandler.h"
#include "OVROverlayController.h"
#include "GraphicsManager.h"

#define TRAYMENU_EXIT 1
#define TRAYMENU_SHOW 2
#define TRAYMENU_INSTALL_MANIFEST 3
#define TRAYMENU_REMOVE_MANIFEST 4

GLuint display_fullscreenQuadVAO;
GLuint display_fullscreenQuadBuffer;
GLuint display_fullscreenQuadUVs;
GLuint display_vertexShader, display_fragmentShader;
GLuint display_shaderProgram;
GLuint display_textureSamplerID;
WNDPROC defaultWndProc;
GLFWwindow* globalWindow;
bool globalKeepRunning = true;

const char* display_vertexShaderCode = R"""(
#version 420 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 uv;

out vec2 vUv;

void main() {
	vUv = uv;
	gl_Position = vec4(position, 1.0);
}
)""";
const size_t display_vertexShaderCodeLength = strlen(display_vertexShaderCode);

const char* display_fragmentShaderCode = R"""(
#version 420 core

layout(location = 0) out vec3 diffuseColor;

uniform sampler2D textureSampler;

in vec2 vUv;

void main() {
	diffuseColor = texture( textureSampler, vec2(vUv.x, vUv.y)).rgb;
}
)""";
const GLint display_fragmentShaderCodeLength = strlen(display_fragmentShaderCode);

static const GLfloat g_quad_vertex_buffer_data[] = {
	-1.0f, -1.0f, 0.0f,
	1.0f, -1.0f, 0.0f,
	-1.0f,  1.0f, 0.0f,
	-1.0f,  1.0f, 0.0f,
	1.0f,  1.0f, 0.0f,
	1.0f, -1.0f, 0.0f,
};

static const GLfloat g_quad_uvs_buffer_data[] = {
	0.0f, 0.0f,
	1.0f, 0.0f,
	0.0f,  1.0f,
	0.0f,  1.0f,
	1.0f,  1.0f,
	1.0f, 0.0f,
};

GLFWimage loadResource(HINSTANCE hInstance, int id) {
	GLFWimage result = { 0 };

	HRSRC hResource = FindResource(hInstance, MAKEINTRESOURCE(id), L"PNG");
	DWORD imageSize = SizeofResource(hInstance, hResource);

	const void* data = LockResource(LoadResource(hInstance, hResource));

	std::vector<unsigned char> pngfile((unsigned char*)data, (unsigned char*)data + imageSize);

	std::vector<uint8_t> pixels;
	unsigned width, height;

	lodepng::decode(pixels, width, height, pngfile);

	std::vector<unsigned char>* pixelData = new std::vector<unsigned char>(pixels);

	result.width = width;
	result.height = height;
	result.pixels = pixelData->data();

	return result;
}

void display_init() {
	GLint Result = GL_FALSE;
	int InfoLogLength;

	display_vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(display_vertexShader, 1, &display_vertexShaderCode, (GLint*)&display_vertexShaderCodeLength);
	glCompileShader(display_vertexShader);

	glGetShaderiv(display_vertexShader, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(display_vertexShader, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(display_vertexShader, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
		printf("%s\n", &VertexShaderErrorMessage[0]);
	}

	display_fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(display_fragmentShader, 1, &display_fragmentShaderCode, &display_fragmentShaderCodeLength);
	glCompileShader(display_fragmentShader);

	glGetShaderiv(display_fragmentShader, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(display_fragmentShader, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(display_fragmentShader, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
		printf("%s\n", &VertexShaderErrorMessage[0]);
	}

	display_shaderProgram = glCreateProgram();
	glAttachShader(display_shaderProgram, display_vertexShader);
	glAttachShader(display_shaderProgram, display_fragmentShader);
	glLinkProgram(display_shaderProgram);

	// Check the program
	glGetProgramiv(display_shaderProgram, GL_LINK_STATUS, &Result);
	glGetProgramiv(display_shaderProgram, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
		glGetProgramInfoLog(display_shaderProgram, InfoLogLength, NULL, &ProgramErrorMessage[0]);
		printf("%s\n", &ProgramErrorMessage[0]);
	}

	display_textureSamplerID = glGetUniformLocation(display_shaderProgram, "textureSampler");

	glGenVertexArrays(1, &display_fullscreenQuadVAO);
	glBindVertexArray(display_fullscreenQuadVAO);

	glGenBuffers(1, &display_fullscreenQuadBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, display_fullscreenQuadBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_quad_vertex_buffer_data), g_quad_vertex_buffer_data, GL_STATIC_DRAW);

	glGenBuffers(1, &display_fullscreenQuadUVs);
	glBindBuffer(GL_ARRAY_BUFFER, display_fullscreenQuadUVs);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_quad_uvs_buffer_data), g_quad_uvs_buffer_data, GL_STATIC_DRAW);
}

void display_render() {
	GraphicsManager* graphicsManager = GraphicsManager::getInstance();

	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(display_shaderProgram);

	glUniform1i(display_textureSamplerID, 0);
	glBindTexture(GL_TEXTURE_2D, graphicsManager->getVideoTexture());

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, display_fullscreenQuadBuffer);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, display_fullscreenQuadUVs);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glDrawArrays(GL_TRIANGLES, 0, 6);
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);

	glEnd();
	glFlush();
}

void addTrayIcon(HINSTANCE hInstance, HWND hWnd, UINT uID) {
	NOTIFYICONDATA nid;
	nid.hWnd = hWnd;
	nid.uID = uID;
	nid.uFlags = NIF_TIP | NIF_MESSAGE | NIF_ICON;
	nid.uCallbackMessage = WM_APP;
	nid.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
	wcscpy_s(nid.szTip, L"Leap Motion SteamVR Overlay");
	
	Shell_NotifyIcon(NIM_ADD, &nid);
}

void removeTrayIcon(HWND hWnd, UINT uID) {
	NOTIFYICONDATA nid;
	nid.hWnd = hWnd;
	nid.uID = uID;

	Shell_NotifyIcon(NIM_DELETE, &nid);
}

void showTrayMenu(HWND hWnd, POINT *curpos, int wDefaultItem) {
	OVROverlayController* vrController = OVROverlayController::getInstance();

	HMENU hPopup = CreatePopupMenu();

	InsertMenu(hPopup, 0, MF_BYPOSITION | MF_STRING, TRAYMENU_SHOW, L"Show Window");

	if (vrController->isManifestInstalled()) {
		InsertMenu(hPopup, 1, MF_BYPOSITION | MF_STRING, TRAYMENU_REMOVE_MANIFEST, L"Unregister from SteamVR");
	} else {
		InsertMenu(hPopup, 1, MF_BYPOSITION | MF_STRING, TRAYMENU_INSTALL_MANIFEST, L"Register with SteamVR");
	}

	InsertMenu(hPopup, 2, MF_BYPOSITION | MF_STRING, TRAYMENU_EXIT, L"Exit");

	POINT pt;
	if (!curpos) {
		GetCursorPos(&pt);
		curpos = &pt;
	}

	WORD cmd = TrackPopupMenu(hPopup, TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD | TPM_NONOTIFY, curpos->x, curpos->y, 0, hWnd, NULL);
	SendMessage(hWnd, WM_COMMAND, cmd, 0);

	DestroyMenu(hPopup);
}

static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData) {
	OVROverlayController* vrController = OVROverlayController::getInstance();

	switch (uMsg) {
		case WM_APP:
			switch (lParam) {
				case WM_LBUTTONDBLCLK:
					glfwShowWindow(globalWindow);
					return 0;
				case WM_RBUTTONUP:
					SetForegroundWindow(hWnd);
					showTrayMenu(hWnd, NULL, -1);
					return 0;
			}
			return 0;
		case WM_COMMAND:
			switch (wParam) {
				case TRAYMENU_EXIT:
					globalKeepRunning = false;
					return 0;
				case TRAYMENU_SHOW:
					glfwShowWindow(globalWindow);
					return 0;
				case TRAYMENU_INSTALL_MANIFEST:
					vrController->installManifest();
					return 0;
				case TRAYMENU_REMOVE_MANIFEST:
					vrController->removeManifest();
					return 0;
			}
			return 0;
		case WM_NCDESTROY:
			RemoveWindowSubclass(hWnd, WndProc, uIdSubclass);
			return DefSubclassProc(hWnd, uMsg, wParam, lParam);
	}

	return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

int APIENTRY WinMain(HINSTANCE hInstance,
	HINSTANCE /*hPrevInstance*/,
	LPSTR /*lpCmdLine*/,
	int /*cmdShow*/)
{
	if (!glfwInit()) {
		MessageBox(NULL, L"GLFW failed to initialize!", L"Leap Motion SteamVR Overlay", MB_OK | MB_ICONERROR);
		std::cerr << "GLFW failed to initialize!" << std::endl;
		return 1;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
	globalWindow = glfwCreateWindow(640, 480, "Leap Motion SteamVR Overlay", NULL, NULL);
	HWND windowHandle = glfwGetWin32Window(globalWindow);

	//defaultWndProc = (WNDPROC)GetWindowLongPtr(windowHandle, GWLP_WNDPROC);
	//SetWindowLongPtr(windowHandle, GWLP_WNDPROC, (long)WndProc);

	SetWindowSubclass(windowHandle, WndProc, 1, 0);

	glfwMakeContextCurrent(globalWindow);
	glfwSwapInterval(1);

	GLenum err = glewInit();
	if (err != GLEW_OK) {
		MessageBox(NULL, L"GLEW failed to initialize!", L"Leap Motion SteamVR Overlay", MB_OK | MB_ICONERROR);
		std::cerr << "glewInit Error: " << glewGetErrorString(err) << std::endl;
		return 1;
	}

	std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;

	OVROverlayController* vrController = OVROverlayController::getInstance();
	LeapHandler* leapHandler = LeapHandler::getInstance();
	GraphicsManager* graphicsManager = GraphicsManager::getInstance();

	if (!graphicsManager->init()) {
		MessageBox(NULL, L"GraphicsManager failed to initialize!", L"Leap Motion SteamVR Overlay", MB_OK | MB_ICONERROR);
		std::cerr << "Graphics Manager initialization failed!" << std::endl;
		return 1;
	}

	addTrayIcon(hInstance, windowHandle, 1);

	// more window setup
	GLFWimage windowIcon = loadResource(hInstance, IDB_PNG1);
	glfwSetWindowIcon(globalWindow, 1, &windowIcon);

	vrController->init();
	leapHandler->openConnection();

	display_init();

	//vrController->showOverlay();

	glfwSetWindowCloseCallback(globalWindow, [](GLFWwindow* wnd) {
		glfwHideWindow(wnd);
	});

	while (globalKeepRunning && vrController->isConnected()) {
		graphicsManager->updateTexture();

		if (leapHandler->swipeDetected()) {
			vrController->toggleOverlay();
		}

		if (graphicsManager->wasUpdated()) {
			//std::cout << "update " << graphicsManager->getVideoTexture() << std::endl;
			vrController->setTexture(graphicsManager->getVideoTexture());
		}

		if (glfwGetWindowAttrib(globalWindow, GLFW_VISIBLE)) {
			int width, height;

			// bind output window as framebuffer
			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			glfwGetFramebufferSize(globalWindow, &width, &height);

			glViewport(0, 0, width, height);

			display_render();

			glfwSwapBuffers(globalWindow);

			glfwPollEvents();
		} else {
			glfwWaitEventsTimeout(1.0 / 60.0); // aim for roughly 60fps when no window is displayed
		}

		vrController->pollEvents();
	}

	removeTrayIcon(windowHandle, 1);

	glfwDestroyWindow(globalWindow);
	glfwTerminate();

	leapHandler->join();

    return 0;
}

int main(int argc, char** argv)
{
	return WinMain(GetModuleHandle(NULL), NULL, GetCommandLineA(), SW_SHOWNORMAL);
}