#pragma once
#include <GL/glew.h>
#include <iostream>

class GraphicsManager
{
public:
	static GraphicsManager* getInstance();

	GraphicsManager();

	void init();
	void setFrame();


private:
	GLuint m_framebuffer;
};

