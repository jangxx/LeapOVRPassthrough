#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <openvr.h>
#include <vector>

GLuint fullscreenQuadVAO;
GLuint fullscreenQuadBuffer;
GLuint fullscreenQuadUVs;
GLuint vertexShader, fragmentShader;
GLuint shaderProgram;
GLuint textureSamplerID;
GLuint testTex;
vr::VROverlayHandle_t overlayHandle;

const char* vertexShaderCode = R"""(
#version 420 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 uv;

out vec2 vUv;

void main() {
	vUv = uv;
	gl_Position = vec4(position, 1.0);
}
)""";
const GLint vertexShaderCodeLength = strlen(vertexShaderCode);

const char* fragmentShaderCode = R"""(
#version 420 core

layout(location = 0) out vec3 diffuseColor;

uniform sampler2D textureSampler;

in vec2 vUv;

void main() {
	diffuseColor = texture( textureSampler, vec2(vUv.x, vUv.y)).rgb;
}
)""";
const GLint fragmentShaderCodeLength = strlen(fragmentShaderCode);

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

void init() {
	GLint Result = GL_FALSE;
	int InfoLogLength;

	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderCode, &vertexShaderCodeLength);
	glCompileShader(vertexShader);

	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(vertexShader, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
		printf("%s\n", &VertexShaderErrorMessage[0]);
	}

	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderCode, &fragmentShaderCodeLength);
	glCompileShader(fragmentShader);

	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(fragmentShader, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
		printf("%s\n", &VertexShaderErrorMessage[0]);
	}

	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	// Check the program
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &Result);
	glGetProgramiv(shaderProgram, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
		glGetProgramInfoLog(shaderProgram, InfoLogLength, NULL, &ProgramErrorMessage[0]);
		printf("%s\n", &ProgramErrorMessage[0]);
	}

	textureSamplerID = glGetUniformLocation(shaderProgram, "textureSampler");

	glGenVertexArrays(1, &fullscreenQuadVAO);
	glBindVertexArray(fullscreenQuadVAO);

	glGenBuffers(1, &fullscreenQuadBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, fullscreenQuadBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_quad_vertex_buffer_data), g_quad_vertex_buffer_data, GL_STATIC_DRAW);

	glGenBuffers(1, &fullscreenQuadUVs);
	glBindBuffer(GL_ARRAY_BUFFER, fullscreenQuadUVs);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_quad_uvs_buffer_data), g_quad_uvs_buffer_data, GL_STATIC_DRAW);
}

void initVR() {
	bool success = true;

	vr::HmdError err = vr::VRInitError_None;
	vr::IVRSystem *pVRSystem = vr::VR_Init(&err, vr::VRApplication_Overlay);

	success &= vr::VRCompositor() != NULL;

	if (vr::VROverlay()) {
		std::string name = "Overlay Test";
		std::string key = "de.literalchaos.minimal_overlay_test";
		vr::VROverlayError overlayError = vr::VROverlay()->CreateOverlay(key.c_str(), name.c_str(), &overlayHandle);

		success &= overlayError == vr::VROverlayError_None;
	}

	if (success) {
		vr::VROverlayError err = vr::VROverlay()->SetOverlayWidthInMeters(overlayHandle, 1.5f);
		if (err != vr::VROverlayError_None) {
			std::cout << "SetOverlayWidthInMeters error: " << err << std::endl;
		}

		vr::HmdMatrix34_t position = {
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, -1.0f
		};

		err = vr::VROverlay()->SetOverlayTransformTrackedDeviceRelative(overlayHandle, 0, &position);

		if (err != vr::VROverlayError_None) {
			std::cout << "SetOverlayTransformAbsolute error: " << err << std::endl;
		}

		std::cout << "Successfully created overlay" << std::endl;
	} else {
		std::cout << "Error creating overlay" << std::endl;
	}
}

void display() {
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(shaderProgram);

	glActiveTexture(GL_TEXTURE0);
	glUniform1i(textureSamplerID, 0);
	glBindTexture(GL_TEXTURE_2D, testTex);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, fullscreenQuadBuffer);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, fullscreenQuadUVs);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glDrawArrays(GL_TRIANGLES, 0, 6);
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);

	glEnd();
	glFlush();
}

GLuint createTestTexture(int width = 512, int height = 512) {
	uint8_t* pixelData = (uint8_t*)malloc(width * height * 4);

	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			int index = (y * width + x) * 4;
			pixelData[index] = x & 0xFF;
			pixelData[index + 1] = 100;
			pixelData[index + 2] = 0;
			pixelData[index + 3] = 255;
		}
	}

	GLuint resultTexture;
	glGenTextures(1, &resultTexture);
	glBindTexture(GL_TEXTURE_2D, resultTexture);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixelData);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	return resultTexture;
}

static void error_callback(int error, const char* description)
{
	fprintf(stderr, "Error: %s\n", description);
}

int main()
{
	if (!glfwInit()) {
		return 1;
	}

	initVR();

	glfwSetErrorCallback(error_callback);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	GLFWwindow* window = glfwCreateWindow(640, 480, "Test Window", NULL, NULL);
	glfwMakeContextCurrent(window);

	GLenum err = glewInit();

	if (err != GLEW_OK) {
		std::cerr << "glewInit Error: " << glewGetErrorString(err) << std::endl;
		return 1;
	}

	std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;

	glfwSwapInterval(1);

	init();

	testTex = createTestTexture(512, 512);
	GLuint testTex2 = createTestTexture(512, 512);

	std::cout << "tex id: " << testTex << std::endl;

	glFinish();

	vr::Texture_t texture;
	texture.handle = (void*)(uintptr_t)testTex;
	texture.eType = vr::TextureType_OpenGL;
	texture.eColorSpace = vr::ColorSpace_Linear;

	vr::VROverlayError vrErr = vr::VROverlay()->SetOverlayTexture(overlayHandle, &texture);

	if (vrErr != vr::VROverlayError_None) {
		std::cout << "setTexture error: " << vrErr << std::endl;
	}

	vrErr = vr::VROverlay()->ShowOverlay(overlayHandle);

	if (vrErr != vr::VROverlayError_None) {
		std::cout << "showOverlay error: " << vrErr << std::endl;
	}

	while (!glfwWindowShouldClose(window)) {
		int width, height;

		glfwGetFramebufferSize(window, &width, &height);

		glViewport(0, 0, width, height);

		display();

		glfwSwapBuffers(window);
		glfwPollEvents();

	}

	glfwDestroyWindow(window);

	glfwTerminate();

	return 0;
}