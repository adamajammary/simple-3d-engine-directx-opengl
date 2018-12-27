#ifndef S3DE_GLOBALS_H
	#include "../globals.h"
#endif

#ifndef S3DE_SHADERPROGRAM_H
#define S3DE_SHADERPROGRAM_H

class ShaderProgram
{
public:
	ShaderProgram(const wxString &name);
	~ShaderProgram();

public:
	GLuint Attribs[NR_OF_ATTRIBS];
	GLint  Uniforms[NR_OF_UBOS_GL];
	GLuint UniformBuffers[NR_OF_UBOS_GL];

private:
	wxString       name;
	GLuint         program;
	VkShaderModule vulkanFS;
	VkShaderModule vulkanVS;

	#if defined _WINDOWS
		ID3D11PixelShader*  shaderFS;
		ID3D11VertexShader* shaderVS;
		ID3DBlob*           fs;
		ID3DBlob*           vs;
	#endif

public:
	bool           IsOK();
	int            Link();
	int            Load(const wxString &shaderFile);
	int            LoadAndLink(const wxString &vs, const wxString &fs);
	void           Log();
	void           Log(GLuint shader);
	wxString       Name();
	GLuint         Program();
	int            UpdateAttribsGL(Component* mesh);
	int            UpdateUniformsGL(Component* mesh, const DrawProperties &properties = {});
	int            UpdateUniformsVK(VkDevice deviceContext, Component* mesh, const VKUniform &uniform, const DrawProperties &properties = {});
	VkShaderModule VulkanFS();
	VkShaderModule VulkanVS();

	#if defined _WINDOWS
		ShaderID            ID();
		ID3DBlob*           FS();
		ID3DBlob*           VS();
		ID3D11PixelShader*  FragmentShader();
		ID3D11VertexShader* VertexShader();
		int                 UpdateUniformsDX11(ID3D11Buffer** constBuffer, const void** constBufferValues, Component* mesh, const DrawProperties &properties = {});
		int                 UpdateUniformsDX12(Component* mesh, const DrawProperties &properties = {});
	#endif

private:
	int  loadShaderGL(GLuint type, const wxString &sourceText);
	void setAttribsGL();
	void setUniformsGL();
	void updateUniformGL(GLint id, UniformBufferTypeGL buffer, void* values, size_t valuesSize);
	int  updateUniformsVK(UniformBufferTypeVK type, UniformBinding binding, const VKUniform &uniform, void* values, size_t valuesSize, VkDevice deviceContext, Component* mesh);
	int  updateUniformSamplersVK(VkDescriptorSet uniformSet, VkDevice deviceContext, Component* mesh);

	#if defined _WINDOWS
		const void* getBufferValues(const CBMatrix &matrices, Component* mesh, const DrawProperties &properties, size_t &bufferSize);
	#endif
};

#endif
