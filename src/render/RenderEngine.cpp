#include "RenderEngine.h"

GLCanvas                RenderEngine::Canvas              = {};
DrawModeType            RenderEngine::drawMode            = DRAW_MODE_FILLED;
Camera*                 RenderEngine::CameraMain          = nullptr;
GPUDescription          RenderEngine::GPU                 = {};
bool                    RenderEngine::DrawBoundingVolume  = false;
bool                    RenderEngine::EnableSRGB          = true;
Mesh*                   RenderEngine::Skybox              = nullptr;
std::vector<Component*> RenderEngine::HUDs;
std::vector<Component*> RenderEngine::LightSources;
bool                    RenderEngine::Ready               = false;
std::vector<Component*> RenderEngine::Renderables;
GraphicsAPI             RenderEngine::SelectedGraphicsAPI = GRAPHICS_API_UNKNOWN;

void RenderEngine::clear(const glm::vec4 &colorRGBA, const DrawProperties &properties)
{
	switch (RenderEngine::SelectedGraphicsAPI) {
	#if defined _WINDOWS
	case GRAPHICS_API_DIRECTX11:
		if (RenderEngine::Canvas.DX != nullptr)
			RenderEngine::Canvas.DX->Clear11(colorRGBA, properties);
		break;
	case GRAPHICS_API_DIRECTX12:
		if (RenderEngine::Canvas.DX != nullptr)
			RenderEngine::Canvas.DX->Clear12(colorRGBA, properties);
		break;
	#endif
	case GRAPHICS_API_OPENGL:
		glClearColor(colorRGBA.r, colorRGBA.g, colorRGBA.b, colorRGBA.a);
		glClearStencil(0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		break;
	case GRAPHICS_API_VULKAN:
		if (RenderEngine::Canvas.VK != nullptr)
			RenderEngine::Canvas.VK->Clear(colorRGBA, properties);
		break;
	}
}

void RenderEngine::Close()
{
	InputManager::Reset();
	SceneManager::Clear();
	ShaderManager::Close();

	_DELETEP(SceneManager::DepthMap2D);
	_DELETEP(SceneManager::DepthMapCube);

	_DELETEP(SceneManager::EmptyCubemap);
	_DELETEP(SceneManager::EmptyTexture);

	_DELETEP(RenderEngine::Canvas.DX);
	_DELETEP(RenderEngine::Canvas.GL);
	_DELETEP(RenderEngine::Canvas.VK);

	if (RenderEngine::Canvas.Canvas != nullptr) {
		RenderEngine::Canvas.Canvas->DestroyChildren();
		RenderEngine::Canvas.Canvas->Destroy();
		RenderEngine::Canvas.Canvas = nullptr;
	}
}

void RenderEngine::createDepthFBO()
{
	if (RenderEngine::Renderables.empty())
		return;

	bool cleared = false;

	for (uint32_t i = 0; i < MAX_LIGHT_SOURCES; i++)
	{
		if (SceneManager::LightSources[i] == nullptr)
			continue;

		FrameBuffer* fbo   = nullptr;
		LightSource* light = SceneManager::LightSources[i];

		switch (light->SourceType()) {
			case ID_ICON_LIGHT_DIRECTIONAL: fbo = SceneManager::DepthMap2D;   break;
			case ID_ICON_LIGHT_POINT:       fbo = SceneManager::DepthMapCube; break;
			case ID_ICON_LIGHT_SPOT: break;
			default: throw;
		}

		if (fbo == nullptr)
			continue;

		DrawProperties drawProperties = {};

		drawProperties.DepthLayer = i;
		drawProperties.FBO        = fbo;
		drawProperties.Light      = light;

		if (light->SourceType() == ID_ICON_LIGHT_POINT)
			drawProperties.Shader = SHADER_ID_DEPTH_OMNI;
		else
			drawProperties.Shader = SHADER_ID_DEPTH;

		// BIND
		VkCommandBuffer cmdBuffer = nullptr;

		if (RenderEngine::SelectedGraphicsAPI == GRAPHICS_API_VULKAN)
			cmdBuffer = RenderEngine::Canvas.VK->CommandBufferBegin();
		else
			fbo->Bind(drawProperties.DepthLayer);

		drawProperties.VKCommandBuffer = cmdBuffer;

		// CLEAR
		if ((RenderEngine::SelectedGraphicsAPI == GRAPHICS_API_OPENGL) &&
			(light->SourceType() == ID_ICON_LIGHT_POINT))
		{
			if (!cleared) {
				RenderEngine::clear(CLEAR_VALUE_DEPTH, drawProperties);
				cleared = true;
			}
		} else {
			RenderEngine::clear(CLEAR_VALUE_DEPTH, drawProperties);
		}

		// DRAW
		RenderEngine::drawRenderables(drawProperties);

		// UNBIND
		if (RenderEngine::SelectedGraphicsAPI == GRAPHICS_API_VULKAN)
			RenderEngine::Canvas.VK->Present(cmdBuffer);
		else
			fbo->Unbind();
	}
}

void RenderEngine::createWaterFBOs()
{
	for (auto component : RenderEngine::Renderables)
	{
		if ((component == nullptr) || (component->Type() != COMPONENT_WATER))
			continue;

		Water* water = dynamic_cast<Water*>(component->Parent);

		if (water == nullptr)
			continue;

		glm::vec3       position       = component->Position();
		glm::vec3       scale          = component->Scale();
		float           cameraDistance = ((RenderEngine::CameraMain->Position().y - position.y) * 2.0f);
		VkCommandBuffer cmdBuffer      = nullptr;

		// WATER REFLECTION PASS - ABOVE WATER
		RenderEngine::CameraMain->MoveBy(glm::vec3(0.0f, -cameraDistance, 0.0f));
		RenderEngine::CameraMain->InvertPitch();

		if (RenderEngine::SelectedGraphicsAPI == GRAPHICS_API_VULKAN)
			cmdBuffer = RenderEngine::Canvas.VK->CommandBufferBegin();
		else
			water->FBO()->BindReflection();

		DrawProperties drawProperties = {};

		drawProperties.EnableClipping  = true;
		drawProperties.FBO             = water->FBO()->ReflectionFBO();
		drawProperties.ClipMax         = glm::vec3(scale.x,  scale.y,    scale.z);
		drawProperties.ClipMin         = glm::vec3(-scale.x, position.y, -scale.z);
		drawProperties.VKCommandBuffer = cmdBuffer;

		RenderEngine::clear(CLEAR_VALUE_COLOR, drawProperties);

		RenderEngine::drawSkybox(drawProperties);
		RenderEngine::drawRenderables(drawProperties);
		
		if (RenderEngine::SelectedGraphicsAPI == GRAPHICS_API_VULKAN)
			RenderEngine::Canvas.VK->Present(cmdBuffer);
		else
			water->FBO()->UnbindReflection();

		RenderEngine::CameraMain->InvertPitch();
		RenderEngine::CameraMain->MoveBy(glm::vec3(0.0, cameraDistance, 0.0));

		// WATER REFRACTION PASS - BELOW WATER
		if (RenderEngine::SelectedGraphicsAPI == GRAPHICS_API_VULKAN)
			cmdBuffer = RenderEngine::Canvas.VK->CommandBufferBegin();
		else
			water->FBO()->BindRefraction();

		drawProperties.ClipMax = glm::vec3(scale.x,  position.y, scale.z);
		drawProperties.ClipMin = glm::vec3(-scale.x, -scale.y,   -scale.z);
		drawProperties.FBO     = water->FBO()->RefractionFBO();

		RenderEngine::clear(CLEAR_VALUE_COLOR, drawProperties);

		RenderEngine::drawSkybox(drawProperties);
		RenderEngine::drawRenderables(drawProperties);

		if (RenderEngine::SelectedGraphicsAPI == GRAPHICS_API_VULKAN)
			RenderEngine::Canvas.VK->Present(cmdBuffer);
		else
			water->FBO()->UnbindRefraction();
	}
}

void RenderEngine::Draw()
{
	RenderEngine::createDepthFBO();
	RenderEngine::createWaterFBOs();

	RenderEngine::clear(CLEAR_VALUE_DEFAULT, {});
	RenderEngine::drawScene();

	switch (RenderEngine::SelectedGraphicsAPI) {
		#if defined _WINDOWS
		case GRAPHICS_API_DIRECTX11:
			if (RenderEngine::Canvas.DX != nullptr)
				RenderEngine::Canvas.DX->Present11();
			break;
		case GRAPHICS_API_DIRECTX12:
			if (RenderEngine::Canvas.DX != nullptr)
				RenderEngine::Canvas.DX->Present12();
			break;
		#endif
		case GRAPHICS_API_OPENGL:
			if (RenderEngine::Canvas.Canvas != nullptr)
				RenderEngine::Canvas.Canvas->SwapBuffers();
			break;
		case GRAPHICS_API_VULKAN:
			if (RenderEngine::Canvas.VK != nullptr)
				RenderEngine::Canvas.VK->Present();
			break;
	}
}

int RenderEngine::drawBoundingVolumes()
{
    if (!RenderEngine::DrawBoundingVolume)
		return 1;

	DrawModeType oldDrawMode = RenderEngine::drawMode;

	RenderEngine::SetDrawMode(DRAW_MODE_WIREFRAME);

	DrawProperties properties = {};

	properties.DrawBoundingVolume = true;
	properties.Shader             = SHADER_ID_WIREFRAME;

	RenderEngine::drawMeshes(RenderEngine::Renderables, properties);

	RenderEngine::drawMode = oldDrawMode;

    return 0;
}    

int RenderEngine::drawHUDs()
{
	if (RenderEngine::HUDs.empty())
		return 1;

	if (RenderEngine::SelectedGraphicsAPI == GRAPHICS_API_OPENGL)
		RenderEngine::setDrawSettingsGL(SHADER_ID_HUD);

	DrawProperties properties = {};
	properties.Shader         = (RenderEngine::drawMode == DRAW_MODE_FILLED ? SHADER_ID_HUD : SHADER_ID_WIREFRAME);

	RenderEngine::drawMeshes(RenderEngine::HUDs, properties);

	return 0;
}

int RenderEngine::drawLightSources()
{
	if (RenderEngine::LightSources.empty())
		return 1;

	if (RenderEngine::SelectedGraphicsAPI == GRAPHICS_API_OPENGL)
		RenderEngine::setDrawSettingsGL(SHADER_ID_COLOR);

	DrawProperties properties = {};
	properties.Shader         = (RenderEngine::drawMode == DRAW_MODE_FILLED ? SHADER_ID_COLOR : SHADER_ID_WIREFRAME);

	RenderEngine::drawMeshes(RenderEngine::LightSources, properties);

	return 0;
}

int RenderEngine::drawRenderables(DrawProperties &properties)
{
	if (RenderEngine::Renderables.empty())
		return 1;

	if (RenderEngine::SelectedGraphicsAPI == GRAPHICS_API_OPENGL)
		RenderEngine::setDrawSettingsGL(SHADER_ID_DEFAULT);

	if (properties.Shader == SHADER_ID_UNKNOWN)
		properties.Shader = (RenderEngine::drawMode == DRAW_MODE_FILLED ? SHADER_ID_DEFAULT : SHADER_ID_WIREFRAME);

	RenderEngine::drawMeshes(RenderEngine::Renderables, properties);

	properties.Shader = SHADER_ID_UNKNOWN;

	return 0;
}

int RenderEngine::drawSelected()
{
	DrawModeType oldDrawMode = RenderEngine::drawMode;

	RenderEngine::SetDrawMode(DRAW_MODE_WIREFRAME);

	DrawProperties properties = {};

	properties.DrawSelected = true;
	properties.Shader       = SHADER_ID_WIREFRAME;

	if (RenderEngine::SelectedGraphicsAPI == GRAPHICS_API_OPENGL)
		RenderEngine::setDrawSettingsGL(SHADER_ID_WIREFRAME);

	RenderEngine::drawMeshes(RenderEngine::Renderables, properties);

	RenderEngine::drawMode = oldDrawMode;

	return 0;
}

int RenderEngine::drawSkybox(DrawProperties &properties)
{
	if (RenderEngine::Skybox == nullptr)
		return 1;

	if (RenderEngine::SelectedGraphicsAPI == GRAPHICS_API_OPENGL)
		RenderEngine::setDrawSettingsGL(SHADER_ID_SKYBOX);

	if (properties.Shader == SHADER_ID_UNKNOWN)
		properties.Shader = (RenderEngine::drawMode == DRAW_MODE_FILLED ? SHADER_ID_SKYBOX : SHADER_ID_WIREFRAME);

	RenderEngine::drawMeshes({ RenderEngine::Skybox }, properties);

	properties.Shader = SHADER_ID_UNKNOWN;

	return 0;
}

void RenderEngine::drawMesh(Component* mesh, ShaderProgram* shaderProgram, DrawProperties &properties)
{
	switch (RenderEngine::SelectedGraphicsAPI) {
		#if defined _WINDOWS
		case GRAPHICS_API_DIRECTX11:
			RenderEngine::drawMeshDX11(mesh, shaderProgram, properties);
			break;
		case GRAPHICS_API_DIRECTX12:
			RenderEngine::drawMeshDX12(mesh, shaderProgram, properties);
			break;
		#endif
		case GRAPHICS_API_OPENGL:
			RenderEngine::drawMeshGL(mesh, shaderProgram, properties);
			break;
		case GRAPHICS_API_VULKAN:
			RenderEngine::drawMeshVK(mesh, shaderProgram, properties);
			break;
		default:
			throw;
	}
}

int RenderEngine::drawMeshDX11(Component* mesh, ShaderProgram* shaderProgram, DrawProperties &properties)
{
	return RenderEngine::Canvas.DX->Draw11(mesh, shaderProgram, properties);
}

int RenderEngine::drawMeshDX12(Component* mesh, ShaderProgram* shaderProgram, DrawProperties &properties)
{
	return RenderEngine::Canvas.DX->Draw12(mesh, shaderProgram, properties);
}

int RenderEngine::drawMeshGL(Component* mesh, ShaderProgram* shaderProgram, DrawProperties &properties)
{
	if ((RenderEngine::CameraMain == nullptr) ||
		(shaderProgram == nullptr) || (shaderProgram->Program() < 1) ||
		(mesh == nullptr) || (dynamic_cast<Mesh*>(mesh)->IBO() < 1))
	{
		return -1;
	}

	// SHADER ATTRIBUTES AND UNIFORMS
	shaderProgram->UpdateAttribsGL(mesh);
	shaderProgram->UpdateUniformsGL(mesh, properties);

    // DRAW
	if (dynamic_cast<Mesh*>(mesh)->IBO() > 0) {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, dynamic_cast<Mesh*>(mesh)->IBO());
		glDrawElements(RenderEngine::GetDrawMode(), (GLsizei)dynamic_cast<Mesh*>(mesh)->NrOfIndices(), GL_UNSIGNED_INT, nullptr);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	} else {
		glBindBuffer(GL_VERTEX_ARRAY, dynamic_cast<Mesh*>(mesh)->VBO());
		glDrawArrays(RenderEngine::GetDrawMode(), 0, (GLsizei)dynamic_cast<Mesh*>(mesh)->NrOfVertices());
		glBindBuffer(GL_VERTEX_ARRAY, 0);
	}

    // UNBIND TEXTURES
    for (int i = 0; i < MAX_TEXTURES; i++) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D,       0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    }

    glActiveTexture(GL_TEXTURE0);

    return 0;
}

