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

	if (Utils::SelectedGraphicsAPI == GRAPHICS_API_OPENGL)
		this->program = glCreateProgram();
}

ShaderProgram::~ShaderProgram()
{
	//switch (Utils::SelectedGraphicsAPI) {
	#if defined _WINDOWS
	//case GRAPHICS_API_DIRECTX11:
	//case GRAPHICS_API_DIRECTX12:
		_RELEASEP(this->shaderVS);
		_RELEASEP(this->shaderFS);
		_RELEASEP(this->vs);
		_RELEASEP(this->fs);
		//break;
	#endif
	//case GRAPHICS_API_OPENGL:
	if (this->program > 0)
		glDeleteProgram(this->program);
		//break;
	//case GRAPHICS_API_VULKAN:
	RenderEngine::Canvas.VK->DestroyShaderModule(&this->vulkanFS);
	RenderEngine::Canvas.VK->DestroyShaderModule(&this->vulkanVS);
	//RenderEngine::Canvas.Vulkan->DestroyPipeline(this->pipeline);
		//break;
	//default:
		//break;
	//}
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

DXLightBuffer ShaderProgram::getBufferLight()
{
	DXLightBuffer light = {};

	light.Color      = Utils::ToXMFLOAT4(SceneManager::SunLight.Color);
	light.Direction  = Utils::ToXMFLOAT3(SceneManager::SunLight.Direction);
	light.Position   = Utils::ToXMFLOAT3(SceneManager::SunLight.Position);
	light.Reflection = SceneManager::SunLight.Reflection;
	light.Shine      = SceneManager::SunLight.Shine;

	return light;
}

DXMatrixBuffer ShaderProgram::getBufferMatrices(Mesh* mesh, bool removeTranslation)
{
	DXMatrixBuffer matrices = {};

	matrices.Model      = Utils::ToXMMATRIX(mesh->Matrix());
	matrices.View       = Utils::ToXMMATRIX(RenderEngine::Camera->View(removeTranslation));
	matrices.Projection = Utils::ToXMMATRIX(RenderEngine::Camera->Projection());
	matrices.MVP        = Utils::ToXMMATRIX(RenderEngine::Camera->MVP(mesh->Matrix(), removeTranslation));

	return matrices;
}

const void* ShaderProgram::getBufferValues(const DXMatrixBuffer &matrices, const DXLightBuffer &sunLight, Mesh* mesh, bool enableClipping, const glm::vec3 &clipMax, const glm::vec3 &clipMin)
{
	const void* bufferValues = nullptr;
	Buffer*     vertexBuffer = mesh->VertexBuffer();

	if (vertexBuffer == nullptr)
		return bufferValues;

	switch (this->ID()) {
	case SHADER_ID_DEFAULT:
		vertexBuffer->DefaultBufferValues.Matrices       = matrices;
		vertexBuffer->DefaultBufferValues.SunLight       = sunLight;
		vertexBuffer->DefaultBufferValues.Ambient        = Utils::ToXMFLOAT3(SceneManager::AmbientLightIntensity);
		vertexBuffer->DefaultBufferValues.EnableClipping = enableClipping;
		vertexBuffer->DefaultBufferValues.ClipMax        = Utils::ToXMFLOAT3(clipMax);
		vertexBuffer->DefaultBufferValues.ClipMin        = Utils::ToXMFLOAT3(clipMin);
		vertexBuffer->DefaultBufferValues.IsTextured     = mesh->IsTextured();
		vertexBuffer->DefaultBufferValues.MaterialColor  = Utils::ToXMFLOAT4(mesh->Color);

		for (int i = 0; i < MAX_TEXTURES; i++)
			vertexBuffer->DefaultBufferValues.TextureScales[i] = Utils::ToXMFLOAT2(mesh->Textures[i]->Scale);

		bufferValues = &vertexBuffer->DefaultBufferValues;

		break;
	case SHADER_ID_HUD:
		vertexBuffer->HUDBufferValues.Matrices       = matrices;
		vertexBuffer->HUDBufferValues.MaterialColor  = Utils::ToXMFLOAT4(mesh->Color);
		vertexBuffer->HUDBufferValues.IsTransparent = dynamic_cast<HUD*>(mesh->Parent)->Transparent;

		bufferValues = &vertexBuffer->HUDBufferValues;

		break;
	case SHADER_ID_SKYBOX:
		vertexBuffer->SkyboxBufferValues.Matrices = matrices;

		for (int i = 0; i < MAX_TEXTURES; i++)
			vertexBuffer->SkyboxBufferValues.TextureScales[i] = Utils::ToXMFLOAT2(mesh->Textures[i]->Scale);

		bufferValues = &vertexBuffer->SkyboxBufferValues;

		break;
	case SHADER_ID_SOLID:
		vertexBuffer->SolidBufferValues.Matrices   = matrices;
		vertexBuffer->SolidBufferValues.SolidColor = Utils::ToXMFLOAT4(SceneManager::SelectColor);

		bufferValues = &vertexBuffer->SolidBufferValues;

		break;
	case SHADER_ID_TERRAIN:
		vertexBuffer->TerrainBufferValues.Matrices       = matrices;
		vertexBuffer->TerrainBufferValues.SunLight       = sunLight;
		vertexBuffer->TerrainBufferValues.Ambient        = Utils::ToXMFLOAT3(SceneManager::AmbientLightIntensity);
		vertexBuffer->TerrainBufferValues.EnableClipping = enableClipping;
		vertexBuffer->TerrainBufferValues.ClipMax        = Utils::ToXMFLOAT3(clipMax);
		vertexBuffer->TerrainBufferValues.ClipMin        = Utils::ToXMFLOAT3(clipMin);

		for (int i = 0; i < MAX_TEXTURES; i++)
			vertexBuffer->TerrainBufferValues.TextureScales[i] = Utils::ToXMFLOAT2(mesh->Textures[i]->Scale);

		bufferValues = &vertexBuffer->TerrainBufferValues;

		break;
	case SHADER_ID_WATER:
		vertexBuffer->WaterBufferValues.CameraMain.Position = Utils::ToXMFLOAT3(RenderEngine::Camera->Position());
		vertexBuffer->WaterBufferValues.CameraMain.Near     = RenderEngine::Camera->Near();
		vertexBuffer->WaterBufferValues.CameraMain.Far      = RenderEngine::Camera->Far();
		vertexBuffer->WaterBufferValues.Matrices            = matrices;
		vertexBuffer->WaterBufferValues.SunLight            = sunLight;
		vertexBuffer->WaterBufferValues.EnableClipping      = enableClipping;
		vertexBuffer->WaterBufferValues.ClipMax             = Utils::ToXMFLOAT3(clipMax);
		vertexBuffer->WaterBufferValues.ClipMin             = Utils::ToXMFLOAT3(clipMin);
		vertexBuffer->WaterBufferValues.MoveFactor          = dynamic_cast<Water*>(mesh->Parent)->FBO()->MoveFactor();
		vertexBuffer->WaterBufferValues.WaveStrength        = dynamic_cast<Water*>(mesh->Parent)->FBO()->WaveStrength;

		for (int i = 0; i < MAX_TEXTURES; i++)
			vertexBuffer->WaterBufferValues.TextureScales[i] = Utils::ToXMFLOAT2(mesh->Textures[i]->Scale);

		bufferValues = &vertexBuffer->WaterBufferValues;

		break;
	}

	return bufferValues;
}

ShaderID ShaderProgram::ID()
{
	if (this->name == Utils::SHADER_RESOURCES_DX[SHADER_ID_DEFAULT].Name)
		return SHADER_ID_DEFAULT;
	else if (this->name == Utils::SHADER_RESOURCES_DX[SHADER_ID_HUD].Name)
		return SHADER_ID_HUD;
	else if (this->name == Utils::SHADER_RESOURCES_DX[SHADER_ID_SKYBOX].Name)
		return SHADER_ID_SKYBOX;
	else if (this->name == Utils::SHADER_RESOURCES_DX[SHADER_ID_SOLID].Name)
		return SHADER_ID_SOLID;
	else if (this->name == Utils::SHADER_RESOURCES_DX[SHADER_ID_TERRAIN].Name)
		return SHADER_ID_TERRAIN;
	else if (this->name == Utils::SHADER_RESOURCES_DX[SHADER_ID_WATER].Name)
		return SHADER_ID_WATER;

	return SHADER_ID_UNKNOWN;
}
#endif

bool ShaderProgram::IsOK()
{
	switch (Utils::SelectedGraphicsAPI) {
		#if defined _WINDOWS
		case GRAPHICS_API_DIRECTX11: return ((this->shaderVS != nullptr) && (this->shaderFS != nullptr));
		case GRAPHICS_API_DIRECTX12: return ((this->vs != nullptr) && (this->fs != nullptr));
		#endif
		case GRAPHICS_API_OPENGL:    return (this->program > 0);
		case GRAPHICS_API_VULKAN:    return ((this->vulkanVS != nullptr) && (this->vulkanFS != nullptr));
		default:                     return false;
	}
}

int ShaderProgram::Link()
{
	GLint resultLink;

	#if defined _DEBUG
		GLint resultValid;
	#endif

	switch (Utils::SelectedGraphicsAPI) {
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
	switch (Utils::SelectedGraphicsAPI) {
	case GRAPHICS_API_DIRECTX11:
		result = RenderEngine::Canvas.DX->CreateShader11(shaderFile, &this->vs, &this->fs, &this->shaderVS, &this->shaderFS);
		break;
	case GRAPHICS_API_DIRECTX12:
		result = RenderEngine::Canvas.DX->CreateShader12(shaderFile, &this->vs, &this->fs);
		break;
	default:
		return -1;
	}

	if (result != 0)
		return -1;
	#endif

	return result;
}

int ShaderProgram::LoadAndLink(const wxString &vs, const wxString &fs)
{
	int result = -1;

	switch (Utils::SelectedGraphicsAPI) {
	case GRAPHICS_API_OPENGL:
		if (this->loadShaderGL(GL_VERTEX_SHADER, vs) != 0)
			return -2;

		if (this->loadShaderGL(GL_FRAGMENT_SHADER, fs) != 0)
			return -3;

		if (this->Link() != 0)
			return -4;

		this->setAttribsGL();
		this->setUniformsGL();

		break;
	case GRAPHICS_API_VULKAN:
		if (RenderEngine::Canvas.VK->CreateShaderModule(vs, wxT("vert"), &this->vulkanVS) != 0)
			return -5;

		if (RenderEngine::Canvas.VK->CreateShaderModule(fs, wxT("frag"), &this->vulkanFS) != 0)
			return -6;

		break;
	default:
		return -7;
	}

	return 0;
}

int ShaderProgram::loadShaderGL(GLuint type, const wxString &sourceText)
{
	if (((type != GL_VERTEX_SHADER) && (type != GL_FRAGMENT_SHADER)) || (Utils::SelectedGraphicsAPI != GRAPHICS_API_OPENGL))
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

			//wxMessageBox((char*)&error[0]);
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

			//wxMessageBox((char*)&error[0]);
			wxLogDebug("%s\n", (char*)&error[0]);
		}
	#endif
}

wxString ShaderProgram::Name()
{
	return this->name;
}

//VkPipeline ShaderProgram::Pipeline()
//{
//	return this->pipeline;
//}

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
	this->Uniforms[MATRIX_BUFFER] = glGetUniformBlockIndex(this->program, "MatrixBuffer");
	glGenBuffers(1, &this->UniformBuffers[MATRIX_BUFFER]);

	// DEFAULT BUFFER
	this->Uniforms[DEFAULT_BUFFER] = glGetUniformBlockIndex(this->program, "DefaultBuffer");
	glGenBuffers(1, &this->UniformBuffers[DEFAULT_BUFFER]);

	// MATRICES
	//this->Uniforms[MATRIX_MODEL]      = glGetUniformLocation(this->program, "MatrixModel");
	//this->Uniforms[MATRIX_VIEW]       = glGetUniformLocation(this->program, "MatrixView");
	//this->Uniforms[MATRIX_PROJECTION] = glGetUniformLocation(this->program, "MatrixProjection");
	//this->Uniforms[MATRIX_MVP]        = glGetUniformLocation(this->program, "MatrixMVP");

	// CAMERA
	this->Uniforms[CAMERA_POSITION] = glGetUniformLocation(this->program, "CameraMain.Position");
	this->Uniforms[CAMERA_NEAR]     = glGetUniformLocation(this->program, "CameraMain.Near");
	this->Uniforms[CAMERA_FAR]      = glGetUniformLocation(this->program, "CameraMain.Far");

	// CLIPPING
	//this->Uniforms[ENABLE_CLIPPING] = glGetUniformLocation(this->program, "EnableClipping");
	//this->Uniforms[CLIP_MAX]        = glGetUniformLocation(this->program, "ClipMax");
	//this->Uniforms[CLIP_MIN]        = glGetUniformLocation(this->program, "ClipMin");

	// AMBIENT LIGHT
	//this->Uniforms[AMBIENT_LIGHT_INTENSITY] = glGetUniformLocation(this->program, "AmbientLightIntensity");
	//this->Uniforms[AMBIENT_LIGHT_INTENSITY] = glGetUniformLocation(this->program, "Ambient");

	// DIRECIONAL LIGHT
	//this->Uniforms[SUNLIGHT_COLOR]      = glGetUniformLocation(this->program, "SunLight.Color");
	//this->Uniforms[SUNLIGHT_DIRECTION]  = glGetUniformLocation(this->program, "SunLight.Direction");
	//this->Uniforms[SUNLIGHT_POSITION]   = glGetUniformLocation(this->program, "SunLight.Position");
	//this->Uniforms[SUNLIGHT_REFLECTION] = glGetUniformLocation(this->program, "SunLight.Reflection");
	//this->Uniforms[SUNLIGHT_SHINE]      = glGetUniformLocation(this->program, "SunLight.Shine");
        
	// MATERIAL COLOR
	//this->Uniforms[MATERIAL_COLOR] = glGetUniformLocation(this->program, "MaterialColor");
	this->Uniforms[SOLID_COLOR]    = glGetUniformLocation(this->program, "SolidColor");
	//this->Uniforms[IS_TEXTURED]    = glGetUniformLocation(this->program, "IsTextured");

	// WATER
	this->Uniforms[MOVE_FACTOR]   = glGetUniformLocation(this->program, "MoveFactor");
	this->Uniforms[WAVE_STRENGTH] = glGetUniformLocation(this->program, "WaveStrength");

	// HUD
	this->Uniforms[IS_TRANSPARENT] = glGetUniformLocation(this->program, "IsTransparent");

	// BIND TEXTURES
	for (int i = 0; i < MAX_TEXTURES; i++) {
		this->Uniforms[TEXTURES0       + i] = glGetUniformLocation(this->program, wxString("Textures[" + std::to_string(i) + "]").c_str());
		//this->Uniforms[TEXTURE_SCALES0 + i] = glGetUniformLocation(this->program, wxString("TextureScales[" + std::to_string(i) + "]").c_str());
	}

	glUseProgram(0);
}

