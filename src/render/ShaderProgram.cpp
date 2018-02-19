#include "ShaderProgram.h"

ShaderProgram::ShaderProgram(const wxString &name)
{
	this->name    = name;
	this->program = 0;

	#ifdef _WINDOWS
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
	switch (Utils::SelectedGraphicsAPI) {
	#ifdef _WINDOWS
	case GRAPHICS_API_DIRECTX11:
	case GRAPHICS_API_DIRECTX12:
		_RELEASEP(this->shaderVS);
		_RELEASEP(this->shaderFS);
		_RELEASEP(this->vs);
		_RELEASEP(this->fs);
		break;
	#endif
	case GRAPHICS_API_OPENGL:
		glDeleteProgram(this->program);
		break;
	case GRAPHICS_API_VULKAN:
		break;
	default:
		break;
	}
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

bool ShaderProgram::IsOK()
{
	switch (Utils::SelectedGraphicsAPI) {
		#ifdef _WINDOWS
		case GRAPHICS_API_DIRECTX11:
			return ((this->shaderVS != nullptr) && (this->shaderFS != nullptr));
		case GRAPHICS_API_DIRECTX12:
			return ((this->vs != nullptr) && (this->fs != nullptr));
		#endif
		case GRAPHICS_API_OPENGL:
			return (this->program > 0);
		case GRAPHICS_API_VULKAN:
			return false;
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
		return -1;
	default:
		return -1;
	}

	return 0;
}

int ShaderProgram::Load(const wxString &shaderFile)
{
	int result = -1;

	#ifdef _WINDOWS
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
		if (this->LoadShader(GL_VERTEX_SHADER, vs) != 0)
			return -1;

		if (this->LoadShader(GL_FRAGMENT_SHADER, fs) != 0)
			return -1;

		if (this->Link() != 0)
			return -1;

		this->setAttribs();
		this->setUniforms();

		break;
	case GRAPHICS_API_VULKAN:
		return -1;
	default:
		return -1;
	}

	return 0;
}

int ShaderProgram::LoadShader(GLuint type, const wxString &sourceText)
{
	if ((type != GL_VERTEX_SHADER) && (type != GL_FRAGMENT_SHADER))
		return -1;

	GLint         result;
	GLuint        shader;
	const GLchar* sourceTextGLchar;
	GLint         sourceTextGlint;

	#if defined _DEBUG
		GLint               errorLength = 0;
		std::vector<GLchar> error;
	#endif

	switch (Utils::SelectedGraphicsAPI) {
	case GRAPHICS_API_OPENGL:
		shader = glCreateShader(type);

		if (shader < 1)
			return -1;

		sourceTextGLchar = (const GLchar*)sourceText.c_str();
		sourceTextGlint  = (const GLint)sourceText.size();
	
		glShaderSource(shader, 1, &sourceTextGLchar, &sourceTextGlint);
		glCompileShader(shader);

		glGetShaderiv(shader, GL_COMPILE_STATUS, &result);

		if (result != GL_TRUE)
		{
			#if defined _DEBUG
				glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &errorLength);

				error.resize(errorLength);
				glGetShaderInfoLog(shader, errorLength, &errorLength, &error[0]);

				wxMessageBox((char*)&error[0]);
			#endif

			return -1;
		}

		glAttachShader(this->program, shader);

		break;
	case GRAPHICS_API_VULKAN:
		return -1;
	default:
		return -1;
	}

	return 0;
}

wxString ShaderProgram::Name()
{
	return this->name;
}

GLuint ShaderProgram::Program()
{
    return this->program;
}

void ShaderProgram::setAttribs()
{
	switch (Utils::SelectedGraphicsAPI) {
	case GRAPHICS_API_OPENGL:
		glUseProgram(this->program);

		// ATTRIBS (BUFFERS)
		this->Attribs[VERTEX_NORMAL]    = glGetAttribLocation(this->program, "VertexNormal");
		this->Attribs[VERTEX_POSITION]  = glGetAttribLocation(this->program, "VertexPosition");
		this->Attribs[VERTEX_TEXCOORDS] = glGetAttribLocation(this->program, "VertexTextureCoords");

		glUseProgram(0);

		break;
	case GRAPHICS_API_VULKAN:
		break;
	default:
		break;
	}
}