int RenderEngine::drawMeshVK(Component* mesh, ShaderProgram* shaderProgram, DrawProperties &properties)
{
	return RenderEngine::Canvas.VK->Draw(mesh, shaderProgram, properties);
}

void RenderEngine::drawMeshes(const std::vector<Component*> meshes, DrawProperties &properties)
{
	ShaderProgram* shaderProgram = RenderEngine::setShaderProgram(true, properties.Shader);

	for (auto mesh : meshes)
	{
		if (!properties.DrawBoundingVolume &&
			((properties.DrawSelected && !dynamic_cast<Mesh*>(mesh)->IsSelected()) ||
			(!properties.DrawSelected && dynamic_cast<Mesh*>(mesh)->IsSelected())))
		{
			continue;
		}

		// SKIP RENDERING WATER WHEN CREATING FBO
		if ((mesh->Type() == COMPONENT_WATER) && (properties.FBO != nullptr) && (properties.FBO->Type() != FBO_UNKNOWN))
			continue;

		glm::vec4 oldColor = mesh->ComponentMaterial.diffuse;

		if (properties.DrawSelected)
			mesh->ComponentMaterial.diffuse = SceneManager::SelectColor;

		RenderEngine::drawMesh(
			(properties.DrawBoundingVolume ? dynamic_cast<Mesh*>(mesh)->GetBoundingVolume() : mesh), shaderProgram, properties
		);

		if (properties.DrawSelected)
			mesh->ComponentMaterial.diffuse = oldColor;
	}

	RenderEngine::setShaderProgram(false);
}

