#include <GL/glew.h>
#include "vr/distortion.hpp"
#include "glbase/gltool.hpp"

unsigned int distortion::m_uiIndexSize;
GLuint distortion::m_unLensVAO;
GLuint distortion::_program;

void distortion::init(vr::IVRSystem *_hmd)
{
	GLuint vs = CG::createCompileShader(GL_VERTEX_SHADER, getVertexShader()); VERIFY(vs);
	GLuint fs = CG::createCompileShader(GL_FRAGMENT_SHADER, getFragmentShader()); VERIFY(fs);

	_program = glCreateProgram();
	glAttachShader(_program, vs);
	glAttachShader(_program, fs);
	_program = CG::linkProgram(_program);

	VERIFY(_program);




	GLushort m_iLensGridSegmentCountH = 43;
	GLushort m_iLensGridSegmentCountV = 43;

	float w = (float)(1.0 / float(m_iLensGridSegmentCountH - 1));
	float h = (float)(1.0 / float(m_iLensGridSegmentCountV - 1));

	float u, v = 0;

	std::vector<glm::vec2> positions;
	std::vector<glm::vec2> texCoordRed;
	std::vector<glm::vec2> texCoordGreen;
	std::vector<glm::vec2> texCoordBlue;

	//left eye distortion verts
	float Xoffset = -1;
	for (int y = 0; y<m_iLensGridSegmentCountV; y++)
	{
		for (int x = 0; x<m_iLensGridSegmentCountH; x++)
		{
			u = x*w; v = 1 - y*h;
			positions.push_back(glm::vec2(Xoffset + u, -1 + 2 * y*h));

			vr::DistortionCoordinates_t dc0 = _hmd->ComputeDistortion(vr::Eye_Left, u, v);

			texCoordRed.push_back(glm::vec2(dc0.rfRed[0], 1 - dc0.rfRed[1]));
			texCoordGreen.push_back(glm::vec2(dc0.rfGreen[0], 1 - dc0.rfGreen[1]));
			texCoordBlue.push_back(glm::vec2(dc0.rfBlue[0], 1 - dc0.rfBlue[1]));
		}
	}

	//right eye distortion verts
	Xoffset = 0;
	for (int y = 0; y<m_iLensGridSegmentCountV; y++)
	{
		for (int x = 0; x<m_iLensGridSegmentCountH; x++)
		{
			u = x*w; v = 1 - y*h;
			positions.push_back(glm::vec2(Xoffset + u, -1 + 2 * y*h));

			vr::DistortionCoordinates_t dc0 = _hmd->ComputeDistortion(vr::Eye_Right, u, v);

			texCoordRed.push_back(glm::vec2(dc0.rfRed[0], 1 - dc0.rfRed[1]));
			texCoordGreen.push_back(glm::vec2(dc0.rfGreen[0], 1 - dc0.rfGreen[1]));
			texCoordBlue.push_back(glm::vec2(dc0.rfBlue[0], 1 - dc0.rfBlue[1]));
		}
	}

	std::vector<unsigned int> vIndices;
	unsigned int a, b, c, d;

	unsigned int offset = 0;
	for (unsigned int y = 0; y<m_iLensGridSegmentCountV - 1; y++)
	{
		for (unsigned int x = 0; x<m_iLensGridSegmentCountH - 1; x++)
		{
			a = m_iLensGridSegmentCountH*y + x + offset;
			b = m_iLensGridSegmentCountH*y + x + 1 + offset;
			c = (y + 1)*m_iLensGridSegmentCountH + x + 1 + offset;
			d = (y + 1)*m_iLensGridSegmentCountH + x + offset;
			vIndices.push_back(a);
			vIndices.push_back(b);
			vIndices.push_back(c);

			vIndices.push_back(a);
			vIndices.push_back(c);
			vIndices.push_back(d);
		}
	}

	offset = (m_iLensGridSegmentCountH)*(m_iLensGridSegmentCountV);
	for (unsigned int y = 0; y<m_iLensGridSegmentCountV - 1; y++)
	{
		for (unsigned int x = 0; x<m_iLensGridSegmentCountH - 1; x++)
		{
			a = m_iLensGridSegmentCountH*y + x + offset;
			b = m_iLensGridSegmentCountH*y + x + 1 + offset;
			c = (y + 1)*m_iLensGridSegmentCountH + x + 1 + offset;
			d = (y + 1)*m_iLensGridSegmentCountH + x + offset;
			vIndices.push_back(a);
			vIndices.push_back(b);
			vIndices.push_back(c);

			vIndices.push_back(a);
			vIndices.push_back(c);
			vIndices.push_back(d);
		}
	}

	m_uiIndexSize = vIndices.size();

	glGenVertexArrays(1, &m_unLensVAO);
	glBindVertexArray(m_unLensVAO);

	GLuint position_buffer;
	glGenBuffers(1, &position_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, position_buffer);
	glBufferData(GL_ARRAY_BUFFER, positions.size() * 2 * sizeof(float), positions.data() , GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

	GLuint red_buffer;
	glGenBuffers(1, &red_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, red_buffer);
	glBufferData(GL_ARRAY_BUFFER, texCoordRed.size() * 2 * sizeof(float), texCoordRed.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);

	GLuint green_buffer;
	glGenBuffers(1, &green_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, green_buffer);
	glBufferData(GL_ARRAY_BUFFER, texCoordGreen.size() * 2 * sizeof(float), texCoordGreen.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);

	GLuint blue_buffer;
	glGenBuffers(1, &blue_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, blue_buffer);
	glBufferData(GL_ARRAY_BUFFER, texCoordBlue.size() * 2 * sizeof(float), texCoordBlue.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 0, 0);

	GLuint index_buffer;
	glGenBuffers(1, &index_buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, vIndices.size() * sizeof(unsigned int), vIndices.data(), GL_STATIC_DRAW);

	glBindVertexArray(0);

	//glDisableVertexAttribArray(0);
	//glDisableVertexAttribArray(1);
	//glDisableVertexAttribArray(2);
	//glDisableVertexAttribArray(3);
	glDeleteBuffers(1, &position_buffer);
	glDeleteBuffers(1, &red_buffer);
	glDeleteBuffers(1, &green_buffer);
	glDeleteBuffers(1, &blue_buffer);

	// check for errors
	VERIFY(CG::checkError());

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void distortion::render(int eyeIndex, int width, int height)
{
	glDisable(GL_DEPTH_TEST);
	glBindFramebuffer(GL_FRAMEBUFFER, hmd::getFramebuffer(eyeIndex).m_nRenderFramebufferId);
	glViewport(0, 0, width, height);

	glBindVertexArray(m_unLensVAO);
	glUseProgram(_program);

	// render one lens (half of index array )
	glActiveTexture(GL_TEXTURE1);
	glUniform1i(glGetUniformLocation(_program, "mytexture"), 1);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, hmd::getFramebuffer(eyeIndex).m_nRenderTextureId);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, hmd::getFramebuffer(eyeIndex).m_nResolveTextureId, 0);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	//glDrawElements(GL_TRIANGLES, m_uiIndexSize / 2, GL_UNSIGNED_SHORT, (const void *)(eyeIndex * m_uiIndexSize));
	if (eyeIndex == 0)
		glDrawElements(GL_TRIANGLES, m_uiIndexSize / 2, GL_UNSIGNED_SHORT, 0);
	else
		glDrawElements(GL_TRIANGLES, m_uiIndexSize / 2, GL_UNSIGNED_SHORT, (const void *)(m_uiIndexSize/2));

	glBindVertexArray(0);
	glUseProgram(0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glEnable(GL_DEPTH_TEST);

	// check for errors
	VERIFY(CG::checkError());
}

std::string distortion::getVertexShader()
{
	return loadShaderFile("../shader/distortion.vs.glsl");
}

std::string distortion::getFragmentShader()
{
	return loadShaderFile("../shader/distortion.fs.glsl");
}

std::string distortion::loadShaderFile(std::string path)
{
	QFile f(path.c_str());
	if (!f.open(QFile::ReadOnly | QFile::Text))
		fprintf(stderr, "Could not open file %s\n", path.c_str());
	QTextStream in(&f);
	return in.readAll().toStdString();
}