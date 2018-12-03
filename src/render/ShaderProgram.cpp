#include "ShaderProgram.h"

ShaderProgram::ShaderProgram(const wxString &name)
{
	this->name          = name;
	this->program       = 0;
	this->vulkanFS      = nullptr;
	this->vulkanVS      = nullptr;

	#if defined _WINDOWS
		this->shaderVS = nullptr;
		this->shaderFS = nullptr;
		this->vs       = nullptr;
		this->fs       = nullptr;
	#endif

	if (RenderEngine::SelectedGraphicsAPI == GRAPHICS_API_OPENGL)
		this->program = glCreateProgram();
}

ShaderProgram::~ShaderProgram()
{
	#if defined _WINDOWS
		_RELEASEP(this->shaderVS);
		_RELEASEP(this->shaderFS);
		_RELEASEP(this->vs);
		_RELEASEP(this->fs);
	#endif

	if (this->program > 0)
		glDeleteProgram(this->program);

	RenderEngine::Canvas.VK->DestroyShaderModule(&this->vulkanFS);
	RenderEngine::Canvas.VK->DestroyShaderModule(&this->vulkanVS);
}

#if defined _WINDOWS
ID3D11PixelShader* ShaderProgram::FragmentShader()
{
	return this->shaderFS;
}

ID3DBlob* ShaderProgram::FS()
{
	return this->fs;
}

const void* ShaderProgram::getBufferValues(const CBMatrix &matrices, Component* mesh, const DrawProperties &properties, size_t &bufferSize)
{
	const void* bufferValues = nullptr;
	Buffer*     vertexBuffer = dynamic_cast<Mesh*>(mesh)->VertexBuffer();

	if (vertexBuffer == nullptr)
		return bufferValues;

	switch (this->ID()) {
	case SHADER_ID_DEFAULT:
	case SHADER_ID_TERRAIN:
		vertexBuffer->ConstantBufferDefault = CBDefaultDX(
			matrices, mesh, properties.ClipMax, properties.ClipMin, properties.EnableClipping
		);

		bufferSize   = sizeof(vertexBuffer->ConstantBufferDefault);
		bufferValues = &vertexBuffer->ConstantBufferDefault;

		break;
	case SHADER_ID_HUD:
		vertexBuffer->ConstantBufferHUD = CBHUDDX(
			matrices, mesh->ComponentMaterial.diffuse, dynamic_cast<HUD*>(mesh->Parent)->Transparent
		);

		bufferSize   = sizeof(vertexBuffer->ConstantBufferHUD);
		bufferValues = &vertexBuffer->ConstantBufferHUD;

		break;
	case SHADER_ID_SKYBOX:
		vertexBuffer->ConstantBufferSkybox = CBSkyboxDX(matrices);

		bufferSize   = sizeof(vertexBuffer->ConstantBufferSkybox);
		bufferValues = &vertexBuffer->ConstantBufferSkybox;

		break;
	case SHADER_ID_WATER:
		vertexBuffer->ConstantBufferWater = CBWaterDX(
			matrices,
			CBDefault(mesh, properties),
			dynamic_cast<Water*>(mesh->Parent)->FBO()->MoveFactor(),
			dynamic_cast<Water*>(mesh->Parent)->FBO()->WaveStrength
		);

		bufferSize   = sizeof(vertexBuffer->ConstantBufferWater);
		bufferValues = &vertexBuffer->ConstantBufferWater;

		break;
	case SHADER_ID_COLOR:
		vertexBuffer->ConstantBufferColor = CBColorDX(matrices, mesh->ComponentMaterial.diffuse);

		bufferSize   = sizeof(vertexBuffer->ConstantBufferColor);
		bufferValues = &vertexBuffer->ConstantBufferColor;

		break;
	}

	return bufferValues;
}
#endif

ShaderID ShaderProgram::ID()
{
	if (this->name == Utils::SHADER_RESOURCES_DX[SHADER_ID_DEFAULT].Name)
		return SHADER_ID_DEFAULT;
	else if (this->name == Utils::SHADER_RESOURCES_DX[SHADER_ID_HUD].Name)
		return SHADER_ID_HUD;
	else if (this->name == Utils::SHADER_RESOURCES_DX[SHADER_ID_SKYBOX].Name)
		return SHADER_ID_SKYBOX;
	else if (this->name == Utils::SHADER_RESOURCES_DX[SHADER_ID_TERRAIN].Name)
		return SHADER_ID_TERRAIN;
	else if (this->name == Utils::SHADER_RESOURCES_DX[SHADER_ID_WATER].Name)
		return SHADER_ID_WATER;
	else if (this->name == Utils::SHADER_RESOURCES_DX[SHADER_ID_COLOR].Name)
		return SHADER_ID_COLOR;

	return SHADER_ID_UNKNOWN;
}

