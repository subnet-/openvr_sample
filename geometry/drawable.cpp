#include <GL/glew.h>

#include "drawable.h"

#include <iostream>

#include <QImage>
#include <QFile>
#include <QTextStream>

#include "glbase/gltool.hpp"


Drawable::Drawable() :
	_vertexArrayObject(0),
	model_matrix()
{
}

void Drawable::init()
{
	initShader();
	createObject();
}

void Drawable::recreate()
{
	createObject();
}

void Drawable::initShader()
{
	GLuint vs = CG::createCompileShader(GL_VERTEX_SHADER, getVertexShader()); VERIFY(vs);
	GLuint fs = CG::createCompileShader(GL_FRAGMENT_SHADER, getFragmentShader()); VERIFY(fs);

	_program = glCreateProgram();
	glAttachShader(_program, vs);
	glAttachShader(_program, fs);
	_program = CG::linkProgram(_program);

	locationM = glGetUniformLocation(_program, "model_matrix");
	locationV = glGetUniformLocation(_program, "view_matrix");
	locationP = glGetUniformLocation(_program, "projection_matrix");
	locationT = glGetUniformLocation(_program, "tex");

	VERIFY(_program);
}


std::string Drawable::loadShaderFile(std::string path) const
{
	QFile f(path.c_str());
	if (!f.open(QFile::ReadOnly | QFile::Text))
		std::cout << "Could not open file " << path << std::endl;
	QTextStream in(&f);
	return in.readAll().toStdString();
}

GLuint Drawable::loadTexture(std::string path)
{

	// initialize unsigned integer
	GLuint textureID;

	// get a free index
	glGenTextures(1, &textureID);

	// bind texture
	glBindTexture(GL_TEXTURE_2D, textureID);

	// load image from resource
	QImage image(QString::fromStdString(path));
	image = image.convertToFormat(QImage::Format_RGBA8888);

	// generate texture
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.width(), image.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, image.bits());

	// texture settings
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	return textureID;
}
