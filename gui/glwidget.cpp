#include <GL/glew.h>
#include "glwidget.hpp"

#include <QMouseEvent>

#include <iostream>

#define GLM_FORCE_RADIANS
#define GLM_SWIZZLE
#include <glm/gtx/transform.hpp>

#include "gui/config.h"

#include "geometry/cube.h"
#include "geometry/skybox.h"

GLWidget::GLWidget(QWidget *&parent) : QOpenGLWidget(parent),
_updateTimer(this), _stopWatch()
{
	// update the scene periodically
	QObject::connect(&_updateTimer, SIGNAL(timeout()), this, SLOT(animateGL()));
	_updateTimer.start(11);
	_stopWatch.start();

	// create all drawable elements
	_skybox = std::make_shared<Skybox>("Skybox");
	_earth = std::make_shared<Cube>("Erde", 1.0, 0.0, 24.0, 1, "../textures/earth.bmp");
}

void GLWidget::show()
{
	QOpenGLWidget::show();

	// check for a valid context
	if (!isValid() || !context()->isValid() || context()->format().majorVersion() != 4)
	{
		QMessageBox::critical(this, "Error", "Cannot get a valid OpenGL 4 context.");
		exit(1);
	}
}

void GLWidget::initializeGL()
{
	/* Initialize OpenGL extensions */
	glewExperimental = GL_TRUE; // otherwise some function pointers are NULL...
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		/* Problem: glewInit failed, something is seriously wrong. */
		fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
	}
	glGetError(); // clear a gl error produced by glewInit

	hmd::init();
	_skybox->init();
	_earth->init();
	

}

void GLWidget::resizeGL(int width, int height)
{
	// update the viewport
	glViewport(0, 0, width, height);
	Config::resolution = glm::vec2(width, height);
}

void GLWidget::paintGL()
{
	glClearColor(0.15f, 0.15f, 0.18f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	if (hmd::IsInputFocusCapturedByAnotherProcess())
		return;

	// process VR events
	hmd::ProcessVREvents();

	// get new positions
	hmd::WaitGetPoses();

	// update controller states
	hmd::GetControllerState();

	if (hmd::controller.menu.isToggled)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	// renderpasses
	for (int eye = 0; eye < 2; eye++)
	{
		// bind framebuffer and stuff
		hmd::preRender(eye);

		// actual rendering
		
		_earth->draw(hmd::getProjectionMatrix(eye), hmd::getViewMatrix(eye));
		hmd::drawRenderModels(hmd::getProjectionMatrix(eye), hmd::getViewMatrix(eye));
		_skybox->draw(hmd::getProjectionMatrix(eye), hmd::getViewMatrix(eye));

		// submitting stuff
		hmd::postRender(eye);
	}

	// setup spectator view
	glm::mat4 view_matrix = hmd::getPose();
	glm::mat4 projection_matrix = glm::perspective(glm::radians(70.0f),
												   Config::resolution.x / Config::resolution.y,
												   0.1f, 100.0f);
	glViewport(0, 0, Config::resolution.x, Config::resolution.y);



	// third person spectator test
	/*
	view_matrix = glm::inverse(view_matrix);
	view_matrix = glm::translate(view_matrix, glm::vec3(0, 1, 1));
	view_matrix = glm::rotate(view_matrix, glm::radians(-30.0f), glm::vec3(1, 0, 0));
	view_matrix = glm::inverse(view_matrix);
	*/

	
	_earth->draw(projection_matrix, view_matrix);
	hmd::drawRenderModels(projection_matrix, view_matrix);
	_skybox->draw(projection_matrix, view_matrix);
}

void GLWidget::mousePressEvent(QMouseEvent *event)
{
	// ignore if it was not the left mouse button
	if (!event->button() != Qt::LeftButton)
		return;

}

void GLWidget::mouseReleaseEvent(QMouseEvent *event)
{
	// ignore if it was not the left mouse button
	if (!event->button() != Qt::LeftButton)
		return;

}

void GLWidget::mouseMoveEvent(QMouseEvent *event)
{

}

void GLWidget::wheelEvent(QWheelEvent *event)
{
	// event->angleDelta().ry()
}

void GLWidget::animateGL()
{

	// get the time delta
	float timeElapsedMs = _stopWatch.nsecsElapsed() / 1000000.0f;
	// restart stopwatch for next update
	_stopWatch.restart();

	// calculate current model_matrix
	glm::mat4 model_matrix;
		

	// update drawables
	_skybox->update(timeElapsedMs, model_matrix);
	_earth->update(timeElapsedMs, model_matrix);

	// update the widget (do not remove this!)
	update();

}


