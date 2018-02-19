#ifndef GE3D_GLOBALS_H
	#include "../globals.h"
#endif

#ifndef GE3D_SHADERPROGRAM_H
#define GE3D_SHADERPROGRAM_H

class ShaderProgram
{
public:
	ShaderProgram(const wxString &name);
	~ShaderProgram();

public:
	GLuint Attribs[NR_OF_ATTRIBS];
	GLint  Uniforms[NR_OF_UNIFORMS];

private:
	wxString name;
	GLuint   program;

	#ifdef _WINDOWS
		ID3D11PixelShader*  shaderFS;
		ID3D11VertexShader* shaderVS;
		ID3DBlob*           fs;
		ID3DBlob*           vs;
	#endif

public:
	bool     IsOK();
	int      Link();
	int      Load(const wxString &shaderFile);
	int      LoadAndLink(const wxString &vs, const wxString &fs);
	int      LoadShader(GLuint type, const wxString &sourceText);
	wxString Name();
	GLuint   Program();
	int      UpdateAttribsGL(Mesh* mesh);
	int      UpdateUniformsDX11(ID3D11Buffer** constBuffer, const void** constBufferValues, Mesh* mesh, bool enableClipping = false, const glm::vec3 &clipMax = glm::vec3(0.0f, 0.0f, 0.0f), const glm::vec3 &clipMin = glm::vec3(0.0f, 0.0f, 0.0f));
	int      UpdateUniformsDX12(Mesh* mesh, bool enableClipping = false, const glm::vec3 &clipMax = glm::vec3(0.0f, 0.0f, 0.0f), const glm::vec3 &clipMin = glm::vec3(0.0f, 0.0f, 0.0f));
	int      UpdateUniformsGL(Mesh*   mesh, bool enableClipping = false, const glm::vec3 &clipMax = glm::vec3(0.0f, 0.0f, 0.0f), const glm::vec3 &clipMin = glm::vec3(0.0f, 0.0f, 0.0f));
	
	#ifdef _WINDOWS
		ShaderID            ID();
		ID3DBlob*           FS();
		ID3DBlob*           VS();
		ID3D11PixelShader*  FragmentShader();
		ID3D11VertexShader* VertexShader();
	#endif

private:
	void setAttribs();
	void setUniforms();

	#ifdef _WINDOWS
		DXLightBuffer  getBufferLight();
		DXMatrixBuffer getBufferMatrices(Mesh* mesh, bool removeTranslation = false);
		const void*    getBufferValues(const DXMatrixBuffer &matrices, const DXLightBuffer &sunLight, Mesh* mesh, bool enableClipping = false, const glm::vec3 &clipMax = glm::vec3(0.0f, 0.0f, 0.0f), const glm::vec3 &clipMin = glm::vec3(0.0f, 0.0f, 0.0f));
	#endif
};

#endif