void RenderEngine::drawScene()
{
	RenderEngine::drawRenderables();
	RenderEngine::drawLightSources();
	RenderEngine::drawSelected();
	RenderEngine::drawBoundingVolumes();
	RenderEngine::drawSkybox();
    RenderEngine::drawHUDs();
}

uint16_t RenderEngine::GetDrawMode()
{
	if (RenderEngine::drawMode == DRAW_MODE_FILLED)
	{
		switch (RenderEngine::SelectedGraphicsAPI) {
		#if defined _WINDOWS
		case GRAPHICS_API_DIRECTX11:
		case GRAPHICS_API_DIRECTX12:
			return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		#endif
		case GRAPHICS_API_OPENGL:
			return GL_TRIANGLES;
		case GRAPHICS_API_VULKAN:
			return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		default:
			throw;
		}
	}
	else if (RenderEngine::drawMode == DRAW_MODE_WIREFRAME)
	{
		switch (RenderEngine::SelectedGraphicsAPI) {
			#if defined _WINDOWS
			case GRAPHICS_API_DIRECTX11:
			case GRAPHICS_API_DIRECTX12:
				return D3D_PRIMITIVE_TOPOLOGY_LINESTRIP;
			#endif
			case GRAPHICS_API_OPENGL:
				return GL_LINE_STRIP;
			case GRAPHICS_API_VULKAN:
				return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
			default:
				throw;
		}
	}

	return DRAW_MODE_UNKNOWN;
}

