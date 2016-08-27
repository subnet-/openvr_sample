#include <GL/glew.h>
#include "cube.h"


#define GLM_FORCE_RADIANS
#define GLM_SWIZZLE
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <algorithm>
#include <iostream>
#include <stack>

#include "glbase/gltool.hpp"

#include "gui/config.h"

Cube::Cube(std::string name,
	float radius,
	float distance,
	float hoursPerDay,
	unsigned int daysPerYear,
	std::string textureLocation) :
	Drawable(),
	_radius(radius),
	_distance(distance),
	_localRotation(0),
	_localRotationSpeed(0),
	_daysPerYear(daysPerYear)
{
	_localRotationSpeed = 1.0f / hoursPerDay;
}

void Cube::init()
{
	Drawable::init();
	_texture = loadTexture("../textures/earth.jpg");
}

void Cube::recreate()
{
	Drawable::recreate();
}



void Cube::draw(glm::mat4 projection_matrix, glm::mat4 view_matrix) const
{
	if (_program == 0) {
		std::cerr << "Not initialized. Call init() first." << std::endl;
		return;
	}

	// Load program
	glUseProgram(_program);

	// bin vertex array object
	glBindVertexArray(_vertexArrayObject);

	glUniformMatrix4fv(locationM, 1, GL_FALSE, glm::value_ptr(model_matrix));
	glUniformMatrix4fv(locationV, 1, GL_FALSE, glm::value_ptr(view_matrix));
	glUniformMatrix4fv(locationP, 1, GL_FALSE, glm::value_ptr(projection_matrix));

	// textures
	glActiveTexture(GL_TEXTURE0);
	glUniform1i(locationT, 0);
	glBindTexture(GL_TEXTURE_2D, _texture);

	// call draw
	glDrawElements(GL_TRIANGLES, _indices, GL_UNSIGNED_INT, 0);

	// unbin vertex array object
	glBindVertexArray(0);

	// check for errors
	VERIFY(CG::checkError());
}

void Cube::update(float elapsedTimeMs, glm::mat4 modelViewMatrix)
{
	// calculate new local rotation
	if (Config::localRotation)
		_localRotation += elapsedTimeMs * _localRotationSpeed * Config::animationSpeed;

	// keep rotation between 0 and 360
	while (_localRotation >= 360.f)
		_localRotation -= 360.0f;
	while (_localRotation < 0.0f)
		_localRotation += 360.0f;

	// return if the pose is invalid
	if (modelViewMatrix == glm::mat4(0))
		return;
	std::stack<glm::mat4> modelview_stack;

	modelview_stack.push(modelViewMatrix);

	
	modelview_stack.top() = glm::rotate(modelview_stack.top(), glm::radians(-90.0f), glm::vec3(1, 0, 0));
	// rotate around y-axis
	modelview_stack.top() = glm::rotate(modelview_stack.top(), glm::radians(_localRotation), glm::vec3(0, 1, 0));
	
	modelview_stack.top() = glm::scale(modelview_stack.top(), glm::vec3(.2, .2, .2));
	
	model_matrix = glm::mat4(modelview_stack.top());

	modelview_stack.pop();
}

void Cube::addChild(std::shared_ptr<Cube> child)
{
	_children.push_back(child);
}


void Cube::createObject() {

	std::vector<glm::vec3> positions;
	std::vector<glm::vec2> textures;
	std::vector<glm::vec3> normals;
	std::vector<unsigned int> indices;

	int _resolution = 40;

	int offset = 0;
	int mod = (_resolution + 1) * (_resolution + 1);
	float x, y, z, phi, theta;
	unsigned int i11, i12, i13, i21, i22, i23;
	int i, j;
	for (i = 0; i <= _resolution; i++) {
		for (j = 0; j <= _resolution; j++) {

			phi = 2 * glm::pi<float>() * j / (float)_resolution;
			theta = glm::pi<float>() * i / (float)_resolution;

			x = sin(theta) * sin(phi);
			y = cos(theta);
			z = sin(theta) * cos(phi);

			positions.push_back(_radius*glm::vec3(x, y, z));
			normals.push_back(_radius*glm::vec3(x, y, z));
			textures.push_back(glm::vec2(j / (float)_resolution, i / (float)_resolution));

			if (j != _resolution && i != _resolution) {
				i11 = (offset + j) % mod;
				i12 = (offset + j + _resolution + 1) % mod;
				i13 = (offset + j + 1) % mod;

				i21 = (offset + j + _resolution + 1) % mod;
				i22 = (offset + j + _resolution + 2) % mod;
				i23 = (offset + j + 1) % mod;

				indices.push_back(i11);
				indices.push_back(i12);
				indices.push_back(i13);

				indices.push_back(i21);
				indices.push_back(i22);
				indices.push_back(i23);
			}

		}
		offset += _resolution + 1;
	}

	// Set up a vertex array object for the geometry
	if (_vertexArrayObject == 0)
		glGenVertexArrays(1, &_vertexArrayObject);
	glBindVertexArray(_vertexArrayObject);

	// fill vertex array object with positional data
	GLuint position_buffer;
	glGenBuffers(1, &position_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, position_buffer);
	glBufferData(GL_ARRAY_BUFFER, positions.size() * 3 * sizeof(float), positions.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	// fill vertex array object with normal data
	GLuint normal_buffer;
	glGenBuffers(1, &normal_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, normal_buffer);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * 3 * sizeof(float), normals.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	// fill vertex array object with texture coords
	GLuint texture_buffer;
	glGenBuffers(1, &texture_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, texture_buffer);
	glBufferData(GL_ARRAY_BUFFER, textures.size() * 2 * sizeof(float), textures.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(2);

	GLuint index_buffer;
	glGenBuffers(1, &index_buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

	// unbind vertex array object
	glBindVertexArray(0);
	// delete buffers (the data is stored in the vertex array object)
	glDeleteBuffers(1, &position_buffer);
	glDeleteBuffers(1, &normal_buffer);
	glDeleteBuffers(1, &texture_buffer);
	glDeleteBuffers(1, &index_buffer);

	// check for errors
	VERIFY(CG::checkError());

	_indices = indices.size();

}

std::string Cube::getVertexShader() const
{
	return Drawable::loadShaderFile("../shader/mvp_pnt.vs.glsl");

}

std::string Cube::getFragmentShader() const
{
	return Drawable::loadShaderFile("../shader/texture.fs.glsl");
}

Cube::~Cube() {
}