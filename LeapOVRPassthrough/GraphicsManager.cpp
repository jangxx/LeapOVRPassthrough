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

in vec2 vUv;

void main() {
	diffuseColor = vec4(texture( textureSampler, vec2(vUv.x, 1 - vUv.y)).rrr, 1);
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
		std::vector<char> VertexShaderErrorMessage(infoLogLength + 1);
		glGetShaderInfoLog(id, infoLogLength, NULL, &VertexShaderErrorMessage[0]);
		std::cout << &VertexShaderErrorMessage[0] << std::endl;
	}

	return infoLogLength == 0;
}

bool checkProgram(GLuint id) {
	GLint result = GL_FALSE;
	int infoLogLength;

	glGetProgramiv(id, GL_COMPILE_STATUS, &result);
	glGetProgramiv(id, GL_INFO_LOG_LENGTH, &infoLogLength);
	if (infoLogLength > 0) {
		std::vector<char> VertexShaderErrorMessage(infoLogLength + 1);
		glGetShaderInfoLog(id, infoLogLength, NULL, &VertexShaderErrorMessage[0]);
		std::cout << &VertexShaderErrorMessage[0] << std::endl;
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
	m_pixelData = (uint8_t*)malloc(m_width * m_height);
}

GraphicsManager::~GraphicsManager()
{
	free(m_pixelData);
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
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, FRAMEBUFFER_WIDTH, FRAMEBUFFER_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_framebufferTexture, 0);
	GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, DrawBuffers);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		return false;
	}

	// generate a dummy initial video texture
	m_pixelData = (uint8_t*)malloc(100 * 100);
	for (int i = 0; i < 100 * 100; i++) {
		m_pixelData[i] = 255;
	}

	glGenTextures(1, &m_videoTexture);
	glBindTexture(GL_TEXTURE_2D, m_videoTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 100, 100, 0, GL_RED, GL_UNSIGNED_BYTE, m_pixelData);

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
		glDeleteTextures(1, &m_videoTexture);
		glGenTextures(1, &m_videoTexture);

		glBindTexture(GL_TEXTURE_2D, m_videoTexture);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	} else {
		glBindTexture(GL_TEXTURE_2D, m_videoTexture);
	}

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, m_width, m_height, 0, GL_RED, GL_UNSIGNED_BYTE, m_pixelData);

	m_wasUpdated = true;
	m_dimensionsChanged = false;
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

	memcpy(m_pixelData, data, m_width * m_height);
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
	glViewport(0, 0, FRAMEBUFFER_WIDTH, FRAMEBUFFER_HEIGHT);

	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(m_shaderProgram);

	glActiveTexture(GL_TEXTURE0);
	glUniform1i(m_textureSamplerID, 0);
	glBindTexture(GL_TEXTURE0, m_videoTexture);

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