int RenderEngine::Init(WindowFrame* window, const wxSize &size)
{
	RenderEngine::Canvas.AspectRatio = (float)((float)size.GetHeight() / (float)size.GetWidth());
	RenderEngine::Canvas.Position    = wxDefaultPosition;
	RenderEngine::Canvas.Size        = size;
	RenderEngine::Canvas.Window      = window;

	int result = RenderEngine::setGraphicsAPI(GRAPHICS_API_OPENGL);

	if (result < 0)
		return result;

	return 0;
}

int RenderEngine::initResources()
{
	wxString              emptyFile  = Utils::RESOURCE_IMAGES["emptyTexture"];
	std::vector<wxString> emptyFiles = { emptyFile, emptyFile, emptyFile, emptyFile, emptyFile, emptyFile };

	SceneManager::EmptyTexture = new Texture(emptyFile);
	SceneManager::EmptyCubemap = new Texture(emptyFiles);
	
	if (!SceneManager::EmptyTexture->IsOK() || !SceneManager::EmptyCubemap->IsOK())
		return -1;

	SceneManager::DepthMap2D   = new FrameBuffer(wxSize(FBO_TEXTURE_SIZE, FBO_TEXTURE_SIZE), FBO_DEPTH, TEXTURE_2D_ARRAY);
	SceneManager::DepthMapCube = new FrameBuffer(wxSize(FBO_TEXTURE_SIZE, FBO_TEXTURE_SIZE), FBO_DEPTH, TEXTURE_CUBEMAP_ARRAY);

	if ((SceneManager::DepthMap2D->GetTexture() == nullptr) || (SceneManager::DepthMapCube->GetTexture() == nullptr))
		return -2;

	return 0;
}

