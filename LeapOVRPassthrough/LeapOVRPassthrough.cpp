#include "windows.h"
#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "LeapHandler.h"
#include "OVROverlayController.h"
#include "GraphicsManager.h"

GLuint dbg_fullscreenQuadVAO;
GLuint dbg_fullscreenQuadBuffer;
GLuint dbg_fullscreenQuadUVs;
GLuint dbg_vertexShader, dbg_fragmentShader;
GLuint dbg_shaderProgram;
GLuint dbg_textureSamplerID;

const char* dbg_vertexShaderCode = R"""(
#version 420 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 uv;

out vec2 vUv;

void main() {
	vUv = uv;
	gl_Position = vec4(position, 1.0);
}
)""";
const GLint dbg_vertexShaderCodeLength = strlen(dbg_vertexShaderCode);

const char* dbg_fragmentShaderCode = R"""(
#version 420 core

layout(location = 0) out vec3 diffuseColor;

uniform sampler2D textureSampler;

in vec2 vUv;

void main() {
	//diffuseColor = vec3(vUv.x, vUv.y, 0.0);
	diffuseColor = texture( textureSampler, vec2(vUv.x, 1 - vUv.y)).rgb;
}
)""";
const GLint dbg_fragmentShaderCodeLength = strlen(dbg_fragmentShaderCode);

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

GLuint testTex, testTex2;

void dbg_init() {
	GLint Result = GL_FALSE;
	int InfoLogLength;

	dbg_vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(dbg_vertexShader, 1, &dbg_vertexShaderCode, &dbg_vertexShaderCodeLength);
	glCompileShader(dbg_vertexShader);

	glGetShaderiv(dbg_vertexShader, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(dbg_vertexShader, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(dbg_vertexShader, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
		printf("%s\n", &VertexShaderErrorMessage[0]);
	}

	dbg_fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(dbg_fragmentShader, 1, &dbg_fragmentShaderCode, &dbg_fragmentShaderCodeLength);
	glCompileShader(dbg_fragmentShader);

	glGetShaderiv(dbg_fragmentShader, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(dbg_fragmentShader, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(dbg_fragmentShader, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
		printf("%s\n", &VertexShaderErrorMessage[0]);
	}

	dbg_shaderProgram = glCreateProgram();
	glAttachShader(dbg_shaderProgram, dbg_vertexShader);
	glAttachShader(dbg_shaderProgram, dbg_fragmentShader);
	glLinkProgram(dbg_shaderProgram);

	// Check the program
	glGetProgramiv(dbg_shaderProgram, GL_LINK_STATUS, &Result);
	glGetProgramiv(dbg_shaderProgram, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
		glGetProgramInfoLog(dbg_shaderProgram, InfoLogLength, NULL, &ProgramErrorMessage[0]);
		printf("%s\n", &ProgramErrorMessage[0]);
	}

	dbg_textureSamplerID = glGetUniformLocation(dbg_shaderProgram, "textureSampler");

	glGenVertexArrays(1, &dbg_fullscreenQuadVAO);
	glBindVertexArray(dbg_fullscreenQuadVAO);

	glGenBuffers(1, &dbg_fullscreenQuadBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, dbg_fullscreenQuadBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_quad_vertex_buffer_data), g_quad_vertex_buffer_data, GL_STATIC_DRAW);

	glGenBuffers(1, &dbg_fullscreenQuadUVs);
	glBindBuffer(GL_ARRAY_BUFFER, dbg_fullscreenQuadUVs);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_quad_uvs_buffer_data), g_quad_uvs_buffer_data, GL_STATIC_DRAW);
}

void dbg_display() {
	GraphicsManager* graphicsManager = GraphicsManager::getInstance();

	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(dbg_shaderProgram);

	//glActiveTexture(GL_TEXTURE0);
	glUniform1i(dbg_textureSamplerID, 0);
	glBindTexture(GL_TEXTURE_2D, graphicsManager->getVideoTexture());
	//glBindTexture(GL_TEXTURE_2D, testTex);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, dbg_fullscreenQuadBuffer);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, dbg_fullscreenQuadUVs);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glDrawArrays(GL_TRIANGLES, 0, 6);
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);

	/*glColor3f(1.0, 0.0, 0.0);

	glBegin(GL_POINTS);
	glVertex2f(10.0, 10.0);
	glVertex2f(150.0, 80.0);
	glVertex2f(100.0, 20.0);
	glVertex2f(200.0, 100.0);*/
	glEnd();
	glFlush();
}

GLuint createTestTexture(int width = 512, int height = 512) {
	const int PIXEL_BYTES = 1;

	uint8_t* pixelData = (uint8_t*)malloc(width * height * PIXEL_BYTES);

	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			int index = (y * width + x) * PIXEL_BYTES;
			pixelData[index] = x & 0xFF;
			/*pixelData[index + 1] = 0;
			pixelData[index + 2] = 0;
			pixelData[index + 3] = 200;*/
		}
	}

	GLuint resultTexture;
	glGenTextures(1, &resultTexture);
	glBindTexture(GL_TEXTURE_2D, resultTexture);

	GLint swizzleMask[] = { GL_RED, GL_RED, GL_RED, GL_RED };
	glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, pixelData);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	return resultTexture;
}

//void dgb_redrawTimer(int value) {
//	glutPostRedisplay();
//	glutTimerFunc(1000 / 30, dgb_redrawTimer, value + 1);
//}

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

	//testTex = createTestTexture(1000, 500);

	dbg_init();

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

		dbg_display();

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
	//glutInit(&argc, argv);

	return WinMain(GetModuleHandle(NULL), NULL, GetCommandLineA(), SW_SHOWNORMAL);
}