#pragma once
#include <GL/glew.h>
//#include <GL/freeglut.h>
#include <iostream>
#include <mutex>

#include "OVROverlayController.h"

class GraphicsManager
{
public:
	static GraphicsManager* getInstance();

	GraphicsManager();
	~GraphicsManager();

	void init();
	void updateTexture();
	void setFrame(int width, int height, uint8_t* left_data, uint8_t* right_data);
	GLuint getVideoTexture();

private:
	GLuint m_videoTexture;

	uint8_t* m_pixelData;
	int m_width{ 128 };
	int m_height{ 128 };

	std::mutex m_updateMutex;
};