int RenderEngine::RemoveMesh(Component* mesh)
{
	if (mesh->Parent == nullptr)
		return -1;

	std::vector<Component*>::iterator index;

	switch (mesh->Parent->Type()) {
	case COMPONENT_HUD:
		index = std::find(RenderEngine::HUDs.begin(), RenderEngine::HUDs.end(), mesh);

        if (index != RenderEngine::HUDs.end())
			RenderEngine::HUDs.erase(index);

		break;
	case COMPONENT_SKYBOX:
        RenderEngine::Skybox = nullptr;
		break;
	case COMPONENT_LIGHTSOURCE:
		index = std::find(RenderEngine::LightSources.begin(), RenderEngine::LightSources.end(), mesh);

		if (index != RenderEngine::LightSources.end())
			RenderEngine::LightSources.erase(index);

		break;
	default:
		index = std::find(RenderEngine::Renderables.begin(), RenderEngine::Renderables.end(), mesh);

		if (index != RenderEngine::Renderables.end())
			RenderEngine::Renderables.erase(index);

		break;
	}

	return 0;
}

void RenderEngine::SetAspectRatio(const wxString &ratio)
{
	if (ratio == Utils::ASPECT_RATIOS[0])
		RenderEngine::Canvas.AspectRatio = 0.5625f;
	else if (ratio == Utils::ASPECT_RATIOS[1])
		RenderEngine::Canvas.AspectRatio = 0.75f;

    RenderEngine::SetCanvasSize(
		RenderEngine::Canvas.Size.GetWidth(),
		(int)((float)RenderEngine::Canvas.Size.GetWidth() * RenderEngine::Canvas.AspectRatio)
	);

	if (RenderEngine::Canvas.VK != nullptr)
		RenderEngine::Canvas.VK->ResetSwapChain();
}