bool ShaderProgram::IsOK()
{
	switch (RenderEngine::SelectedGraphicsAPI) {
		#if defined _WINDOWS
		case GRAPHICS_API_DIRECTX11:
			return ((this->shaderVS != nullptr) && (this->shaderFS != nullptr));
		case GRAPHICS_API_DIRECTX12:
			return ((this->vs != nullptr) && (this->fs != nullptr));
		#endif
		case GRAPHICS_API_OPENGL:
			return (this->program > 0);
		case GRAPHICS_API_VULKAN:
			return ((this->vulkanVS != nullptr) && (this->vulkanFS != nullptr));
		default:
			return false;
	}
}

int ShaderProgram::Link()
{
	GLint resultLink;

	#if defined _DEBUG
		GLint resultValid;
	#endif

	switch (RenderEngine::SelectedGraphicsAPI) {
	case GRAPHICS_API_OPENGL:
		glLinkProgram(this->program);
		glGetProgramiv(this->program, GL_LINK_STATUS, &resultLink);

		if (resultLink != GL_TRUE)
			return -1;

		#if defined _DEBUG
			glValidateProgram(this->program);
			glGetProgramiv(this->program, GL_VALIDATE_STATUS, &resultValid);

			if (resultValid != GL_TRUE)
				return -1;
		#endif

		break;
	case GRAPHICS_API_VULKAN:
		break;
	default:
		return -1;
	}

	return 0;
}

int ShaderProgram::Load(const wxString &shaderFile)
{
	int result = -1;

	#if defined _WINDOWS
	switch (RenderEngine::SelectedGraphicsAPI) {
	case GRAPHICS_API_DIRECTX11:
		result = RenderEngine::Canvas.DX->CreateShader11(shaderFile, &this->vs, &this->fs, &this->shaderVS, &this->shaderFS);
		break;
	case GRAPHICS_API_DIRECTX12:
		result = RenderEngine::Canvas.DX->CreateShader12(shaderFile, &this->vs, &this->fs);
		break;
	default:
		return -1;
	}

	if (result < 0)
		return -1;
	#endif

	return result;
}

int ShaderProgram::LoadAndLink(const wxString &vs, const wxString &fs)
{
	switch (RenderEngine::SelectedGraphicsAPI) {
	case GRAPHICS_API_OPENGL:
		if (this->loadShaderGL(GL_VERTEX_SHADER, vs) < 0)
			return -1;

		if (this->loadShaderGL(GL_FRAGMENT_SHADER, fs) < 0)
			return -2;

		if (this->Link() < 0)
			return -3;

		this->setAttribsGL();
		this->setUniformsGL();

		break;
	case GRAPHICS_API_VULKAN:
		if (RenderEngine::Canvas.VK->CreateShaderModule(vs, "vert", &this->vulkanVS) < 0)
			return -4;

		if (RenderEngine::Canvas.VK->CreateShaderModule(fs, "frag", &this->vulkanFS) < 0)
			return -5;

		break;
	default:
		return -6;
	}

	return 0;
}

int ShaderProgram::loadShaderGL(GLuint type, const wxString &sourceText)
{
	if (((type != GL_VERTEX_SHADER) && (type != GL_FRAGMENT_SHADER)) || (RenderEngine::SelectedGraphicsAPI != GRAPHICS_API_OPENGL))
		return -1;

	GLuint shader = glCreateShader(type);

	if (shader < 1)
		return -1;

	const GLchar* sourceTextGLchar = (const GLchar*)sourceText.c_str();
	GLint         sourceTextGlint  = (const GLint)sourceText.size();
	
	glShaderSource(shader, 1, &sourceTextGLchar, &sourceTextGlint);
	glCompileShader(shader);

	GLint result;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &result);

	if (result != GL_TRUE) {
		this->Log(shader);
		return -1;
	}

	glAttachShader(this->program, shader);

	return 0;
}

