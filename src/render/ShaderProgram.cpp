#include "ShaderProgram.h"

ShaderProgram::ShaderProgram(const wxString &name)
{
	this->name          = name;
	this->program       = 0;
	this->vulkanFS      = nullptr;
	this->vulkanGS      = nullptr;
	this->vulkanVS      = nullptr;

	#if defined _WINDOWS
		this->shaderVS = nullptr;
		this->shaderGS = nullptr;
		this->shaderFS = nullptr;
		this->vs       = nullptr;
		this->gs       = nullptr;
		this->fs       = nullptr;
	#endif

	if (RenderEngine::SelectedGraphicsAPI == GRAPHICS_API_OPENGL)
		this->program = glCreateProgram();
}

ShaderProgram::~ShaderProgram()
{
	#if defined _WINDOWS
		_RELEASEP(this->shaderVS);
		_RELEASEP(this->shaderGS);
		_RELEASEP(this->shaderFS);
		_RELEASEP(this->vs);
		_RELEASEP(this->gs);
		_RELEASEP(this->fs);
	#endif

	if (this->program > 0)
		glDeleteProgram(this->program);

	RenderEngine::Canvas.VK->DestroyShaderModule(&this->vulkanFS);
	RenderEngine::Canvas.VK->DestroyShaderModule(&this->vulkanGS);
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

ID3D11GeometryShader* ShaderProgram::GeometryShader()
{
	return this->shaderGS;
}

ID3DBlob* ShaderProgram::GS()
{
	return this->gs;
}

const void* ShaderProgram::getBufferValues(const CBMatrix &matrices, Component* mesh, const DrawProperties &properties, size_t &bufferSize)
{
	const void* bufferValues = nullptr;
	Buffer*     vertexBuffer = dynamic_cast<Mesh*>(mesh)->VertexBuffer();

	if (vertexBuffer == nullptr)
		return bufferValues;

	switch (this->ID()) {
	case SHADER_ID_COLOR:
	case SHADER_ID_WIREFRAME:
		vertexBuffer->ConstantBufferColor = CBColorDX(matrices, mesh->ComponentMaterial.diffuse);

		bufferSize   = sizeof(vertexBuffer->ConstantBufferColor);
		bufferValues = &vertexBuffer->ConstantBufferColor;

		break;
	case SHADER_ID_DEFAULT:
		vertexBuffer->ConstantBufferDefault = CBDefaultDX(
			matrices, mesh, properties.ClipMax, properties.ClipMin, properties.EnableClipping
		);

		bufferSize   = sizeof(vertexBuffer->ConstantBufferDefault);
		bufferValues = &vertexBuffer->ConstantBufferDefault;

		break;
	case SHADER_ID_DEPTH_OMNI:
		vertexBuffer->ConstantBufferDepth = CBDepthDX(
			matrices, properties.Light->GetLight().position, -1
		);

		bufferSize   = sizeof(vertexBuffer->ConstantBufferDepth);
		bufferValues = &vertexBuffer->ConstantBufferDepth;

		break;
	case SHADER_ID_HUD:
		vertexBuffer->ConstantBufferHUD = CBHUDDX(
			matrices, mesh->ComponentMaterial.diffuse, dynamic_cast<HUD*>(mesh->Parent)->Transparent
		);

		bufferSize   = sizeof(vertexBuffer->ConstantBufferHUD);
		bufferValues = &vertexBuffer->ConstantBufferHUD;

		break;
	default:
		vertexBuffer->ConstantBufferSkybox = CBSkyboxDX(matrices);

		bufferSize   = sizeof(vertexBuffer->ConstantBufferSkybox);
		bufferValues = &vertexBuffer->ConstantBufferSkybox;

		break;
	}

	return bufferValues;
}
#endif

ShaderID ShaderProgram::ID()
{
	if (this->name == Utils::SHADER_RESOURCES_DX[SHADER_ID_COLOR].Name)
		return SHADER_ID_COLOR;
	else if (this->name == Utils::SHADER_RESOURCES_DX[SHADER_ID_DEFAULT].Name)
		return SHADER_ID_DEFAULT;
	else if (this->name == Utils::SHADER_RESOURCES_DX[SHADER_ID_DEPTH].Name)
		return SHADER_ID_DEPTH;
	else if (this->name == Utils::SHADER_RESOURCES_DX[SHADER_ID_DEPTH_OMNI].Name)
		return SHADER_ID_DEPTH_OMNI;
	else if (this->name == Utils::SHADER_RESOURCES_DX[SHADER_ID_HUD].Name)
		return SHADER_ID_HUD;
	else if (this->name == Utils::SHADER_RESOURCES_DX[SHADER_ID_SKYBOX].Name)
		return SHADER_ID_SKYBOX;
	else if (this->name == Utils::SHADER_RESOURCES_DX[SHADER_ID_WIREFRAME].Name)
		return SHADER_ID_WIREFRAME;

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
			throw;
	}
}