void RenderEngine::SetCanvasSize(int width, int height)
{
	RenderEngine::Canvas.Size = wxSize(width, height);
	RenderEngine::Canvas.Canvas->SetSize(RenderEngine::Canvas.Size);
	RenderEngine::CameraMain->UpdateProjection();

	if (RenderEngine::SelectedGraphicsAPI == GRAPHICS_API_OPENGL)
		glViewport(0, 0, width, height);
}

void RenderEngine::SetDrawMode(DrawModeType mode)
{
	RenderEngine::drawMode = mode;
}

void RenderEngine::SetDrawMode(const wxString &mode)
{
	if (mode == Utils::DRAW_MODES[DRAW_MODE_FILLED])
		RenderEngine::drawMode = DRAW_MODE_FILLED;
	else if (mode == Utils::DRAW_MODES[DRAW_MODE_WIREFRAME])
		RenderEngine::drawMode = DRAW_MODE_WIREFRAME;
}

void RenderEngine::setDrawSettingsGL(ShaderID shaderID)
{
	switch (shaderID) {
	case SHADER_ID_HUD:
		glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_CLAMP);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_STENCIL_TEST);
		break;
	case SHADER_ID_SKYBOX:
		glEnable(GL_DEPTH_TEST); glDepthFunc(GL_LEQUAL); glDepthMask(GL_TRUE);
		glDisable(GL_BLEND);
		glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_CLAMP);
		glDisable(GL_STENCIL_TEST);
		break;
	case SHADER_ID_DEPTH:
	case SHADER_ID_DEPTH_OMNI:
		glEnable(GL_CULL_FACE);  glCullFace(GL_FRONT); glFrontFace(GL_CCW);
		glEnable(GL_DEPTH_CLAMP);
		glEnable(GL_DEPTH_TEST); glDepthFunc(GL_LESS); glDepthMask(GL_TRUE);
		glDisable(GL_STENCIL_TEST);
		glDisable(GL_BLEND);
		break;
	default:
		glEnable(GL_CULL_FACE);  glCullFace(GL_BACK);  glFrontFace(GL_CCW);
		glEnable(GL_DEPTH_TEST); glDepthFunc(GL_LESS); glDepthMask(GL_TRUE);
		glDisable(GL_BLEND);
		glDisable(GL_DEPTH_CLAMP);
		glDisable(GL_STENCIL_TEST);
		break;
	}
}