void ShaderProgram::setUniforms()
{
	switch (Utils::SelectedGraphicsAPI) {
	case GRAPHICS_API_OPENGL:
		glUseProgram(this->program);

		// MATRICES
		this->Uniforms[MATRIX_MODEL]      = glGetUniformLocation(this->program, "MatrixModel");
		this->Uniforms[MATRIX_VIEW]       = glGetUniformLocation(this->program, "MatrixView");
		this->Uniforms[MATRIX_PROJECTION] = glGetUniformLocation(this->program, "MatrixProjection");
		this->Uniforms[MATRIX_MVP]        = glGetUniformLocation(this->program, "MatrixMVP");

		// CAMERA
		this->Uniforms[CAMERA_POSITION] = glGetUniformLocation(this->program, "CameraMain.Position");
		this->Uniforms[CAMERA_NEAR]     = glGetUniformLocation(this->program, "CameraMain.Near");
		this->Uniforms[CAMERA_FAR]      = glGetUniformLocation(this->program, "CameraMain.Far");

		// CLIPPING
		this->Uniforms[ENABLE_CLIPPING] = glGetUniformLocation(this->program, "EnableClipping");
		this->Uniforms[CLIP_MAX]        = glGetUniformLocation(this->program, "ClipMax");
		this->Uniforms[CLIP_MIN]        = glGetUniformLocation(this->program, "ClipMin");

		// AMBIENT LIGHT
		this->Uniforms[AMBIENT_LIGHT_INTENSITY] = glGetUniformLocation(this->program, "AmbientLightIntensity");

		// DIRECIONAL LIGHT
		this->Uniforms[SUNLIGHT_COLOR]      = glGetUniformLocation(this->program, "SunLight.Color");
		this->Uniforms[SUNLIGHT_DIRECTION]  = glGetUniformLocation(this->program, "SunLight.Direction");
		this->Uniforms[SUNLIGHT_POSITION]   = glGetUniformLocation(this->program, "SunLight.Position");
		this->Uniforms[SUNLIGHT_REFLECTION] = glGetUniformLocation(this->program, "SunLight.Reflection");
		this->Uniforms[SUNLIGHT_SHINE]      = glGetUniformLocation(this->program, "SunLight.Shine");
        
		// MATERIAL COLOR
		this->Uniforms[MATERIAL_COLOR] = glGetUniformLocation(this->program, "MaterialColor");
		this->Uniforms[SOLID_COLOR]    = glGetUniformLocation(this->program, "SolidColor");
		this->Uniforms[IS_TEXTURED]    = glGetUniformLocation(this->program, "IsTextured");

		// WATER
		this->Uniforms[MOVE_FACTOR]   = glGetUniformLocation(this->program, "MoveFactor");
		this->Uniforms[WAVE_STRENGTH] = glGetUniformLocation(this->program, "WaveStrength");

		// HUD
		this->Uniforms[IS_TRANSPARENT] = glGetUniformLocation(this->program, "IsTransparent");

		// BIND TEXTURES
		for (int i = 0; i < MAX_TEXTURES; i++) {
			this->Uniforms[TEXTURES0       + i] = glGetUniformLocation(this->program, wxString("Textures[" + std::to_string(i) + "]").c_str());
			this->Uniforms[TEXTURE_SCALES0 + i] = glGetUniformLocation(this->program, wxString("TextureScales[" + std::to_string(i) + "]").c_str());
		}

		glUseProgram(0);

		break;
	case GRAPHICS_API_VULKAN:
		break;
	default:
		break;
	}
}

int ShaderProgram::UpdateAttribsGL(Mesh* mesh)
{
	if (mesh == nullptr)
		return -1;

	GLint id;

	if (mesh->NBO() > 0) {
		if ((id = this->Attribs[VERTEX_NORMAL]) >= 0)
			mesh->BindBuffer(mesh->NBO(), id, 3, GL_FLOAT, GL_FALSE);
	}

	if (mesh->VBO() > 0) {
		if ((id = this->Attribs[VERTEX_POSITION]) >= 0)
			mesh->BindBuffer(mesh->VBO(), id, 3, GL_FLOAT, GL_FALSE);
	}

	if (mesh->TBO() > 0) {
		if ((id = this->Attribs[VERTEX_TEXCOORDS]) >= 0)
			mesh->BindBuffer(mesh->TBO(), id, 2, GL_FLOAT, GL_FALSE);
	}

	return 0;
}

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

