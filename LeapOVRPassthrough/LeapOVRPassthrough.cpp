#include "windows.h"
#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "LeapHandler.h"
#include "OVROverlayController.h"
#include "GraphicsManager.h"

GLuint display_fullscreenQuadVAO;
GLuint display_fullscreenQuadBuffer;
GLuint display_fullscreenQuadUVs;
GLuint display_vertexShader, display_fragmentShader;
GLuint display_shaderProgram;
GLuint display_textureSamplerID;

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
const GLint display_vertexShaderCodeLength = strlen(display_vertexShaderCode);

const char* display_fragmentShaderCode = R"""(
#version 420 core

layout(location = 0) out vec3 diffuseColor;

uniform sampler2D textureSampler;

in vec2 vUv;

void main() {
	diffuseColor = texture( textureSampler, vec2(vUv.x, 1 - vUv.y)).rgb;
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

void display_init() {
	GLint Result = GL_FALSE;
	int InfoLogLength;

	display_vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(display_vertexShader, 1, &display_vertexShaderCode, &display_vertexShaderCodeLength);
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

void display_display() {
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

int APIENTRY WinMain(HINSTANCE /*hInstance*/,
	HINSTANCE /*hPrevInstance*/,
	LPSTR /*lpCmdLine*/,
	int /*cmdShow*/)
{
	if (!glfwInit()) {
		return 1;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	//glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
	GLFWwindow* window = glfwCreateWindow(640, 480, "Test Window", NULL, NULL);
	glfwMakeContextCurrent(window);

	GLenum err = glewInit();
	if (err != GLEW_OK) {
		std::cerr << "glewInit Error: " << glewGetErrorString(err) << std::endl;
		return 1;
	}

	std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;

	OVROverlayController* vrController = OVROverlayController::getInstance();
	LeapHandler* leapHandler = LeapHandler::getInstance();
	GraphicsManager* graphicsManager = GraphicsManager::getInstance();

	if (!graphicsManager->init()) {
		std::cerr << "Graphics Manager initialization failed!" << std::endl;
		return 1;
	}

	glfwSwapInterval(1);

	vrController->init();
	leapHandler->openConnection();

	display_init();

	vrController->showOverlay();

	while (!glfwWindowShouldClose(window)) {
		graphicsManager->updateTexture();

		if (leapHandler->swipeDetected()) {
			vrController->toggleOverlay();
		}

		int width, height;

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glfwGetFramebufferSize(window, &width, &height);

		glViewport(0, 0, width, height);

		display_display();

		if (graphicsManager->wasUpdated()) {
			//std::cout << "update " << graphicsManager->getVideoTexture() << std::endl;
			vrController->setTexture(graphicsManager->getVideoTexture());
		}

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate();

	leapHandler->join();

    return 0;
}

int main(int argc, char** argv)
{
	return WinMain(GetModuleHandle(NULL), NULL, GetCommandLineA(), SW_SHOWNORMAL);
}