int RenderEngine::SetGraphicsAPI(const wxString &api)
{
	GraphicsAPI lastAPI = RenderEngine::SelectedGraphicsAPI;
	int         result  = -1;

	if (api == "DirectX 11")
		result = RenderEngine::setGraphicsAPI(GRAPHICS_API_DIRECTX11);
	else if (api == "DirectX 12")
		result = RenderEngine::setGraphicsAPI(GRAPHICS_API_DIRECTX12);
	else if (api == "OpenGL")
		result = RenderEngine::setGraphicsAPI(GRAPHICS_API_OPENGL);
	else if (api == "Vulkan")
		result = RenderEngine::setGraphicsAPI(GRAPHICS_API_VULKAN);

	if (result < 0)
	{
		wxMessageBox("ERROR: Failed to initialize the " + api + " graphics engine.");

		RenderEngine::setGraphicsAPI(lastAPI);
		RenderEngine::Canvas.Window->SetGraphicsAPI(lastAPI);
	}

	return result;
}

int RenderEngine::setGraphicsAPI(GraphicsAPI api)
{
	RenderEngine::Ready               = false;
	RenderEngine::SelectedGraphicsAPI = api;

	// CLEAR SCENE AND FREE MEMORY
	RenderEngine::Close();

	// RE-CREATE THE CANVAS
	if (RenderEngine::setGraphicsApiCanvas() < 0)
		return -1;

	int result = -2;

	// RE-CREATE THE GRAPHICS CONTEXT
	switch (api) {
	#if defined _WINDOWS
	case GRAPHICS_API_DIRECTX11:
	case GRAPHICS_API_DIRECTX12:
		result = RenderEngine::setGraphicsApiDX(api);
		break;
	#endif
	case GRAPHICS_API_OPENGL:
		result = RenderEngine::setGraphicsApiGL();
		break;
	case GRAPHICS_API_VULKAN:
		result = RenderEngine::setGraphicsApiVK();
		break;
	default:
		throw;
	}

	if (result < 0)
		return result;

	// RE-INITIALIZE ENGINE MODULES AND RESOURCES
	if (ShaderManager::Init() < 0) {
		RenderEngine::Close();
		return -3;
	}

	if (RenderEngine::initResources() < 0) {
		RenderEngine::Close();
		return -4;
	}

	if (InputManager::Init() < 0) {
		RenderEngine::Close();
		return -5;
	}

	if (RenderEngine::CameraMain == nullptr) {
		SceneManager::AddComponent(new Camera());
		SceneManager::LoadLightSource(ID_ICON_LIGHT_DIRECTIONAL);
	}

	RenderEngine::Ready = true;

	return 0;
}

int RenderEngine::setGraphicsApiCanvas()
{
	// Defaults: RGBA, Z-depth 16 bits, double buffering, 1 sample buffer, 4 samplers.
	wxGLAttributes attribs = {};
	attribs.PlatformDefaults().Defaults().Samplers(16).EndList();

	RenderEngine::Canvas.Canvas = new wxGLCanvas(
		RenderEngine::Canvas.Window, attribs, ID_CANVAS,
		RenderEngine::Canvas.Position, RenderEngine::Canvas.Size
	);

	RenderEngine::Canvas.Window->SetCanvas(RenderEngine::Canvas.Canvas);
	RenderEngine::SetDrawMode(RenderEngine::Canvas.Window->SelectedDrawMode());

	RenderEngine::Canvas.GL = new wxGLContext(RenderEngine::Canvas.Canvas);

	if (!RenderEngine::Canvas.GL->IsOK())
		return -1;

	RenderEngine::Canvas.Canvas->SetCurrent(*RenderEngine::Canvas.GL);

	if (glewInit() != GLEW_OK)
		return -2;

	return 0;
}