void ShaderProgram::Log()
{
	#if defined _DEBUG
		std::vector<GLchar> error;
		GLint               errorLength = 0;

		glGetProgramiv(this->program, GL_INFO_LOG_LENGTH, &errorLength);

		if (errorLength > 0)
		{
			error.resize(errorLength);
			glGetProgramInfoLog(this->program, errorLength, &errorLength, &error[0]);

			wxLogDebug("%s\n", (char*)&error[0]);
		}
	#endif
}

void ShaderProgram::Log(GLuint shader)
{
	#if defined _DEBUG
		std::vector<GLchar> error;
		GLint               errorLength = 0;

		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &errorLength);

		if (errorLength > 0)
		{
			error.resize(errorLength);
			glGetShaderInfoLog(shader, errorLength, &errorLength, &error[0]);

			wxLogDebug("%s\n", (char*)&error[0]);
		}
	#endif
}

wxString ShaderProgram::Name()
{
	return this->name;
}

GLuint ShaderProgram::Program()
{
    return this->program;
}

void ShaderProgram::setAttribsGL()
{
	glUseProgram(this->program);

	// ATTRIBS (BUFFERS)
	this->Attribs[ATTRIB_NORMAL]    = glGetAttribLocation(this->program, "VertexNormal");
	this->Attribs[ATTRIB_POSITION]  = glGetAttribLocation(this->program, "VertexPosition");
	this->Attribs[ATTRIB_TEXCOORDS] = glGetAttribLocation(this->program, "VertexTextureCoords");

	glUseProgram(0);
}

void ShaderProgram::setUniformsGL()
{
	glUseProgram(this->program);

	// MATRIX BUFFER
	this->Uniforms[UBO_GL_MATRIX] = glGetUniformBlockIndex(this->program, "MatrixBuffer");
	glGenBuffers(1, &this->UniformBuffers[UBO_GL_MATRIX]);

	// COLOR BUFFER
	this->Uniforms[UBO_GL_COLOR] = glGetUniformBlockIndex(this->program, "ColorBuffer");
	glGenBuffers(1, &this->UniformBuffers[UBO_GL_COLOR]);

	// DEFAULT BUFFER
	this->Uniforms[UBO_GL_DEFAULT] = glGetUniformBlockIndex(this->program, "DefaultBuffer");
	glGenBuffers(1, &this->UniformBuffers[UBO_GL_DEFAULT]);

	// HUD BUFFER
	this->Uniforms[UBO_GL_HUD] = glGetUniformBlockIndex(this->program, "HUDBuffer");
	glGenBuffers(1, &this->UniformBuffers[UBO_GL_HUD]);

	// TERRAIN BUFFER
	this->Uniforms[UBO_GL_TERRAIN] = glGetUniformBlockIndex(this->program, "TerrainBuffer");
	glGenBuffers(1, &this->UniformBuffers[UBO_GL_TERRAIN]);

	// WATER BUFFER
	this->Uniforms[UBO_GL_WATER] = glGetUniformBlockIndex(this->program, "WaterBuffer");
	glGenBuffers(1, &this->UniformBuffers[UBO_GL_WATER]);

	// BIND TEXTURES
	for (int i = 0; i < MAX_TEXTURES; i++)
		this->Uniforms[UBO_GL_TEXTURES0 + i] = glGetUniformLocation(this->program, wxString("Textures[" + std::to_string(i) + "]").c_str());

	glUseProgram(0);
}

int ShaderProgram::UpdateAttribsGL(Component* mesh)
{
	if (mesh == nullptr)
		return -1;

	GLint id;
	Mesh* mesh2 = dynamic_cast<Mesh*>(mesh);

	if ((mesh2->NBO() > 0) && ((id = this->Attribs[ATTRIB_NORMAL]) >= 0))
		mesh2->BindBuffer(mesh2->NBO(), id, 3, GL_FLOAT, GL_FALSE);

	if ((mesh2->VBO() > 0) && ((id = this->Attribs[ATTRIB_POSITION]) >= 0))
		mesh2->BindBuffer(mesh2->VBO(), id, 3, GL_FLOAT, GL_FALSE);

	if ((mesh2->TBO() > 0) && ((id = this->Attribs[ATTRIB_TEXCOORDS]) >= 0))
		mesh2->BindBuffer(mesh2->TBO(), id, 2, GL_FLOAT, GL_FALSE);

	return 0;
}

