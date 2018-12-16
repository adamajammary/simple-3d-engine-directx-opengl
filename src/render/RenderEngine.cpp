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
std::vector<Component*> RenderEngine::Terrains;
std::vector<Component*> RenderEngine::Waters;

void RenderEngine::clear(float r, float g, float b, float a, FrameBuffer* fbo, VkCommandBuffer cmdBuffer)
{
	switch (RenderEngine::SelectedGraphicsAPI) {
	#if defined _WINDOWS
	case GRAPHICS_API_DIRECTX11:
		if (RenderEngine::Canvas.DX != nullptr)
			RenderEngine::Canvas.DX->Clear11(r, g, b, a, fbo);
		break;
	case GRAPHICS_API_DIRECTX12:
		if (RenderEngine::Canvas.DX != nullptr)
			RenderEngine::Canvas.DX->Clear12(r, g, b, a, fbo);
		break;
	#endif
	case GRAPHICS_API_OPENGL:
		glClearColor(r, g, b, a);
		glClearStencil(0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		break;
	case GRAPHICS_API_VULKAN:
		if (RenderEngine::Canvas.VK != nullptr)
			RenderEngine::Canvas.VK->Clear(r, g, b, a, fbo, cmdBuffer);
		break;
	}
}

void RenderEngine::Close()
{
	InputManager::Reset();
	SceneManager::Clear();
	ShaderManager::Close();

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

void RenderEngine::createWaterFBOs()
{
	for (auto water : RenderEngine::Waters)
	{
		if (water == nullptr)
			continue;

		Water* parent = dynamic_cast<Water*>(water->Parent);

		if (parent == nullptr)
			continue;

		glm::vec3       position       = water->Position();
		glm::vec3       scale          = water->Scale();
		float           cameraDistance = ((RenderEngine::CameraMain->Position().y - position.y) * 2.0f);
		VkCommandBuffer cmdBuffer      = nullptr;

		// WATER REFLECTION PASS - ABOVE WATER
		RenderEngine::CameraMain->MoveBy(glm::vec3(0.0f, -cameraDistance, 0.0f));
		RenderEngine::CameraMain->InvertPitch();

		if (RenderEngine::SelectedGraphicsAPI == GRAPHICS_API_VULKAN)
			cmdBuffer = RenderEngine::Canvas.VK->CommandBufferBegin();
		else
			parent->FBO()->BindReflection();

		DrawProperties drawProperties = {};

		drawProperties.EnableClipping = true;
		drawProperties.FBO            = true;
		drawProperties.ClipMax        = glm::vec3(scale.x, scale.y, scale.z);
		drawProperties.ClipMin        = glm::vec3(-scale.x, position.y, -scale.z);

		RenderEngine::clear(0.0f, 0.0f, 1.0f, 1.0f, parent->FBO()->ReflectionFBO(), cmdBuffer);

		RenderEngine::drawSkybox(drawProperties,      cmdBuffer);
		RenderEngine::drawTerrains(drawProperties,    cmdBuffer);
		RenderEngine::drawRenderables(drawProperties, cmdBuffer);
		
		if (RenderEngine::SelectedGraphicsAPI == GRAPHICS_API_VULKAN)
			RenderEngine::Canvas.VK->Present(cmdBuffer);
		else
			parent->FBO()->UnbindReflection();

		RenderEngine::CameraMain->InvertPitch();
		RenderEngine::CameraMain->MoveBy(glm::vec3(0.0, cameraDistance, 0.0));

		// WATER REFRACTION PASS - BELOW WATER
		if (RenderEngine::SelectedGraphicsAPI == GRAPHICS_API_VULKAN)
			cmdBuffer = RenderEngine::Canvas.VK->CommandBufferBegin();
		else
			parent->FBO()->BindRefraction();

		drawProperties.ClipMax = glm::vec3(scale.x,   position.y, scale.z);
		drawProperties.ClipMin = glm::vec3(-scale.x, -scale.y,   -scale.z);

		RenderEngine::clear(0.0f, 0.0f, 1.0f, 1.0f, parent->FBO()->RefractionFBO(), cmdBuffer);

		RenderEngine::drawSkybox(drawProperties,      cmdBuffer);
		RenderEngine::drawTerrains(drawProperties,    cmdBuffer);
		RenderEngine::drawRenderables(drawProperties, cmdBuffer);

		if (RenderEngine::SelectedGraphicsAPI == GRAPHICS_API_VULKAN)
			RenderEngine::Canvas.VK->Present(cmdBuffer);
		else
			parent->FBO()->UnbindRefraction();
	}
}

void RenderEngine::Draw()
{
	RenderEngine::createWaterFBOs();
	RenderEngine::clear(0.0f, 0.2f, 0.4f, 1.0f);
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

	RenderEngine::drawMeshes(RenderEngine::Renderables, SHADER_ID_WIREFRAME, properties);

	RenderEngine::drawMode = oldDrawMode;

    return 0;
}    

int RenderEngine::drawHUDs(const DrawProperties &properties)
{
	if (RenderEngine::HUDs.empty())
		return 1;

	if (RenderEngine::SelectedGraphicsAPI == GRAPHICS_API_OPENGL)
	{
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
		glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	ShaderID shaderID = (RenderEngine::drawMode == DRAW_MODE_FILLED ? SHADER_ID_HUD : SHADER_ID_WIREFRAME);

	RenderEngine::drawMeshes(RenderEngine::HUDs, shaderID, properties);

	return 0;
}

int RenderEngine::drawLightSources(const DrawProperties &properties)
{
	if (RenderEngine::LightSources.empty())
		return 1;

	if (RenderEngine::SelectedGraphicsAPI == GRAPHICS_API_OPENGL)
	{
		glEnable(GL_DEPTH_TEST); glDepthFunc(GL_LESS);
		glEnable(GL_CULL_FACE);  glCullFace(GL_BACK); glFrontFace(GL_CCW);
		glDisable(GL_BLEND);
	}

	ShaderID shaderID = (RenderEngine::drawMode == DRAW_MODE_FILLED ? SHADER_ID_COLOR : SHADER_ID_WIREFRAME);

	RenderEngine::drawMeshes(RenderEngine::LightSources, shaderID, properties);

	return 0;
}

int RenderEngine::drawRenderables(const DrawProperties &properties, VkCommandBuffer cmdBuffer)
{
	if (RenderEngine::Renderables.empty())
		return 1;

	if (RenderEngine::SelectedGraphicsAPI == GRAPHICS_API_OPENGL)
	{
		glEnable(GL_DEPTH_TEST); glDepthFunc(GL_LESS);
		glEnable(GL_CULL_FACE);  glCullFace(GL_BACK); glFrontFace(GL_CCW);
		glDisable(GL_BLEND);
	}

	ShaderID shaderID = (RenderEngine::drawMode == DRAW_MODE_FILLED ? SHADER_ID_DEFAULT : SHADER_ID_WIREFRAME);

	RenderEngine::drawMeshes(RenderEngine::Renderables, shaderID, properties, cmdBuffer);

	return 0;
}

int RenderEngine::drawSelected()
{
	DrawModeType oldDrawMode = RenderEngine::drawMode;

	RenderEngine::SetDrawMode(DRAW_MODE_WIREFRAME);

	DrawProperties properties = {};
	properties.DrawSelected   = true;

	if (RenderEngine::SelectedGraphicsAPI == GRAPHICS_API_OPENGL)
	{
		glEnable(GL_DEPTH_TEST); glDepthFunc(GL_LESS);
		glEnable(GL_CULL_FACE);  glCullFace(GL_BACK); glFrontFace(GL_CCW);
		glDisable(GL_BLEND);
	}

	RenderEngine::drawMeshes(RenderEngine::Renderables, SHADER_ID_WIREFRAME, properties);

	RenderEngine::drawMode = oldDrawMode;

	return 0;
}

int RenderEngine::drawSkybox(const DrawProperties &properties, VkCommandBuffer cmdBuffer)
{
	if (RenderEngine::Skybox == nullptr)
		return 1;

	if (RenderEngine::SelectedGraphicsAPI == GRAPHICS_API_OPENGL)
	{
		glEnable(GL_DEPTH_TEST); glDepthFunc(GL_LEQUAL);
		glDisable(GL_CULL_FACE);
		glDisable(GL_BLEND);
	}

	ShaderID shaderID = (RenderEngine::drawMode == DRAW_MODE_FILLED ? SHADER_ID_SKYBOX : SHADER_ID_WIREFRAME);

	RenderEngine::drawMeshes({ RenderEngine::Skybox }, shaderID, properties, cmdBuffer);

	return 0;
}

int RenderEngine::drawTerrains(const DrawProperties &properties, VkCommandBuffer cmdBuffer)
{
	if (RenderEngine::Terrains.empty())
		return 1;

	if (RenderEngine::SelectedGraphicsAPI == GRAPHICS_API_OPENGL)
	{
		glEnable(GL_DEPTH_TEST); glDepthFunc(GL_LESS);
		glEnable(GL_CULL_FACE);  glCullFace(GL_BACK); glFrontFace(GL_CCW);
		glDisable(GL_BLEND);
	}

	ShaderID shaderID = (RenderEngine::drawMode == DRAW_MODE_FILLED ? SHADER_ID_TERRAIN : SHADER_ID_WIREFRAME);

	RenderEngine::drawMeshes(RenderEngine::Terrains, shaderID, properties, cmdBuffer);

	return 0;
}

int RenderEngine::drawWaters(const DrawProperties &properties)
{
	if (RenderEngine::Waters.empty())
		return 1;

	if (RenderEngine::SelectedGraphicsAPI == GRAPHICS_API_OPENGL)
	{
		glEnable(GL_DEPTH_TEST); glDepthFunc(GL_LESS);
		glEnable(GL_CULL_FACE);  glCullFace(GL_BACK); glFrontFace(GL_CCW);
		glDisable(GL_BLEND);
	}

	ShaderID shaderID = (RenderEngine::drawMode == DRAW_MODE_FILLED ? SHADER_ID_WATER : SHADER_ID_WIREFRAME);

	RenderEngine::drawMeshes(RenderEngine::Waters, shaderID, properties);

	return 0;
}

void RenderEngine::drawMesh(Component* mesh, ShaderProgram* shaderProgram, const DrawProperties &properties, VkCommandBuffer cmdBuffer)
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
			RenderEngine::drawMeshVK(mesh, shaderProgram, properties, cmdBuffer);
			break;
		default:
			throw;
	}
}

int RenderEngine::drawMeshDX11(Component* mesh, ShaderProgram* shaderProgram, const DrawProperties &properties)
{
	return RenderEngine::Canvas.DX->Draw11(mesh, shaderProgram, properties);
}

int RenderEngine::drawMeshDX12(Component* mesh, ShaderProgram* shaderProgram, const DrawProperties &properties)
{
	return RenderEngine::Canvas.DX->Draw12(mesh, shaderProgram, properties);
}

int RenderEngine::drawMeshGL(Component* mesh, ShaderProgram* shaderProgram, const DrawProperties &properties)
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
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, dynamic_cast<Mesh*>(mesh)->IBO());
	//glEnable(GL_FRAMEBUFFER_SRGB);
	glDrawElements(RenderEngine::GetDrawMode(), (GLsizei)dynamic_cast<Mesh*>(mesh)->NrOfIndices(), GL_UNSIGNED_INT, nullptr);
	//glDisable(GL_FRAMEBUFFER_SRGB);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	//glDrawArrays(RenderEngine::DrawMode, 0, (GLsizei)mesh->NrOfVertices());

    // UNBIND TEXTURES
    for (int i = 0; i < MAX_TEXTURES; i++) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D,       0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    }

    glActiveTexture(GL_TEXTURE0);

    return 0;
}

