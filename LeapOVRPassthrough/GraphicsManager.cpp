#include "stdafx.h"
#include "GraphicsManager.h"

GraphicsManager* s_sharedInstance = nullptr;

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

void GraphicsManager::init()
{
	GLenum err = glewInit();

	if (err != GLEW_OK) {
		std::cerr << "glewInit Error: " << glewGetErrorString(err) << std::endl;
		return;
	}

	for (int y = 0; y < m_height; y++) {
		for (int x = 0; x < m_width; x++) {
			int index = (y * m_width + x) * 1;
			m_pixelData[index] = 100;
			//m_pixelData[index + 1] = x & 0xFF;
			//m_pixelData[index + 2] = y & 0xFF;
		}
	}

	glGenTextures(1, &m_videoTexture);
	glBindTexture(GL_TEXTURE_2D, m_videoTexture);

	GLint swizzleMask[] = { GL_RED, GL_RED, GL_RED, GL_RED };
	glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, m_width, m_height, 0, GL_RED, GL_UNSIGNED_BYTE, m_pixelData);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

void GraphicsManager::updateTexture()
{
	std::lock_guard<std::mutex> lock(m_updateMutex);
	//std::cout << "update!" << std::endl;

	glBindTexture(GL_TEXTURE_2D, m_videoTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, m_width, m_height, 0, GL_RED, GL_UNSIGNED_BYTE, m_pixelData);
	glBindTexture(GL_TEXTURE_2D, 0);

	//OVROverlayController* vrController = OVROverlayController::getInstance();
	//vrController->setTexture(&m_videoTexture);
}

void GraphicsManager::setFrame(int width, int height, uint8_t* left_data, uint8_t* right_data)
{
	std::lock_guard<std::mutex> lock(m_updateMutex);
	//std::cout << "width: " << width << " height: " << height << std::endl;

	m_width = width;
	m_height = height;

	free(m_pixelData);
	m_pixelData = (uint8_t*)malloc(m_width * m_height);

	memcpy(m_pixelData, left_data, m_width * m_height);
}

GLuint GraphicsManager::getVideoTexture()
{
	return m_videoTexture;
}