int ShaderProgram::UpdateAttribsGL(Mesh* mesh)
{
	if (mesh == nullptr)
		return -1;

	GLint id;

	if ((mesh->NBO() > 0) && ((id = this->Attribs[ATTRIB_NORMAL]) >= 0))
		mesh->BindBuffer(mesh->NBO(), id, 3, GL_FLOAT, GL_FALSE);

	if ((mesh->VBO() > 0) && ((id = this->Attribs[ATTRIB_POSITION]) >= 0))
		mesh->BindBuffer(mesh->VBO(), id, 3, GL_FLOAT, GL_FALSE);

	if ((mesh->TBO() > 0) && ((id = this->Attribs[ATTRIB_TEXCOORDS]) >= 0))
		mesh->BindBuffer(mesh->TBO(), id, 2, GL_FLOAT, GL_FALSE);

	return 0;
}

int ShaderProgram::UpdateUniformsGL(Mesh* mesh, bool enableClipping, const glm::vec3 &clipMax, const glm::vec3 &clipMin)
{
	if (mesh == nullptr)
		return -1;

	GLint id;
	bool  isSkybox = (this->name == Utils::SHADER_RESOURCES_DX[SHADER_ID_SKYBOX].Name);
	
	// MATRIX BUFFER
	if ((id = this->Uniforms[MATRIX_BUFFER]) >= 0)
	{
		GLMatrixBuffer mb;

		mb.Model      = mesh->Matrix();
		mb.MVP        = RenderEngine::Camera->MVP(mesh->Matrix(), isSkybox);
		mb.Projection = RenderEngine::Camera->Projection();
		mb.View       = RenderEngine::Camera->View(isSkybox);

		glBindBufferBase(GL_UNIFORM_BUFFER, id, this->UniformBuffers[MATRIX_BUFFER]);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(mb), &mb, GL_STATIC_DRAW);
		glUniformBlockBinding(this->program, id, id);
	}

	// DEFAULT BUFFER
	if ((id = this->Uniforms[DEFAULT_BUFFER]) >= 0)
	{
		GLDefaultBuffer db;

		db.Ambient        = SceneManager::AmbientLightIntensity;
		db.ClipMax        = clipMax;
		db.ClipMin        = clipMin;
		db.EnableClipping = enableClipping;
		db.IsTextured     = mesh->IsTextured();
		db.MaterialColor  = mesh->Color;
		db.SunLight       = SceneManager::SunLight;

		for (int i = 0; i < MAX_TEXTURES; i++)
			db.TextureScales[i] = mesh->Textures[i]->Scale;

		glBindBufferBase(GL_UNIFORM_BUFFER, id, this->UniformBuffers[DEFAULT_BUFFER]);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(db), &db, GL_STATIC_DRAW);
		glUniformBlockBinding(this->program, id, id);
	}

	// MATRICES
	//if ((id = this->Uniforms[MATRIX_MODEL])      >= 0) { glUniformMatrix4fv(id, 1, GL_FALSE, glm::value_ptr(mesh->Matrix())); }
	//if ((id = this->Uniforms[MATRIX_PROJECTION]) >= 0) { glUniformMatrix4fv(id, 1, GL_FALSE, glm::value_ptr(RenderEngine::Camera->Projection())); }
	//if ((id = this->Uniforms[MATRIX_VIEW])       >= 0) { glUniformMatrix4fv(id, 1, GL_FALSE, glm::value_ptr(RenderEngine::Camera->View(isSkybox))); }
	//if ((id = this->Uniforms[MATRIX_MVP])        >= 0) { glUniformMatrix4fv(id, 1, GL_FALSE, glm::value_ptr(RenderEngine::Camera->MVP(mesh->Matrix(), isSkybox))); }

    // CAMERA
    if ((id = this->Uniforms[CAMERA_POSITION]) >= 0) { glUniform3fv(id, 1, glm::value_ptr(RenderEngine::Camera->Position())); }
    if ((id = this->Uniforms[CAMERA_NEAR])     >= 0) { glUniform1f(id, RenderEngine::Camera->Near()); }
    if ((id = this->Uniforms[CAMERA_FAR])      >= 0) { glUniform1f(id, RenderEngine::Camera->Far()); }

    // CLIPPING
    //if ((id = this->Uniforms[ENABLE_CLIPPING]) >= 0) { glUniform1i(id, enableClipping); }
    //if ((id = this->Uniforms[CLIP_MAX])        >= 0) { glUniform3fv(id, 1, glm::value_ptr(clipMax)); }
    //if ((id = this->Uniforms[CLIP_MIN])        >= 0) { glUniform3fv(id, 1, glm::value_ptr(clipMin)); }

    // AMBIENT LIGHT
    //if ((id = this->Uniforms[AMBIENT_LIGHT_INTENSITY]) >= 0) { glUniform3fv(id, 1, glm::value_ptr(SceneManager::AmbientLightIntensity)); }

    // DIRECIONAL LIGHT
    //if ((id = this->Uniforms[SUNLIGHT_COLOR])      >= 0) { glUniform4fv(id, 1, glm::value_ptr(SceneManager::SunLight.Color)); }
    //if ((id = this->Uniforms[SUNLIGHT_DIRECTION])  >= 0) { glUniform3fv(id, 1, glm::value_ptr(SceneManager::SunLight.Direction)); }
    //if ((id = this->Uniforms[SUNLIGHT_POSITION])   >= 0) { glUniform3fv(id, 1, glm::value_ptr(SceneManager::SunLight.Position)); }
    //if ((id = this->Uniforms[SUNLIGHT_REFLECTION]) >= 0) { glUniform1f(id, SceneManager::SunLight.Reflection); }
    //if ((id = this->Uniforms[SUNLIGHT_SHINE])      >= 0) { glUniform1f(id, SceneManager::SunLight.Shine); }
        
    // MATERIAL COLOR
    //if ((id = this->Uniforms[MATERIAL_COLOR]) >= 0) { glUniform4fv(id, 1, glm::value_ptr(mesh->Color)); }
    if ((id = this->Uniforms[SOLID_COLOR])    >= 0) { glUniform4fv(id, 1, glm::value_ptr(SceneManager::SelectColor)); }
    //if ((id = this->Uniforms[IS_TEXTURED])    >= 0) { glUniform1i(id, mesh->IsTextured()); }

    // WATER
	if (((id = this->Uniforms[MOVE_FACTOR])   >= 0) && (mesh->Parent != nullptr)) { glUniform1f(id, dynamic_cast<Water*>(mesh->Parent)->FBO()->MoveFactor()); }
	if (((id = this->Uniforms[WAVE_STRENGTH]) >= 0) && (mesh->Parent != nullptr)) { glUniform1f(id, dynamic_cast<Water*>(mesh->Parent)->FBO()->WaveStrength); }

    // HUD
    if (((id = this->Uniforms[IS_TRANSPARENT]) >= 0) && (mesh->Parent != nullptr)) { glUniform1i(id, dynamic_cast<HUD*>(mesh->Parent)->Transparent); }

    // BIND TEXTURES
    for (int i = 0; i < MAX_TEXTURES; i++)
    {
        if (mesh->Textures[i] == nullptr)
			continue;

        glUniform1i(this->Uniforms[TEXTURES0 + i], i);
        //glUniform2fv(this->Uniforms[TEXTURE_SCALES0 + i], 1, glm::value_ptr(mesh->Textures[i]->Scale));
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(mesh->Textures[i]->Type(), mesh->Textures[i]->ID());
    }

	return 0;
}