int ShaderProgram::UpdateUniformsGL(Component* mesh, const DrawProperties &properties)
{
	if (mesh == nullptr)
		return -1;

	// MATRIX BUFFER
	GLint id = this->Uniforms[UBO_GL_MATRIX];

	if (id >= 0) {
		CBMatrix mb = CBMatrix(mesh, (this->ID() == SHADER_ID_SKYBOX));
		this->updateUniformGL(id, UBO_GL_MATRIX, &mb, sizeof(mb));
	}

	// COLOR BUFFER
	id = this->Uniforms[UBO_GL_COLOR];

	if (id >= 0) {
		CBColor cb = CBColor(dynamic_cast<Mesh*>(mesh)->GetBoundingVolume() != nullptr ? mesh->ComponentMaterial.diffuse : mesh->Parent->ComponentMaterial.diffuse);
		this->updateUniformGL(id, UBO_GL_COLOR, &cb, sizeof(cb));
	}

	// DEFAULT BUFFER
	id = this->Uniforms[UBO_GL_DEFAULT];

	if (id >= 0) {
		CBDefault db = CBDefault(mesh, properties);
		this->updateUniformGL(id, UBO_GL_DEFAULT, &db, sizeof(db));
	}

	// TERRAIN BUFFER
	id = this->Uniforms[UBO_GL_TERRAIN];

	if (id >= 0) {
		CBDefault tb = CBDefault(mesh, properties);
		this->updateUniformGL(id, UBO_GL_TERRAIN, &tb, sizeof(tb));
	}

	// HUD BUFFER
	id = this->Uniforms[UBO_GL_HUD];

	if (id >= 0) {
		CBHUD hb = CBHUD(mesh->ComponentMaterial.diffuse, dynamic_cast<HUD*>(mesh->Parent)->Transparent);
		this->updateUniformGL(id, UBO_GL_HUD, &hb, sizeof(hb));
	}

	// WATER BUFFER
	id = this->Uniforms[UBO_GL_WATER];

	if (id >= 0)
	{
		CBWater wb = CBWater(
			CBDefault(mesh, properties),
			dynamic_cast<Water*>(mesh->Parent)->FBO()->MoveFactor(),
			dynamic_cast<Water*>(mesh->Parent)->FBO()->WaveStrength
		);

		this->updateUniformGL(id, UBO_GL_WATER, &wb, sizeof(wb));
	}

    // BIND TEXTURES
    for (int i = 0; i < MAX_TEXTURES; i++)
    {
        if (mesh->Textures[i] == nullptr)
			continue;

        glUniform1i(this->Uniforms[UBO_GL_TEXTURES0 + i], i);
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(mesh->Textures[i]->Type(), mesh->Textures[i]->ID());
    }

	return 0;
}

void ShaderProgram::updateUniformGL(GLint id, UniformBufferTypeGL buffer, void* values, size_t valuesSize)
{
	glBindBufferBase(GL_UNIFORM_BUFFER, id, this->UniformBuffers[buffer]);
	glBufferData(GL_UNIFORM_BUFFER, valuesSize, values, GL_STATIC_DRAW);
	glUniformBlockBinding(this->program, id, id);
}