int ShaderProgram::Link()
{
	GLint resultLink;

	switch (RenderEngine::SelectedGraphicsAPI) {
	case GRAPHICS_API_OPENGL:
		glLinkProgram(this->program);
		glGetProgramiv(this->program, GL_LINK_STATUS, &resultLink);

		if (resultLink != GL_TRUE)
			return -1;

		break;
	case GRAPHICS_API_VULKAN:
		break;
	default:
		throw;
	}

	return 0;
}

int ShaderProgram::Load(const wxString &shaderFile)
{
	int result = -1;

	#if defined _WINDOWS
	switch (RenderEngine::SelectedGraphicsAPI) {
	case GRAPHICS_API_DIRECTX11:
		result = RenderEngine::Canvas.DX->CreateShader11(shaderFile, &this->vs, &this->gs, &this->fs, &this->shaderVS, &this->shaderGS, &this->shaderFS);
		break;
	case GRAPHICS_API_DIRECTX12:
		result = RenderEngine::Canvas.DX->CreateShader12(shaderFile, &this->vs, &this->gs, &this->fs);
		break;
	default:
		throw;
	}

	if (result < 0)
		return -1;
	#endif

	return result;
}

int ShaderProgram::LoadAndLink(const wxString &vs, const wxString &fs, const wxString& gs)
{
	switch (RenderEngine::SelectedGraphicsAPI) {
	case GRAPHICS_API_OPENGL:
		if (this->loadShaderGL(GL_VERTEX_SHADER, vs) < 0)
			return -1;

		if (!gs.empty() && (this->loadShaderGL(GL_GEOMETRY_SHADER, gs) < 0))
			return -2;

		if (this->loadShaderGL(GL_FRAGMENT_SHADER, fs) < 0)
			return -3;

		if (this->Link() < 0)
			return -4;

		this->setAttribsGL();
		this->setUniformsGL();

		break;
	case GRAPHICS_API_VULKAN:
		if (RenderEngine::Canvas.VK->CreateShaderModule(vs, "vert", &this->vulkanVS) < 0)
			return -10;

		if (!gs.empty() && (RenderEngine::Canvas.VK->CreateShaderModule(gs, "geom", &this->vulkanGS) < 0))
			return -11;

		if (RenderEngine::Canvas.VK->CreateShaderModule(fs, "frag", &this->vulkanFS) < 0)
			return -12;

		break;
	default:
		throw;
	}

	return 0;
}

