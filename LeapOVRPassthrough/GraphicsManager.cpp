#include "GraphicsManager.h"

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

layout(location = 0) out vec4 diffuseColor;

uniform sampler2D textureSampler;
uniform sampler2D distortionTextureSampler;
uniform bool useDistortionMap; 

in vec2 vUv;

void main() {
	if (useDistortionMap) {
		vec2 uv = vUv;
		vec2 distortionIndex = texture2D(distortionTextureSampler, uv).xy;

		float hIndex = distortionIndex.x;
		float vIndex = distortionIndex.y;

		if (vIndex > 0.0 && vIndex < 1.0 && hIndex > 0.0 && hIndex < 1.0) {
			diffuseColor = vec4(texture(textureSampler, distortionIndex).rrr, 1);
		} else {
			diffuseColor = vec4(0.2, 0.0, 0.0, 0.0);
		}
	} else {
		vec2 uv = vec2(vUv.x, 1 - vUv.y);
		diffuseColor = vec4(texture( textureSampler, uv).rrr, 1);
	}
}
)""";
const GLint fragmentShaderCodeLength = strlen(fragmentShaderCode);

static const GLfloat fullscreenQuadGeo[] = {
	-1.0f, -1.0f, 0.0f,
	1.0f, -1.0f, 0.0f,
	-1.0f,  1.0f, 0.0f,
	-1.0f,  1.0f, 0.0f,
	1.0f,  1.0f, 0.0f,
	1.0f, -1.0f, 0.0f,
};

static const GLfloat fullscreenQuadUVs[] = {
	0.0f, 0.0f,
	1.0f, 0.0f,
	0.0f,  1.0f,
	0.0f,  1.0f,
	1.0f,  1.0f,
	1.0f, 0.0f,
};

GraphicsManager* s_sharedInstance = nullptr;

bool checkShader(GLuint id) {
	GLint result = GL_FALSE;
	int infoLogLength;

	glGetShaderiv(id, GL_COMPILE_STATUS, &result);
	glGetShaderiv(id, GL_INFO_LOG_LENGTH, &infoLogLength);
	if (infoLogLength > 0) {
		std::vector<char> shaderErrorMessage(infoLogLength + 1);
		std::vector<wchar_t> shaderErrorMessageW(infoLogLength + 1);

		glGetShaderInfoLog(id, infoLogLength, NULL, &shaderErrorMessage[0]);

		size_t convertedChars = 0;
		mbstowcs_s(&convertedChars, &shaderErrorMessageW[0], shaderErrorMessageW.size(), &shaderErrorMessage[0], shaderErrorMessage.size() - 1);
		OutputDebugString(&shaderErrorMessageW[0]);
	}

	return infoLogLength == 0;
}

bool checkProgram(GLuint id) {
	GLint result = GL_FALSE;
	int infoLogLength;

	glGetProgramiv(id, GL_COMPILE_STATUS, &result);
	glGetProgramiv(id, GL_INFO_LOG_LENGTH, &infoLogLength);
	if (infoLogLength > 0) {
		std::vector<char> shaderErrorMessage(infoLogLength + 1);
		std::vector<wchar_t> shaderErrorMessageW(infoLogLength + 1);

		glGetShaderInfoLog(id, infoLogLength, NULL, &shaderErrorMessage[0]);

		size_t convertedChars = 0;
		mbstowcs_s(&convertedChars, &shaderErrorMessageW[0], shaderErrorMessageW.size(), &shaderErrorMessage[0], shaderErrorMessage.size() - 1);
		OutputDebugString(&shaderErrorMessageW[0]);
	}

	return infoLogLength == 0;
}


GraphicsManager * GraphicsManager::getInstance()
{
	if (s_sharedInstance == nullptr) {
		s_sharedInstance = new GraphicsManager();
	}

	return s_sharedInstance;
}

GraphicsManager::GraphicsManager()
{
	m_pixelData = (uint8_t*)malloc(m_width * m_height * sizeof(uint8_t));
	m_distortionPixelData = (float*)malloc(LEAP_DISTORTION_MATRIX_N * LEAP_DISTORTION_MATRIX_N * 2 * sizeof(float));
}

GraphicsManager::~GraphicsManager()
{
	free(m_pixelData);
	free(m_distortionPixelData);
}

bool GraphicsManager::init()
{
	// create and compile vertex shader
	m_vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(m_vertexShader, 1, &vertexShaderCode, &vertexShaderCodeLength);
	glCompileShader(m_vertexShader);

	if (!checkShader(m_vertexShader)) return false;

	// create and compile fragment shader
	m_fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(m_fragmentShader, 1, &fragmentShaderCode, &fragmentShaderCodeLength);
	glCompileShader(m_fragmentShader);

	if (!checkShader(m_fragmentShader)) return false;

	// turn vertex and fragment shader into a program
	m_shaderProgram = glCreateProgram();
	glAttachShader(m_shaderProgram, m_vertexShader);
	glAttachShader(m_shaderProgram, m_fragmentShader);
	glLinkProgram(m_shaderProgram);

	if (!checkProgram(m_shaderProgram)) return false;

	m_textureSamplerID = glGetUniformLocation(m_shaderProgram, "textureSampler");
	m_distortionTextureSamplerID = glGetUniformLocation(m_shaderProgram, "distortionTextureSampler");
	m_useDistortionMapID = glGetUniformLocation(m_shaderProgram, "useDistortionMap");

	glGenVertexArrays(1, &m_fullscreenQuadVAO);
	glBindVertexArray(m_fullscreenQuadVAO);

	glGenBuffers(1, &m_fullscreenQuadBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, m_fullscreenQuadBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(fullscreenQuadGeo), fullscreenQuadGeo, GL_STATIC_DRAW);

	glGenBuffers(1, &m_fullscreenQuadUVs);
	glBindBuffer(GL_ARRAY_BUFFER, m_fullscreenQuadUVs);
	glBufferData(GL_ARRAY_BUFFER, sizeof(fullscreenQuadUVs), fullscreenQuadUVs, GL_STATIC_DRAW);

	// create framebuffer
	glGenFramebuffers(1, &m_framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);

	glGenTextures(1, &m_framebufferTexture);
	glBindTexture(GL_TEXTURE_2D, m_framebufferTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_fbWidth, m_fbHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_framebufferTexture, 0);
	GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, DrawBuffers);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		return false;
	}

	// generate a dummy initial video texture
	for (int i = 0; i < m_width * m_height; i++) {
		m_pixelData[i] = 255;
	}
	for (int i = 0; i < LEAP_DISTORTION_MATRIX_N * LEAP_DISTORTION_MATRIX_N * 2; i++) {
		m_distortionPixelData[i] = 0.5f;
	}

	glGenTextures(1, &m_videoTexture);
	glBindTexture(GL_TEXTURE_2D, m_videoTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, m_width, m_height, 0, GL_RED, GL_UNSIGNED_BYTE, m_pixelData);

	glGenTextures(1, &m_distortionTexture);
	glBindTexture(GL_TEXTURE_2D, m_distortionTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG, LEAP_DISTORTION_MATRIX_N, LEAP_DISTORTION_MATRIX_N, 0, GL_RG, GL_FLOAT, m_distortionPixelData);

	updateFramebuffer();

	return true;
}

void GraphicsManager::updateTexture()
{
	std::lock_guard<std::mutex> lock(m_updateMutex);

	if (!m_frameChanged) {
		return;
	}

	if (m_dimensionsChanged) { // re-gen texture
		//glDeleteTextures(1, &m_videoTexture);
		//glGenTextures(1, &m_videoTexture);

		//glBindTexture(GL_TEXTURE_2D, m_videoTexture);

		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glBindTexture(GL_TEXTURE_2D, m_videoTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, m_width, m_height, 0, GL_RED, GL_UNSIGNED_BYTE, m_pixelData);

		glBindTexture(GL_TEXTURE_2D, m_framebufferTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_fbWidth, m_fbHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	} else {
		glBindTexture(GL_TEXTURE_2D, m_videoTexture);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_width, m_height, GL_RED, GL_UNSIGNED_BYTE, m_pixelData);
	}

	if (m_distortionMapChanged) {
		glBindTexture(GL_TEXTURE_2D, m_distortionTexture);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, LEAP_DISTORTION_MATRIX_N, LEAP_DISTORTION_MATRIX_N, GL_RG, GL_FLOAT, m_distortionPixelData);
	}

	m_wasUpdated = true;
	m_dimensionsChanged = false;
	m_distortionMapChanged = false;
	m_frameChanged = false;

	updateFramebuffer();
}

void GraphicsManager::setFrame(int width, int height, uint8_t* data)
{
	std::lock_guard<std::mutex> lock(m_updateMutex);
	//std::cout << "width: " << width << " height: " << height << std::endl;

	if (m_width != width || m_height != height) {
		m_dimensionsChanged = true;
	}

	m_frameChanged = true;
	m_width = width;
	m_height = height;

	if (m_dimensionsChanged) {
		if (m_pixelData != nullptr) {
			free(m_pixelData);
		}

		m_pixelData = (uint8_t*)malloc(m_width * m_height);
	}

	assert(m_pixelData != nullptr); 

	memcpy(m_pixelData, data, m_width * m_height);
}

void GraphicsManager::setDistortionMap(float* data)
{
	std::lock_guard<std::mutex> lock(m_updateMutex);

	m_distortionMapChanged = true;

	OutputDebugString(L"Distortion map changed\n");

	memcpy(m_distortionPixelData, data, LEAP_DISTORTION_MATRIX_N * LEAP_DISTORTION_MATRIX_N * 2 * sizeof(float));
}

void GraphicsManager::setDistortionMapActive(bool active)
{
	std::lock_guard<std::mutex> lock(m_updateMutex);

	m_useDistortionMap = active;
}

bool GraphicsManager::getDistortionMapActive()
{
	std::lock_guard<std::mutex> lock(m_updateMutex);

	return m_useDistortionMap;
}

GLuint GraphicsManager::getVideoTexture()
{
	return m_framebufferTexture;
}

bool GraphicsManager::wasUpdated()
{
	std::lock_guard<std::mutex> lock(m_updateMutex);
	return m_wasUpdated;
}

void GraphicsManager::updateFramebuffer()
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);
	glViewport(0, 0, m_fbWidth, m_fbHeight);

	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(m_shaderProgram);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_videoTexture);
	glUniform1i(m_textureSamplerID, 0); // set the sampler to use texture unit 0
	glBindTexture(GL_TEXTURE0, m_videoTexture);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_distortionTexture);
	glUniform1i(m_distortionTextureSamplerID, 1); // set the distortion sampler to use texture unit 0
	glBindTexture(GL_TEXTURE1, m_distortionTexture);

	glUniform1i(m_useDistortionMapID, m_useDistortionMap);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, m_fullscreenQuadBuffer);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, m_fullscreenQuadUVs);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glDrawArrays(GL_TRIANGLES, 0, 6);
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);

	glEnd();
	glFlush();
}

void GraphicsManager::setFramebufferSize(int width, int height)
{
	std::lock_guard<std::mutex> lock(m_updateMutex);

	m_fbWidth = width;
	m_fbHeight = height;

	m_dimensionsChanged = true;
}
