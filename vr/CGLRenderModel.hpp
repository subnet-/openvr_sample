#ifndef CGLRENDERMODEL_H
#define CGLRENDERMODEL_H

#include <string>
#include <openvr.h>
#include <glm/mat4x4.hpp>
#include <vector>
//#include <glm/vec4.hpp>

class CGLRenderModel
{
public:
	CGLRenderModel(const std::string & sRenderModelName, vr::RenderModel_t *pModel, vr::RenderModel_TextureMap_t *pTexture);
	~CGLRenderModel();

	bool BInit(const vr::RenderModel_t & vrModel, const vr::RenderModel_TextureMap_t & vrDiffuseTexture);
	void Cleanup();
	void render(glm::mat4 projection_matrix, glm::mat4 view_matrix, glm::mat4 model_matrix);
	const std::string & GetName() const { return m_sModelName; }
	void initShader();

	static CGLRenderModel* FindOrLoadRenderModel(const char * pchRenderModelName);

private:
	static std::vector<CGLRenderModel*> m_vecRenderModels;

	GLuint m_glVertBuffer;
	GLuint m_glIndexBuffer;
	GLuint m_glVertArray;
	GLuint m_glTexture;
	GLsizei m_unVertexCount;
	GLuint _program;
	std::string m_sModelName;

	std::string getVertexShader();
	std::string getFragmentShader();
	std::string loadShaderFile(std::string path);

	

	GLint locationM;
	GLint locationV;
	GLint locationP;
};

#endif