int ShaderProgram::UpdateUniformsVK(VkDevice deviceContext, Component* mesh, const VKUniform &uniform, const DrawProperties &properties)
{
	CBColor         cbColor;
	CBDefault       cbDefault;
	CBHUD           cbHUD;
	CBWater         cbWater;
	CBMatrix        cbMatrices  = CBMatrix(mesh, (this->ID() == SHADER_ID_SKYBOX));

	int result = ShaderProgram::updateUniformsVK(
		UBO_VK_MATRIX, UBO_BINDING_MATRIX, uniform, &cbMatrices, sizeof(cbMatrices), deviceContext, mesh
	);

	if (result < 0)
		return -1;

	switch (this->ID()) {
	case SHADER_ID_COLOR:
		cbColor = CBColor(
			dynamic_cast<Mesh*>(mesh)->GetBoundingVolume() != nullptr ? mesh->ComponentMaterial.diffuse : mesh->Parent->ComponentMaterial.diffuse
		);

		result = ShaderProgram::updateUniformsVK(
			UBO_VK_COLOR, UBO_BINDING_DEFAULT, uniform, &cbColor, sizeof(cbColor), deviceContext, mesh
		);

		break;
	case SHADER_ID_DEFAULT:
		cbDefault = CBDefault(mesh, properties);

		result = ShaderProgram::updateUniformsVK(
			UBO_VK_DEFAULT, UBO_BINDING_DEFAULT, uniform, &cbDefault, sizeof(cbDefault), deviceContext, mesh
		);

		break;
	case SHADER_ID_TERRAIN:
		cbDefault = CBDefault(mesh, properties);

		result = ShaderProgram::updateUniformsVK(
			UBO_VK_TERRAIN, UBO_BINDING_DEFAULT, uniform, &cbDefault, sizeof(cbDefault), deviceContext, mesh
		);

		break;
	case SHADER_ID_HUD:
		cbHUD = CBHUD(mesh->ComponentMaterial.diffuse, dynamic_cast<HUD*>(mesh->Parent)->Transparent);

		result = ShaderProgram::updateUniformsVK(
			UBO_VK_HUD, UBO_BINDING_DEFAULT, uniform, &cbHUD, sizeof(cbHUD), deviceContext, mesh
		);

		break;
	case SHADER_ID_WATER:
		cbWater = CBWater(
			CBDefault(mesh, properties),
			dynamic_cast<Water*>(mesh->Parent)->FBO()->MoveFactor(),
			dynamic_cast<Water*>(mesh->Parent)->FBO()->WaveStrength
		);

		result = ShaderProgram::updateUniformsVK(
			UBO_VK_WATER, UBO_BINDING_DEFAULT, uniform, &cbWater, sizeof(cbWater), deviceContext, mesh
		);

		break;
	}

	if (result < 0)
		return -2;

	result = ShaderProgram::updateUniformSamplersVK(uniform.Set, deviceContext, mesh);

	if (result < 0)
		return -3;

	return 0;
}

int ShaderProgram::updateUniformsVK(UniformBufferTypeVK type, UniformBinding binding, const VKUniform &uniform, void* values, size_t valuesSize, VkDevice deviceContext, Component* mesh)
{
	if ((deviceContext == nullptr) || (mesh == nullptr))
		return -1;

	Buffer* vertexBuffer = dynamic_cast<Mesh*>(mesh)->VertexBuffer();

	if (vertexBuffer == nullptr)
		return -2;

	void*                  bufferMemData       = nullptr;
	VkBuffer               uniformBuffer       = uniform.Buffers[type];
	VkDescriptorBufferInfo uniformBufferInfo   = {};
	VkDeviceMemory         uniformBufferMemory = uniform.BufferMemories[type];
	VkWriteDescriptorSet   uniformWriteSet     = {};

	if ((uniformBuffer == nullptr) || (uniformBufferMemory == nullptr) || (uniform.Set == nullptr))
		return -3;

	// Initialize uniform buffer descriptor set
	uniformBufferInfo.range = VK_WHOLE_SIZE;

	uniformWriteSet.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	uniformWriteSet.dstSet          = uniform.Set;
	uniformWriteSet.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uniformWriteSet.descriptorCount = 1;
	uniformWriteSet.pBufferInfo     = &uniformBufferInfo;
	//uniformWriteSet.dstArrayElement = 0;

	// Copy values to device local buffer
	VkResult result = vkMapMemory(deviceContext, uniformBufferMemory, 0, VK_WHOLE_SIZE, 0, &bufferMemData);

	if (result != VK_SUCCESS)
		return -4;

	memcpy(bufferMemData, values, valuesSize);
	vkUnmapMemory(deviceContext, uniformBufferMemory);

	// Update the descriptor set for binding 1 (default buffer)
	uniformBufferInfo.buffer   = uniformBuffer;
	uniformWriteSet.dstBinding = binding;

	vkUpdateDescriptorSets(deviceContext, 1, &uniformWriteSet, 0, nullptr);

	return 0;
}

