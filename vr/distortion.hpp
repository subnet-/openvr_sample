#ifndef DISTORTION_H
#define DISTORTION_H

#include <glm/vec2.hpp>
#include <openvr.h>
#include <vector>
#include "vr/hmd.h"
#include <QFile>
#include <QTextStream>


struct VertexDataLens
{
	glm::vec2 position;
	glm::vec2 texCoordRed;
	glm::vec2 texCoordGreen;
	glm::vec2 texCoordBlue;
};

class distortion
{

public:
	static void init(vr::IVRSystem *_hmd);
	static void render(int eyeIndex, int width, int height);

private:
	static unsigned int m_uiIndexSize;
	static GLuint m_unLensVAO;
	static GLuint _program;

	static std::string getVertexShader();
	static std::string getFragmentShader();
	static std::string loadShaderFile(std::string path);

};
#endif