int ShaderProgram::UpdateUniformsGL(Mesh* mesh, bool enableClipping, const glm::vec3 &clipMax, const glm::vec3 &clipMin)
{
	if (mesh == nullptr)
		return -1;

	GLint id;
	bool  isSkybox = (this->name == Utils::SHADER_RESOURCES_DX[SHADER_ID_SKYBOX].Name);
	
	// MATRICES
	if ((id = this->Uniforms[MATRIX_MODEL])      >= 0) { glUniformMatrix4fv(id, 1, GL_FALSE, glm::value_ptr(mesh->Matrix())); }
	if ((id = this->Uniforms[MATRIX_PROJECTION]) >= 0) { glUniformMatrix4fv(id, 1, GL_FALSE, glm::value_ptr(RenderEngine::Camera->Projection())); }
	if ((id = this->Uniforms[MATRIX_VIEW])       >= 0) { glUniformMatrix4fv(id, 1, GL_FALSE, glm::value_ptr(RenderEngine::Camera->View(isSkybox))); }
	if ((id = this->Uniforms[MATRIX_MVP])        >= 0) { glUniformMatrix4fv(id, 1, GL_FALSE, glm::value_ptr(RenderEngine::Camera->MVP(mesh->Matrix(), isSkybox))); }

    // CAMERA
    if ((id = this->Uniforms[CAMERA_POSITION]) >= 0) { glUniform3fv(id, 1, glm::value_ptr(RenderEngine::Camera->Position())); }
    if ((id = this->Uniforms[CAMERA_NEAR])     >= 0) { glUniform1f(id, RenderEngine::Camera->Near()); }
    if ((id = this->Uniforms[CAMERA_FAR])      >= 0) { glUniform1f(id, RenderEngine::Camera->Far()); }

    // CLIPPING
    if ((id = this->Uniforms[ENABLE_CLIPPING]) >= 0) { glUniform1i(id, enableClipping); }
    if ((id = this->Uniforms[CLIP_MAX])        >= 0) { glUniform3fv(id, 1, glm::value_ptr(clipMax)); }
    if ((id = this->Uniforms[CLIP_MIN])        >= 0) { glUniform3fv(id, 1, glm::value_ptr(clipMin)); }

    // AMBIENT LIGHT
    if ((id = this->Uniforms[AMBIENT_LIGHT_INTENSITY]) >= 0) { glUniform3fv(id, 1, glm::value_ptr(SceneManager::AmbientLightIntensity)); }

    // DIRECIONAL LIGHT
    if ((id = this->Uniforms[SUNLIGHT_COLOR])      >= 0) { glUniform4fv(id, 1, glm::value_ptr(SceneManager::SunLight.Color)); }
    if ((id = this->Uniforms[SUNLIGHT_DIRECTION])  >= 0) { glUniform3fv(id, 1, glm::value_ptr(SceneManager::SunLight.Direction)); }
    if ((id = this->Uniforms[SUNLIGHT_POSITION])   >= 0) { glUniform3fv(id, 1, glm::value_ptr(SceneManager::SunLight.Position)); }
    if ((id = this->Uniforms[SUNLIGHT_REFLECTION]) >= 0) { glUniform1f(id, SceneManager::SunLight.Reflection); }
    if ((id = this->Uniforms[SUNLIGHT_SHINE])      >= 0) { glUniform1f(id, SceneManager::SunLight.Shine); }
        
    // MATERIAL COLOR
    if ((id = this->Uniforms[MATERIAL_COLOR]) >= 0) { glUniform4fv(id, 1, glm::value_ptr(mesh->Color)); }
    if ((id = this->Uniforms[SOLID_COLOR])    >= 0) { glUniform4fv(id, 1, glm::value_ptr(SceneManager::SelectColor)); }
    if ((id = this->Uniforms[IS_TEXTURED])    >= 0) { glUniform1i(id, mesh->IsTextured()); }

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
        glUniform2fv(this->Uniforms[TEXTURE_SCALES0 + i], 1, glm::value_ptr(mesh->Textures[i]->Scale));
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(mesh->Textures[i]->Type(), mesh->Textures[i]->ID());
    }

	return 0;
}

ID3DBlob* ShaderProgram::FS()
{
	return this->fs;
}

ID3DBlob* ShaderProgram::VS()
{
	return this->vs;
}

ID3D11PixelShader* ShaderProgram::FragmentShader()
{
	return this->shaderFS;
}

ID3D11VertexShader* ShaderProgram::VertexShader()
{
	return this->shaderVS;
}
