#include <GL/glew.h>

#include "skybox.h"

#include <glm/gtc/type_ptr.hpp>

#include "glbase/gltool.hpp"

#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include <algorithm>
#include <iostream>
#include <stack>

#include <qimage.h>

#include "gui/config.h"
#include <openvr.h>

Skybox::Skybox(std::string name)
{
}

void Skybox::init()
{
	// init calls  initShader(); and createObject();
	Drawable::init();
	loadTexture();
}

void Skybox::draw(glm::mat4 projection_matrix, glm::mat4 view_matrix) const
{

	// Load program
	glUseProgram(_program);

	// bind vertex array object
	glBindVertexArray(_vertexArrayObject);
	// skybox does not have a position
	view_matrix = glm::mat4(glm::mat3(view_matrix));

	// sends projection matrix and modelviewmatrix to shader
	glUniformMatrix4fv(locationP, 1, GL_FALSE, glm::value_ptr(projection_matrix));
	glUniformMatrix4fv(locationM, 1, GL_FALSE, glm::value_ptr(model_matrix));
	glUniformMatrix4fv(locationV, 1, GL_FALSE, glm::value_ptr(view_matrix));

	// call draw
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

	// unbind vertex array object
	glBindVertexArray(0);

	// check for errors
	VERIFY(CG::checkError());
}

std::string Skybox::getVertexShader() const
{
	return Drawable::loadShaderFile("../shader/skybox.vs.glsl");
}

std::string Skybox::getFragmentShader() const
{
	return Drawable::loadShaderFile("../shader/skybox.fs.glsl");
}

void Skybox::update(float timeElapsedMS, glm::mat4 modelMatrix)
{
	model_matrix = modelMatrix;
}

void Skybox::loadTexture()
{
	/// SKYBOXES:
	/// negative X = back
	/// negative y = down
	/// negative z = right
	/// positive x = front
	/// positive y = up
	/// positive z = left


	// initialize unsigned integer

	GLuint textureID;

	// get a free index
	glGenTextures(1, &textureID);

	// load image from resource
	QImage image[6];

	image[0] = QImage(QString::fromStdString("../textures/skybox/right.jpg"));
	image[1] = QImage(QString::fromStdString("../textures/skybox/left.jpg"));
	image[2] = QImage(QString::fromStdString("../textures/skybox/back.jpg"));
	image[3] = QImage(QString::fromStdString("../textures/skybox/front.jpg"));
	image[4] = QImage(QString::fromStdString("../textures/skybox/up.jpg"));
	image[5] = QImage(QString::fromStdString("../textures/skybox/down.jpg"));

	for (int i = 0; i < 6; i++)
	{
		image[i] = image[i].convertToFormat(QImage::Format_RGBA8888);
	}

	for (int i = 0; i < 4; i++)
		image[i] = image[i].mirrored(true, false);
	image[4] = image[4].mirrored(false, true);
	image[5] = image[5].mirrored(false, true);

	GLuint compSkybox[6];
	vr::Texture_t* skyboxpointer = new vr::Texture_t[6];
	for (int i = 0; i < 6; i++)
	{
		glGenTextures(1, &compSkybox[i]);
		glBindTexture(GL_TEXTURE_2D, compSkybox[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image[i].width(), image[i].height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, image[i].bits());
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		skyboxpointer[i] = { (void*)compSkybox[i], vr::API_OpenGL, vr::ColorSpace_Gamma };
	}

	if (Config::submitSkybox)
		vr::EVRCompositorError cError = vr::VRCompositor()->SetSkyboxOverride(skyboxpointer, 6);

	for (int i = 0; i < 4; i++)
		image[i] = image[i].mirrored(true, false);
	image[4] = image[4].mirrored(false, true);
	image[5] = image[5].mirrored(false, true);

	// parameterize cube map
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGBA, image[0].width(), image[0].height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, image[0].bits());
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGBA, image[1].width(), image[1].height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, image[1].bits());
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGBA, image[2].width(), image[2].height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, image[2].bits());
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGBA, image[3].width(), image[3].height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, image[3].bits());
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGBA, image[4].width(), image[4].height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, image[4].bits());
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGBA, image[5].width(), image[5].height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, image[5].bits());


	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);	

	VERIFY(CG::checkError());
}

void Skybox::createObject()
{

	std::vector<glm::vec3> positions;
	std::vector<unsigned int> indices;

	// positions (corners of the cube)
	// https://homepages.thm.de/~hg10013/Lehre/MMS/SS02/Deseyve/Images/indwur.gif

	positions.push_back(glm::vec3(-1, -1, -1)); // 0 -> 0
	positions.push_back(glm::vec3(-1, -1, 1)); // 1 -> 1
	positions.push_back(glm::vec3(1, -1, -1)); // 4 -> 2
	positions.push_back(glm::vec3(1, -1, 1)); // 5 -> 3
	positions.push_back(glm::vec3(1, 1, -1)); // 6 -> 4
	positions.push_back(glm::vec3(1, 1, 1)); // 7 -> 5
	positions.push_back(glm::vec3(-1, 1, -1)); // 2 -> 6
	positions.push_back(glm::vec3(-1, 1, 1)); // 3 -> 7


											  // indices (two triangles for each side of the cube)
											  // front

	indices.push_back(0);
	indices.push_back(1);
	indices.push_back(2);

	indices.push_back(2);
	indices.push_back(1);
	indices.push_back(3);

	// back

	indices.push_back(4);
	indices.push_back(5);
	indices.push_back(6);

	indices.push_back(6);
	indices.push_back(5);
	indices.push_back(7);

	// left
	indices.push_back(6);
	indices.push_back(7);
	indices.push_back(0);

	indices.push_back(0);
	indices.push_back(7);
	indices.push_back(1);

	// right
	indices.push_back(2);
	indices.push_back(3);
	indices.push_back(4);

	indices.push_back(4);
	indices.push_back(3);
	indices.push_back(5);

	// top
	indices.push_back(1);
	indices.push_back(7);
	indices.push_back(3);

	indices.push_back(3);
	indices.push_back(7);
	indices.push_back(5);

	// bottom
	indices.push_back(0);
	indices.push_back(2);
	indices.push_back(6);

	indices.push_back(2);
	indices.push_back(4);
	indices.push_back(6);

	// Set up a vertex array object for the geometry
	if (_vertexArrayObject == 0)
		glGenVertexArrays(1, &_vertexArrayObject);
	glBindVertexArray(_vertexArrayObject);

	// fill vertex array object with data
	GLuint position_buffer;
	glGenBuffers(1, &position_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, position_buffer);
	glBufferData(GL_ARRAY_BUFFER, positions.size() * 3 * sizeof(float), positions.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	GLuint index_buffer;
	glGenBuffers(1, &index_buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

	// unbind vertex array object
	glBindVertexArray(0);
	// delete buffers (the data is stored in the vertex array object)
	glDeleteBuffers(1, &position_buffer);
	glDeleteBuffers(1, &index_buffer);

	// check for errors
	VERIFY(CG::checkError());
}

