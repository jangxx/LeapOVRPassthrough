#pragma once
#include <GL/glew.h>
#include <iostream>
#include <mutex>
#include <cassert>

#include "OVROverlayController.h"

extern "C" {
	#include <LeapC.h>
}

class GraphicsManager
{
public:
	static GraphicsManager* getInstance();

	GraphicsManager();
	~GraphicsManager();

	bool init();
	void updateTexture();
	void setFrame(int width, int height, uint8_t* data);
	void setDistortionMap(float* data);
	void setDistortionMapActive(bool active);
	bool getDistortionMapActive();
	GLuint getVideoTexture();
	bool wasUpdated();

private:
	void updateFramebuffer();
	void setFramebufferSize(int width, int height);

	GLuint m_videoTexture { 0 };
	GLuint m_distortionTexture{ 0 };

	uint8_t* m_pixelData { nullptr };
	float* m_distortionPixelData { nullptr };
	int m_width { 100 };
	int m_height { 100 };
	int m_fbWidth { 640 };
	int m_fbHeight { 480 };
	int m_useDistortionMap { false };

	bool m_wasUpdated { false };
	bool m_frameChanged { false };
	bool m_dimensionsChanged { false };
	bool m_distortionMapChanged { false };
	std::mutex m_updateMutex;

	// opengl stuff
	GLuint m_framebuffer { 0 };
	GLuint m_framebufferTexture { 0 };
	GLuint m_fullscreenQuadVAO { 0 };
	GLuint m_fullscreenQuadBuffer { 0 };
	GLuint m_fullscreenQuadUVs { 0 };
	GLuint m_vertexShader { 0 };
	GLuint m_fragmentShader { 0 };
	GLuint m_shaderProgram { 0 };
	GLuint m_textureSamplerID { 0 };
	GLuint m_distortionTextureSamplerID { 0 };
	GLuint m_useDistortionMapID { 0 };
};