int RenderEngine::setGraphicsApiDX(GraphicsAPI api)
{
	RenderEngine::Canvas.DX = new DXContext(api, RenderEngine::Canvas.Window->VSyncEnable->GetValue());

	if (!RenderEngine::Canvas.DX->IsOK()) {
		RenderEngine::Close();
		return -1;
	}

	return 0;
}

int RenderEngine::setGraphicsApiGL()
{
	glViewport(0, 0, RenderEngine::Canvas.Size.GetWidth(), RenderEngine::Canvas.Size.GetHeight());
		
	RenderEngine::SetVSync(RenderEngine::Canvas.Window->VSyncEnable->GetValue());

	glEnable(GL_MULTISAMPLE);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_TEXTURE_CUBE_MAP);

	RenderEngine::GPU.Renderer = glGetString(GL_RENDERER);
	RenderEngine::GPU.Vendor   = glGetString(GL_VENDOR);
	RenderEngine::GPU.Version  = wxString("OpenGL ").append(glGetString(GL_VERSION));

	return 0;
}

int RenderEngine::setGraphicsApiVK()
{
	RenderEngine::Canvas.VK = new VKContext(RenderEngine::Canvas.Window->VSyncEnable->GetValue());

	if (!RenderEngine::Canvas.VK->IsOK()) {
		RenderEngine::Close();
		return -1;
	}

	return 0;
}

ShaderProgram* RenderEngine::setShaderProgram(bool enable, ShaderID program)
{
	if (RenderEngine::SelectedGraphicsAPI == GRAPHICS_API_OPENGL)
		glUseProgram(enable ? ShaderManager::Programs[program]->Program() : 0);

	return (enable ? ShaderManager::Programs[program] : nullptr);
}

void RenderEngine::SetVSync(bool enable)
{
	switch (RenderEngine::SelectedGraphicsAPI) {
	#if defined _WINDOWS
	case GRAPHICS_API_DIRECTX11:
	case GRAPHICS_API_DIRECTX12:
		if (RenderEngine::Canvas.DX != nullptr)
			RenderEngine::Canvas.DX->SetVSync(enable);
		break;
	#endif
	case GRAPHICS_API_OPENGL:
	#if defined _WINDOWS
		if (std::strstr((char*)glGetString(GL_EXTENSIONS), "WGL_EXT_swap_control") > 0)
		{
			PFNWGLGETEXTENSIONSSTRINGEXTPROC wglGetExtensionsStringEXT = (PFNWGLGETEXTENSIONSSTRINGEXTPROC)wglGetProcAddress("wglGetExtensionsStringEXT");

			if (std::strstr(wglGetExtensionsStringEXT(), "WGL_EXT_swap_control") > 0) {
				PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");
				wglSwapIntervalEXT(enable ? 1 : 0);
			}
		}
	#else
		if (std::strstr((char*)glGetString(GL_EXTENSIONS), "GLX_EXT_swap_control") > 0) {
			PFNGLXSWAPINTERVALEXTPROC glXSwapIntervalEXT = (PFNGLXSWAPINTERVALEXTPROC)glXGetProcAddress("glXSwapIntervalEXT");
			glXSwapIntervalEXT(enable ? 1 : 0);
		}
	#endif
		break;
	case GRAPHICS_API_VULKAN:
		if (RenderEngine::Canvas.VK != nullptr)
			RenderEngine::Canvas.VK->SetVSync(enable);
		break;
	default:
		throw;
	}
}