int RenderEngine::drawMeshVK(Component* mesh, ShaderProgram* shaderProgram, const DrawProperties &properties, VkCommandBuffer cmdBuffer)
{
	return RenderEngine::Canvas.VK->Draw(mesh, shaderProgram, properties, cmdBuffer);
}

void RenderEngine::drawMeshes(const std::vector<Component*> meshes, ShaderID shaderID, const DrawProperties &properties, VkCommandBuffer cmdBuffer)
{
	ShaderProgram* shaderProgram = RenderEngine::setShaderProgram(true, shaderID);

	for (auto mesh : meshes)
	{
		if (!properties.DrawBoundingVolume &&
			((properties.DrawSelected && !dynamic_cast<Mesh*>(mesh)->IsSelected()) ||
			(!properties.DrawSelected && dynamic_cast<Mesh*>(mesh)->IsSelected())))
		{
			continue;
		}
		
		glm::vec4 oldColor = mesh->ComponentMaterial.diffuse;

		if (properties.DrawSelected)
			mesh->ComponentMaterial.diffuse = SceneManager::SelectColor;

		RenderEngine::drawMesh(
			(properties.DrawBoundingVolume ? dynamic_cast<Mesh*>(mesh)->GetBoundingVolume() : mesh),
			shaderProgram, properties, cmdBuffer
		);

		if (properties.DrawSelected)
			mesh->ComponentMaterial.diffuse = oldColor;
	}

	RenderEngine::setShaderProgram(false);
}

void RenderEngine::drawScene(const DrawProperties &properties)
{
	RenderEngine::drawRenderables(properties);
	RenderEngine::drawLightSources(properties);
	RenderEngine::drawSelected();
	RenderEngine::drawBoundingVolumes();
	RenderEngine::drawSkybox(properties);
	RenderEngine::drawTerrains(properties);
	RenderEngine::drawWaters(properties);
    RenderEngine::drawHUDs(properties);
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
	case COMPONENT_TERRAIN:
		index = std::find(RenderEngine::Terrains.begin(), RenderEngine::Terrains.end(), mesh);

		if (index != RenderEngine::Terrains.end())
			RenderEngine::Terrains.erase(index);

		break;
	case COMPONENT_WATER:
		index = std::find(RenderEngine::Waters.begin(), RenderEngine::Waters.end(), mesh);

		if (index != RenderEngine::Waters.end())
			RenderEngine::Waters.erase(index);

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