int ShaderProgram::UpdateUniformsVK(VkDevice deviceContext, Mesh* mesh, bool enableClipping, const glm::vec3 &clipMax, const glm::vec3 &clipMin)
{
	if ((deviceContext == nullptr) || (mesh == nullptr))
		return -1;

	Buffer* vertexBuffer = mesh->VertexBuffer();

	if (vertexBuffer == nullptr)
		return -2;

	void*                  bufferMemData     = nullptr;
	GLDefaultBuffer        defaultValues     = {};
	GLMatrixBuffer         matrices          = {};
	bool                   removeTranslation = (this->name == Utils::SHADER_RESOURCES_DX[SHADER_ID_SKYBOX].Name);
	VkDescriptorBufferInfo uniformBufferInfo = {};
	VkDescriptorImageInfo  uniformImageInfo[MAX_TEXTURES] = {};
	VkDescriptorSet        uniformSet        = vertexBuffer->UniformSet();
	VkWriteDescriptorSet   uniformWriteSet   = {};

	// Initialize uniform buffer descriptor set
	uniformBufferInfo.range = VK_WHOLE_SIZE;

	uniformWriteSet.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	uniformWriteSet.dstSet          = uniformSet;
	uniformWriteSet.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uniformWriteSet.descriptorCount = 1;
	uniformWriteSet.pBufferInfo     = &uniformBufferInfo;
	//uniformWriteSet.dstArrayElement = 0;

	// MATRIX BUFFER
	VkBuffer       uniformBuffer       = vertexBuffer->UniformBuffer(UNIFORM_BUFFER_MATRIX);
	VkDeviceMemory uniformBufferMemory = vertexBuffer->UniformBufferMemory(UNIFORM_BUFFER_MATRIX);

	if ((uniformBuffer == nullptr) || (uniformBufferMemory == nullptr))
		return -3;

	// Update matrix values
	matrices.Model      = mesh->Matrix();
	matrices.View       = RenderEngine::Camera->View(removeTranslation);
	matrices.Projection = RenderEngine::Camera->Projection();
	matrices.MVP        = RenderEngine::Camera->MVP(mesh->Matrix(), removeTranslation);

	// Copy values to device local buffer
	VkResult result = vkMapMemory(deviceContext, uniformBufferMemory, 0, VK_WHOLE_SIZE, 0, &bufferMemData);

	if (result != VK_SUCCESS)
		return -4;

	memcpy(bufferMemData, &matrices, sizeof(matrices));
	vkUnmapMemory(deviceContext, uniformBufferMemory);

	// Update the descriptor set for binding 0 (matrix buffer)
	uniformBufferInfo.buffer   = uniformBuffer;
	uniformWriteSet.dstBinding = 0;

	vkUpdateDescriptorSets(deviceContext, 1, &uniformWriteSet, 0, nullptr);

	switch (this->ID()) {
	case SHADER_ID_DEFAULT:
		// DEFAULT BUFFER
		uniformBuffer       = vertexBuffer->UniformBuffer(UNIFORM_BUFFER_DEFAULT);
		uniformBufferMemory = vertexBuffer->UniformBufferMemory(UNIFORM_BUFFER_DEFAULT);

		if ((uniformBuffer == nullptr) || (uniformBufferMemory == nullptr))
			return -5;

		// Update default values
		defaultValues.Ambient        = SceneManager::AmbientLightIntensity;
		defaultValues.ClipMax        = clipMax;
		defaultValues.ClipMin        = clipMin;
		defaultValues.EnableClipping = enableClipping;
		defaultValues.IsTextured     = mesh->IsTextured();
		defaultValues.MaterialColor  = mesh->Color;
		defaultValues.SunLight       = SceneManager::SunLight;

		for (int i = 0; i < MAX_TEXTURES; i++)
			defaultValues.TextureScales[i] = mesh->Textures[i]->Scale;

		// Copy values to device local buffer
		result = vkMapMemory(deviceContext, uniformBufferMemory, 0, VK_WHOLE_SIZE, 0, &bufferMemData);

		if (result != VK_SUCCESS)
			return -6;

		memcpy(bufferMemData, &defaultValues, sizeof(defaultValues));
		vkUnmapMemory(deviceContext, uniformBufferMemory);

		// Update the descriptor set for binding 1 (default buffer)
		uniformBufferInfo.buffer   = uniformBuffer;
		uniformWriteSet.dstBinding = 1;

		vkUpdateDescriptorSets(deviceContext, 1, &uniformWriteSet, 0, nullptr);

		// TEXTURE SAMPLER
		for (int i = 0; i < MAX_TEXTURES; i++) {
			uniformImageInfo[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			uniformImageInfo[i].imageView   = mesh->Textures[i]->ImageView();
			uniformImageInfo[i].sampler     = mesh->Textures[i]->Sampler();
		}

		// Update the descriptor set for binding 2 (texture sampler)
		uniformWriteSet.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		uniformWriteSet.descriptorCount = MAX_TEXTURES;
		uniformWriteSet.dstBinding      = 2;
		uniformWriteSet.pBufferInfo     = nullptr;
		uniformWriteSet.pImageInfo      = uniformImageInfo;
		//uniformWriteSet.pTexelBufferView = nullptr;

		vkUpdateDescriptorSets(deviceContext, 1, &uniformWriteSet, 0, nullptr);

		break;
	case SHADER_ID_HUD:
		break;
	case SHADER_ID_SKYBOX:
		break;
	case SHADER_ID_SOLID:
		break;
	case SHADER_ID_TERRAIN:
		break;
	case SHADER_ID_WATER:
		break;
	}

	return 0;
}

#if defined _WINDOWS
int ShaderProgram::UpdateUniformsDX11(ID3D11Buffer** constBuffer, const void** constBufferValues, Mesh* mesh, bool enableClipping, const glm::vec3 &clipMax, const glm::vec3 &clipMin)
{
	if (mesh == nullptr)
		return -1;

	Buffer* vertexBuffer = mesh->VertexBuffer();

	if (vertexBuffer == nullptr)
		return -1;

	DXMatrixBuffer matrices = {};
	DXLightBuffer  sunLight = {};

	if (this->name == Utils::SHADER_RESOURCES_DX[SHADER_ID_SKYBOX].Name) {
		matrices = ShaderProgram::getBufferMatrices(mesh, true);
	} else {
		matrices = ShaderProgram::getBufferMatrices(mesh, false);
		sunLight = ShaderProgram::getBufferLight();
	}

	*constBufferValues = ShaderProgram::getBufferValues(matrices, sunLight, mesh, enableClipping, clipMax, clipMin);

	switch (this->ID()) {
		case SHADER_ID_DEFAULT: *constBuffer = vertexBuffer->ConstantBuffersDX11[SHADER_ID_DEFAULT]; break;
		case SHADER_ID_HUD:     *constBuffer = vertexBuffer->ConstantBuffersDX11[SHADER_ID_HUD];     break;
		case SHADER_ID_SKYBOX:  *constBuffer = vertexBuffer->ConstantBuffersDX11[SHADER_ID_SKYBOX];  break;
		case SHADER_ID_SOLID:   *constBuffer = vertexBuffer->ConstantBuffersDX11[SHADER_ID_SOLID];   break;
		case SHADER_ID_TERRAIN: *constBuffer = vertexBuffer->ConstantBuffersDX11[SHADER_ID_TERRAIN]; break;
		case SHADER_ID_WATER:   *constBuffer = vertexBuffer->ConstantBuffersDX11[SHADER_ID_WATER];   break;
	}
	
	if ((*constBuffer == nullptr) || (*constBufferValues == nullptr))
		return -1;

	return 0;
}

int ShaderProgram::UpdateUniformsDX12(Mesh* mesh, bool enableClipping, const glm::vec3 &clipMax, const glm::vec3 &clipMin)
{
	if (mesh == nullptr)
		return -1;

	Buffer* vertexBuffer = mesh->VertexBuffer();

	if (vertexBuffer == nullptr)
		return -1;

	DXMatrixBuffer matrices = {};
	DXLightBuffer  sunLight = {};

	if (this->name == Utils::SHADER_RESOURCES_DX[SHADER_ID_SKYBOX].Name) {
		matrices = ShaderProgram::getBufferMatrices(mesh, true);
	} else {
		matrices = ShaderProgram::getBufferMatrices(mesh, false);
		sunLight = ShaderProgram::getBufferLight();
	}

	uint8_t*        bufferData        = nullptr;
	ID3D12Resource* constBuffer       = nullptr;
	size_t          constBufferSize   = 0;
	const void*     constBufferValues = ShaderProgram::getBufferValues(matrices, sunLight, mesh, enableClipping, clipMax, clipMin);
	CD3DX12_RANGE   readRange(0, 0);

	switch (this->ID()) {
	case SHADER_ID_DEFAULT:
		constBuffer     = vertexBuffer->ConstantBuffersDX12[SHADER_ID_DEFAULT];
		constBufferSize = sizeof(vertexBuffer->DefaultBufferValues);
		break;
	case SHADER_ID_HUD:
		constBuffer     = vertexBuffer->ConstantBuffersDX12[SHADER_ID_HUD];
		constBufferSize = sizeof(vertexBuffer->HUDBufferValues);
		break;
	case SHADER_ID_SKYBOX:
		constBuffer     = vertexBuffer->ConstantBuffersDX12[SHADER_ID_SKYBOX];
		constBufferSize = sizeof(vertexBuffer->SkyboxBufferValues);
		break;
	case SHADER_ID_SOLID:
		constBuffer     = vertexBuffer->ConstantBuffersDX12[SHADER_ID_SOLID];
		constBufferSize = sizeof(vertexBuffer->SolidBufferValues);
		break;
	case SHADER_ID_TERRAIN:
		constBuffer     = vertexBuffer->ConstantBuffersDX12[SHADER_ID_TERRAIN];
		constBufferSize = sizeof(vertexBuffer->TerrainBufferValues);
		break;
	case SHADER_ID_WATER:
		constBuffer     = vertexBuffer->ConstantBuffersDX12[SHADER_ID_WATER];
		constBufferSize = sizeof(vertexBuffer->WaterBufferValues);
		break;
	}

	if ((constBuffer == nullptr) || (constBufferValues == nullptr))
		return -1;

	HRESULT result = constBuffer->Map(0, &readRange, reinterpret_cast<void**>(&bufferData));

	if (FAILED(result))
		return -1;

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
