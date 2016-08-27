#include <GL/glew.h>
#include "vr/CGLRenderModel.hpp"
#include "glbase/gltool.hpp"

#include <QFile>
#include <QTextStream>

#include <glm/gtc/type_ptr.hpp>

std::vector<CGLRenderModel*> CGLRenderModel::m_vecRenderModels;

//-----------------------------------------------------------------------------
// Purpose: Allocates and populates the GL resources for a render model
//-----------------------------------------------------------------------------
bool CGLRenderModel::BInit(const vr::RenderModel_t & vrModel, const vr::RenderModel_TextureMap_t & vrDiffuseTexture)
{
	initShader();
	// create and bind a VAO to hold state for this model
	glGenVertexArrays(1, &m_glVertArray);
	glBindVertexArray(m_glVertArray);

	// Populate a vertex buffer
	glGenBuffers(1, &m_glVertBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, m_glVertBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vr::RenderModel_Vertex_t) * vrModel.unVertexCount, vrModel.rVertexData, GL_STATIC_DRAW);

	// Identify the components in the vertex buffer
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vr::RenderModel_Vertex_t), (void *)offsetof(vr::RenderModel_Vertex_t, vPosition));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vr::RenderModel_Vertex_t), (void *)offsetof(vr::RenderModel_Vertex_t, vNormal));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(vr::RenderModel_Vertex_t), (void *)offsetof(vr::RenderModel_Vertex_t, rfTextureCoord));

	// Create and populate the index buffer
	glGenBuffers(1, &m_glIndexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_glIndexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint16_t) * vrModel.unTriangleCount * 3, vrModel.rIndexData, GL_STATIC_DRAW);

	glBindVertexArray(0);

	// create and populate the texture
	glGenTextures(1, &m_glTexture);
	glBindTexture(GL_TEXTURE_2D, m_glTexture);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, vrDiffuseTexture.unWidth, vrDiffuseTexture.unHeight,
				 0, GL_RGBA, GL_UNSIGNED_BYTE, vrDiffuseTexture.rubTextureMapData);

	// If this renders black ask McJohn what's wrong.
	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	GLfloat fLargest;
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &fLargest);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, fLargest);

	glBindTexture(GL_TEXTURE_2D, 0);

	m_unVertexCount = vrModel.unTriangleCount * 3;

	return true;
}

void CGLRenderModel::initShader()
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

	VERIFY(_program);
}

std::string CGLRenderModel::getVertexShader()
{
	return loadShaderFile("../shader/mvp_pnt.vs.glsl");
}

std::string CGLRenderModel::getFragmentShader()
{
	return loadShaderFile("../shader/texture.fs.glsl");
}

std::string CGLRenderModel::loadShaderFile(std::string path)
{
	QFile f(path.c_str());
	if (!f.open(QFile::ReadOnly | QFile::Text))
		fprintf(stderr, "Could not open file %s\n", path.c_str());
	QTextStream in(&f);
	return in.readAll().toStdString();
}

CGLRenderModel* CGLRenderModel::FindOrLoadRenderModel(const char *pchRenderModelName)
{
	CGLRenderModel *pRenderModel = NULL;
	for (std::vector< CGLRenderModel * >::iterator i = m_vecRenderModels.begin(); i != m_vecRenderModels.end(); i++)
	{
		if (!stricmp((*i)->GetName().c_str(), pchRenderModelName))
		{
			pRenderModel = *i;
			break;
		}
	}

	// load the model if we didn't find one
	if (!pRenderModel)
	{
		vr::RenderModel_t *pModel;
		vr::EVRRenderModelError error;
		while (1)
		{
			error = vr::VRRenderModels()->LoadRenderModel_Async(pchRenderModelName, &pModel);
			if (error != vr::VRRenderModelError_Loading)
				break;

			//ThreadSleep(1);
		}

		if (error != vr::VRRenderModelError_None)
		{
			char buf[1024];
			sprintf_s(buf, sizeof(buf), "Unable to load render model %s - %s\n", pchRenderModelName, vr::VRRenderModels()->GetRenderModelErrorNameFromEnum(error));
			return NULL; // move on to the next tracked device
		}

		vr::RenderModel_TextureMap_t *pTexture;
		while (1)
		{
			error = vr::VRRenderModels()->LoadTexture_Async(pModel->diffuseTextureId, &pTexture);
			if (error != vr::VRRenderModelError_Loading)
				break;

			//ThreadSleep(1);
		}

		if (error != vr::VRRenderModelError_None)
		{
			char buf[1024];
			sprintf_s(buf, sizeof(buf), "Unable to load render texture id:%d for render model %s\n", pModel->diffuseTextureId, pchRenderModelName);
			vr::VRRenderModels()->FreeRenderModel(pModel);
			return NULL; // move on to the next tracked device
		}

		pRenderModel = new CGLRenderModel(pchRenderModelName, pModel, pTexture);
		if (!pRenderModel)
		{
			char buf[1024];
			sprintf_s(buf, sizeof(buf), "Unable to create GL model from render model %s\n", pchRenderModelName);
			delete pRenderModel;
			pRenderModel = NULL;
		}
		else
		{
			m_vecRenderModels.push_back(pRenderModel);
		}
		vr::VRRenderModels()->FreeRenderModel(pModel);
		vr::VRRenderModels()->FreeTexture(pTexture);
	}
	return pRenderModel;
}


void CGLRenderModel::render(glm::mat4 projection_matrix, glm::mat4 view_matrix, glm::mat4 model_matrix)
{

	if (_program == 0)
	{
		fprintf(stderr, "CGLRenderModel _program not initialized!\n");
		return;
	}

	// Load program
	glUseProgram(_program);

	glBindVertexArray(m_glVertArray);

	// set parameter
	glUniformMatrix4fv(locationM, 1, GL_FALSE, glm::value_ptr(model_matrix));
	glUniformMatrix4fv(locationV, 1, GL_FALSE, glm::value_ptr(view_matrix));
	glUniformMatrix4fv(locationP, 1, GL_FALSE, glm::value_ptr(projection_matrix));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_glTexture);

	glDrawElements(GL_TRIANGLES, m_unVertexCount, GL_UNSIGNED_SHORT, 0);

	glBindVertexArray(0);


	// check for errors
	VERIFY(CG::checkError());
}

void CGLRenderModel::Cleanup()
{
	if (m_glVertBuffer)
	{
		glDeleteBuffers(1, &m_glIndexBuffer);
		glDeleteVertexArrays(1, &m_glVertArray);
		glDeleteBuffers(1, &m_glVertBuffer);
		m_glIndexBuffer = 0;
		m_glVertArray = 0;
		m_glVertBuffer = 0;
	}
}

CGLRenderModel::CGLRenderModel(const std::string & sRenderModelName, vr::RenderModel_t *pModel, vr::RenderModel_TextureMap_t *pTexture)
	: m_sModelName(sRenderModelName)
{
	m_glIndexBuffer = 0;
	m_glVertArray = 0;
	m_glVertBuffer = 0;
	m_glTexture = 0;
	BInit(*pModel, *pTexture);
}


CGLRenderModel::~CGLRenderModel()
{
	Cleanup();
}