int ShaderProgram::updateUniformSamplersVK(VkDescriptorSet uniformSet, VkDevice deviceContext, Component* mesh)
{
	if ((deviceContext == nullptr) || (mesh == nullptr))
		return -1;

	Buffer* vertexBuffer = dynamic_cast<Mesh*>(mesh)->VertexBuffer();

	if (vertexBuffer == nullptr)
		return -2;

	VkDescriptorBufferInfo uniformBufferInfo              = {};
	VkDescriptorImageInfo  uniformImageInfo[MAX_TEXTURES] = {};
	VkWriteDescriptorSet   uniformWriteSet                = {};

	if (uniformSet == nullptr)
		return -3;

	// Initialize uniform buffer descriptor set
	uniformBufferInfo.range = VK_WHOLE_SIZE;

	uniformWriteSet.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	uniformWriteSet.dstSet          = uniformSet;
	uniformWriteSet.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uniformWriteSet.descriptorCount = 1;
	uniformWriteSet.pBufferInfo     = &uniformBufferInfo;
	//uniformWriteSet.dstArrayElement = 0;

	for (int i = 0; i < MAX_TEXTURES; i++)
	{
		if ((mesh->Textures[i]->ImageView == nullptr) || (mesh->Textures[i]->Sampler == nullptr))
			return -4;

		uniformImageInfo[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		uniformImageInfo[i].imageView   = mesh->Textures[i]->ImageView;
		uniformImageInfo[i].sampler     = mesh->Textures[i]->Sampler;
	}

	// Update the descriptor set for binding 2 (texture sampler)
	uniformWriteSet.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	uniformWriteSet.descriptorCount = MAX_TEXTURES;
	uniformWriteSet.dstBinding      = UBO_BINDING_TEXTURES;
	uniformWriteSet.pBufferInfo     = nullptr;
	uniformWriteSet.pImageInfo      = uniformImageInfo;
	//uniformWriteSet.pTexelBufferView = nullptr;

	vkUpdateDescriptorSets(deviceContext, 1, &uniformWriteSet, 0, nullptr);

	return 0;
}

#if defined _WINDOWS
int ShaderProgram::UpdateUniformsDX11(ID3D11Buffer** constBuffer, const void** constBufferValues, Component* mesh, const DrawProperties &properties)
{
	if (mesh == nullptr)
		return -1;

	Buffer* vertexBuffer = dynamic_cast<Mesh*>(mesh)->VertexBuffer();

	if (vertexBuffer == nullptr)
		return -2;

	size_t   bufferSize = 0;
	CBMatrix matrices   = CBMatrix(mesh, (this->ID() == SHADER_ID_SKYBOX));

	*constBufferValues = ShaderProgram::getBufferValues(matrices, mesh, properties, bufferSize);
	*constBuffer       = vertexBuffer->ConstantBuffersDX11[this->ID()];
	
	if ((*constBuffer == nullptr) || (*constBufferValues == nullptr))
		return -3;

	return 0;
}

int ShaderProgram::UpdateUniformsDX12(Component* mesh, const DrawProperties &properties)
{
	if (mesh == nullptr)
		return -1;

	Buffer* vertexBuffer = dynamic_cast<Mesh*>(mesh)->VertexBuffer();

	if (vertexBuffer == nullptr)
		return -2;

	CBMatrix        matrices          = CBMatrix(mesh, (this->ID() == SHADER_ID_SKYBOX));
	size_t          constBufferSize   = 0;
	const void*     constBufferValues = ShaderProgram::getBufferValues(matrices, mesh, properties, constBufferSize);
	ID3D12Resource* constBuffer       = vertexBuffer->ConstantBuffersDX12[this->ID()];

	if ((constBuffer == nullptr) || (constBufferValues == nullptr) || (constBufferSize == 0))
		return -3;

	CD3DX12_RANGE readRange(0, 0);
	uint8_t*      bufferData = nullptr;
	HRESULT       result     = constBuffer->Map(0, &readRange, reinterpret_cast<void**>(&bufferData));

	if (FAILED(result))
		return -4;

	std::memcpy(bufferData, constBufferValues, constBufferSize);
	constBuffer->Unmap(0, nullptr);

	return 0;
}

ID3D11VertexShader* ShaderProgram::VertexShader()
{
	return this->shaderVS;
}

ID3DBlob* ShaderProgram::VS()
{
	return this->vs;
}
#endif

VkShaderModule ShaderProgram::VulkanFS()
{
	return this->vulkanFS;
}

VkShaderModule ShaderProgram::VulkanVS()
{
	return this->vulkanVS;
}
