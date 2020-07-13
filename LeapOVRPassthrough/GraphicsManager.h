#pragma once
#include <GL/glew.h>
#include <iostream>
#include <mutex>

#include "OVROverlayController.h"

class GraphicsManager
{
public:
	static GraphicsManager* getInstance();

	GraphicsManager();
	~GraphicsManager();

	bool init();
	void updateTexture();
	void setFrame(int width, int height, uint8_t* data);
	GLuint getVideoTexture();
	bool wasUpdated();

private:
	void updateFramebuffer();

	GLuint m_videoTexture { 0 };

	uint8_t* m_pixelData { nullptr };
	int m_width { 0 };
	int m_height { 0 };

	bool m_wasUpdated { false };
	bool m_frameChanged { false };
	bool m_dimensionsChanged { false };
	std::mutex m_updateMutex;

	// opengl stuff
	GLuint m_framebuffer;
	GLuint m_framebufferTexture;
	GLuint m_fullscreenQuadVAO;
	GLuint m_fullscreenQuadBuffer;
	GLuint m_fullscreenQuadUVs;
	GLuint m_vertexShader, m_fragmentShader;
	GLuint m_shaderProgram;
	GLuint m_textureSamplerID;
};