int ShaderProgram::loadShaderGL(GLuint type, const wxString &sourceText)
{
	if (RenderEngine::SelectedGraphicsAPI != GRAPHICS_API_OPENGL)
		return -1;

	if ((type != GL_VERTEX_SHADER) && (type != GL_FRAGMENT_SHADER) && (type != GL_GEOMETRY_SHADER))
		return -2;

	GLuint shader = glCreateShader(type);

	if (shader < 1)
		return -3;

	const GLchar* sourceTextGLchar = (const GLchar*)sourceText.c_str();
	GLint         sourceTextGlint  = (const GLint)sourceText.size();
	
	glShaderSource(shader, 1, &sourceTextGLchar, &sourceTextGlint);
	glCompileShader(shader);

	GLint result;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &result);

	if (result != GL_TRUE) {
		this->Log(shader);
		return -4;
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

	// DEPTH BUFFER
	this->Uniforms[UBO_GL_DEPTH] = glGetUniformBlockIndex(this->program, "DepthBuffer");
	glGenBuffers(1, &this->UniformBuffers[UBO_GL_DEPTH]);

	// HUD BUFFER
	this->Uniforms[UBO_GL_HUD] = glGetUniformBlockIndex(this->program, "HUDBuffer");
	glGenBuffers(1, &this->UniformBuffers[UBO_GL_HUD]);

	// MESH TEXTURES
	for (int i = 0; i < MAX_TEXTURES; i++)
		this->Uniforms[UBO_GL_TEXTURES0 + i] = glGetUniformLocation(this->program, wxString("Textures[" + std::to_string(i) + "]").c_str());
	
	// DEPTH MAP 2D TEXTURES
	this->Uniforms[UBO_GL_TEXTURES6] = glGetUniformLocation(this->program, wxString("DepthMapTextures2D").c_str());

	// DEPTH MAP CUBE TEXTURES
	this->Uniforms[UBO_GL_TEXTURES7] = glGetUniformLocation(this->program, wxString("DepthMapTexturesCube").c_str());

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

	ShaderID shaderID = this->ID();

	// MATRIX BUFFER
	GLint id = this->Uniforms[UBO_GL_MATRIX];

	if (id >= 0)
	{
		CBMatrix mb;

		if ((shaderID == SHADER_ID_DEPTH) || (shaderID == SHADER_ID_DEPTH_OMNI))
			mb = CBMatrix(properties.Light, mesh);
		else
			mb = CBMatrix(mesh, (shaderID == SHADER_ID_SKYBOX));

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

	// DEPTH BUFFER
	id = this->Uniforms[UBO_GL_DEPTH];

	if (id >= 0) {
		CBDepth db = CBDepth(properties.Light->GetLight().position, properties.DepthLayer);
		this->updateUniformGL(id, UBO_GL_DEPTH, &db, sizeof(db));
	}

	// HUD BUFFER
	id = this->Uniforms[UBO_GL_HUD];

	if (id >= 0) {
		CBHUD hb = CBHUD(mesh->ComponentMaterial.diffuse, dynamic_cast<HUD*>(mesh->Parent)->Transparent);
		this->updateUniformGL(id, UBO_GL_HUD, &hb, sizeof(hb));
	}

    // BIND MESH TEXTURES - Texture slots: [GL_TEXTURE0, GL_TEXTURE5]
	for (int i = 0; i < MAX_TEXTURES; i++)
	{
		id = this->Uniforms[UBO_GL_TEXTURES0 + i];

		if (id >= 0) {
			glActiveTexture(GL_TEXTURE0 + i);
			glUniform1i(id, i);
			glBindTexture(mesh->Textures[i]->TypeGL(), mesh->Textures[i]->ID());
		} else {
			glBindTexture(GL_TEXTURE0 + i, 0);
		}
	}

	// BIND DEPTH MAP - 2D TEXTURE ARRAY
	id = this->Uniforms[UBO_GL_TEXTURES6];

	if ((id >= 0) && (SceneManager::DepthMap2D != nullptr)) {
		glActiveTexture(GL_TEXTURE6);
		glUniform1i(this->Uniforms[UBO_GL_TEXTURES6], 6);
		glBindTexture(GL_TEXTURE_2D_ARRAY, SceneManager::DepthMap2D->GetTexture()->ID());
	} else {
		glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
	}

	// BIND DEPTH MAP - CUBE MAP ARRAY
	id = this->Uniforms[UBO_GL_TEXTURES7];

	if ((id >= 0) && (SceneManager::DepthMapCube != nullptr)) {
		glActiveTexture(GL_TEXTURE7);
		glUniform1i(this->Uniforms[UBO_GL_TEXTURES7], 7);
		glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, SceneManager::DepthMapCube->GetTexture()->ID());
	} else {
		glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, 0);
	}

	#if defined _DEBUG
		glValidateProgram(this->program);

		GLint resultValid;
		glGetProgramiv(this->program, GL_VALIDATE_STATUS, &resultValid);

		if (resultValid != GL_TRUE)
			return -2;
	#endif

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
	ShaderID shaderID = this->ID();

	// MATRIX UNIFORM BUFFER
	CBMatrix cbMatrices;

	if ((shaderID == SHADER_ID_DEPTH) || (shaderID == SHADER_ID_DEPTH_OMNI))
		cbMatrices = CBMatrix(properties.Light, mesh);
	else
		cbMatrices = CBMatrix(mesh, (shaderID == SHADER_ID_SKYBOX));

	int result = ShaderProgram::updateUniformsVK(
		UBO_VK_MATRIX, UBO_BINDING_MATRIX, uniform, &cbMatrices, sizeof(cbMatrices), deviceContext, mesh
	);

	if (result < 0)
		return -1;

	// UNIFORM BUFFERS
	CBColor   cbColor;
	CBDefault cbDefault;
	CBDepth   cbDepth;
	CBHUD     cbHUD;

	switch (this->ID()) {
	case SHADER_ID_COLOR:
	case SHADER_ID_WIREFRAME:
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
	case SHADER_ID_DEPTH_OMNI:
		cbDepth = CBDepth(properties.Light->GetLight().position, -1);

		result = ShaderProgram::updateUniformsVK(
			UBO_VK_DEPTH, UBO_BINDING_DEFAULT, uniform, &cbDepth, sizeof(cbDepth), deviceContext, mesh
		);

		break;
	case SHADER_ID_HUD:
		cbHUD = CBHUD(mesh->ComponentMaterial.diffuse, dynamic_cast<HUD*>(mesh->Parent)->Transparent);

		result = ShaderProgram::updateUniformsVK(
			UBO_VK_HUD, UBO_BINDING_DEFAULT, uniform, &cbHUD, sizeof(cbHUD), deviceContext, mesh
		);

		break;
	default:
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

	void*                  bufferMemData       = nullptr;
	VkBuffer               uniformBuffer       = uniform.Buffers[type];
	VkDescriptorBufferInfo uniformBufferInfo   = {};
	VkDeviceMemory         uniformBufferMemory = uniform.BufferMemories[type];
	VkWriteDescriptorSet   uniformWriteSet     = {};

	if ((uniformBuffer == nullptr) || (uniformBufferMemory == nullptr) || (uniform.Set == nullptr))
		return -2;

	// Initialize uniform buffer descriptor set
	uniformBufferInfo.range = VK_WHOLE_SIZE;

	uniformWriteSet.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	uniformWriteSet.dstSet          = uniform.Set;
	uniformWriteSet.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uniformWriteSet.descriptorCount = 1;
	uniformWriteSet.pBufferInfo     = &uniformBufferInfo;

	// Copy values to device local buffer
	VkResult result = vkMapMemory(deviceContext, uniformBufferMemory, 0, VK_WHOLE_SIZE, 0, &bufferMemData);

	if (result != VK_SUCCESS)
		return -3;

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
	if ((deviceContext == nullptr) || (mesh == nullptr) || (uniformSet == nullptr))
		return -1;

	ShaderID              shaderID                         = this->ID();
	VkDescriptorImageInfo uniformTextureInfo[MAX_TEXTURES] = {};
	VkWriteDescriptorSet  uniformWriteSet                  = {};

	uniformWriteSet.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	uniformWriteSet.dstSet          = uniformSet;
	uniformWriteSet.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uniformWriteSet.descriptorCount = 1;

	// BIND MESH TEXTURES
	switch (shaderID) {
	case SHADER_ID_DEFAULT:
	case SHADER_ID_HUD:
	case SHADER_ID_SKYBOX:
		for (int i = 0; i < MAX_TEXTURES; i++)
		{
			uniformTextureInfo[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			uniformTextureInfo[i].imageView   = mesh->Textures[i]->ImageView;
			uniformTextureInfo[i].sampler     = mesh->Textures[i]->Sampler;
		}

		// BIND 2D TEXTURE
		uniformWriteSet.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		uniformWriteSet.descriptorCount = MAX_TEXTURES;
		uniformWriteSet.dstBinding      = UBO_BINDING_TEXTURES;
		uniformWriteSet.pImageInfo      = uniformTextureInfo;

		vkUpdateDescriptorSets(deviceContext, 1, &uniformWriteSet, 0, nullptr);

		break;
	default:
		break;
	}

	// BIND DEPTH MAP TEXTURES
	if (shaderID == SHADER_ID_DEFAULT)
	{
		VkDescriptorImageInfo uniformDepthInfo = {};

		// BIND DEPTH MAP - 2D TEXTURE ARRAY
		uniformDepthInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		uniformDepthInfo.imageView   = SceneManager::DepthMap2D->GetTexture()->ImageView;
		uniformDepthInfo.sampler     = SceneManager::DepthMap2D->GetTexture()->Sampler;

		uniformWriteSet.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		uniformWriteSet.descriptorCount = 1;
		uniformWriteSet.dstBinding      = UBO_BINDING_DEPTH_2D;
		uniformWriteSet.pImageInfo      = &uniformDepthInfo;

		vkUpdateDescriptorSets(deviceContext, 1, &uniformWriteSet, 0, nullptr);

		// BIND DEPTH MAP - CUBE MAP ARRAY
		uniformDepthInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		uniformDepthInfo.imageView   = SceneManager::DepthMapCube->GetTexture()->ImageView;
		uniformDepthInfo.sampler     = SceneManager::DepthMapCube->GetTexture()->Sampler;

		uniformWriteSet.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		uniformWriteSet.descriptorCount = 1;
		uniformWriteSet.dstBinding      = UBO_BINDING_DEPTH_CUBEMAPS;
		uniformWriteSet.pImageInfo      = &uniformDepthInfo;

		vkUpdateDescriptorSets(deviceContext, 1, &uniformWriteSet, 0, nullptr);
	}

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

	// MATRIX UNIFORM BUFFER
	CBMatrix matrices;

	if ((this->ID() == SHADER_ID_DEPTH) || (this->ID() == SHADER_ID_DEPTH_OMNI))
		matrices = CBMatrix(properties.Light, mesh);
	else
		matrices = CBMatrix(mesh, (this->ID() == SHADER_ID_SKYBOX));

	// UNIFORM BUFFERS
	size_t bufferSize = 0;

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

	// MATRIX UNIFORM BUFFER
	CBMatrix matrices;

	if ((this->ID() == SHADER_ID_DEPTH) || (this->ID() == SHADER_ID_DEPTH_OMNI))
		matrices = CBMatrix(properties.Light, mesh);
	else
		matrices = CBMatrix(mesh, (this->ID() == SHADER_ID_SKYBOX));

	// UNIFORM BUFFERS
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

VkShaderModule ShaderProgram::VulkanGS()
{
	return this->vulkanGS;
}

VkShaderModule ShaderProgram::VulkanVS()
{
	return this->vulkanVS;
}
