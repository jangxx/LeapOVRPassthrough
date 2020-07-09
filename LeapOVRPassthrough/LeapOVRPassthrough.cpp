#define GLEW_STATIC

#include "stdafx.h"
#include "windows.h"
#include <iostream>
#include <GL/glew.h>
#include <GL/freeglut.h>

#include "LeapHandler.h"
#include "OVROverlayController.h"
#include "GraphicsManager.h"

void dbg_init() {
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glColor3f(1.0, 0.0, 0.0);
	glPointSize(5.0);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0.0, 499.0, 0.0, 499.0);
}

void dbg_display() {
	glClear(GL_COLOR_BUFFER_BIT);
	glColor3f(1.0, 0.0, 0.0);

	glBegin(GL_POINTS);
	glVertex2f(10.0, 10.0);
	glVertex2f(150.0, 80.0);
	glVertex2f(100.0, 20.0);
	glVertex2f(200.0, 100.0);
	glEnd();
	glFlush();
}

int APIENTRY WinMain(HINSTANCE /*hInstance*/,
	HINSTANCE /*hPrevInstance*/,
	LPSTR /*lpCmdLine*/,
	int /*cmdShow*/)
{
	std::cout << "Hello World" << std::endl;

	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	glutInitWindowSize(500, 500);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("Hello OpenGL");
	glutDisplayFunc(dbg_display);

	//OVROverlayController* vrController = OVROverlayController::getInstance();
	LeapHandler* leapHandler = LeapHandler::getInstance();
	GraphicsManager* graphicsManager = GraphicsManager::getInstance();

	graphicsManager->init();

	//vrController->init();
	leapHandler->openConnection();

	dbg_init();
	glutMainLoop();

	leapHandler->join();

    return 0;
}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);

	return WinMain(GetModuleHandle(NULL), NULL, GetCommandLineA(), SW_SHOWNORMAL);
}