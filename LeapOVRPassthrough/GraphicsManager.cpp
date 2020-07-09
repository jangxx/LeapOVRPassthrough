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
}

void GraphicsManager::init()
{
	GLenum err = glewInit();

	if (err != GLEW_OK) {
		std::cerr << "glewInit Error: " << glewGetErrorString(err) << std::endl;
		return;
	}

	/*GLuint framebuffer = 0;

	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);*/
}

void GraphicsManager::setFrame()
{
}
