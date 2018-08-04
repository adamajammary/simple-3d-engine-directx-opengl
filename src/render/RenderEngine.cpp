#include "RenderEngine.h"

GLCanvas           RenderEngine::Canvas             = {};
uint16_t           RenderEngine::DrawMode           = 0;
Camera*            RenderEngine::Camera             = nullptr;
GPUDescription     RenderEngine::GPU                = {};
bool               RenderEngine::DrawBoundingVolume = false;
Mesh*              RenderEngine::Skybox             = nullptr;
std::vector<Mesh*> RenderEngine::HUDs;
bool               RenderEngine::Ready              = false;
std::vector<Mesh*> RenderEngine::Renderables;
std::vector<Mesh*> RenderEngine::Terrains;
std::vector<Mesh*> RenderEngine::Waters;

void RenderEngine::clear(float r, float g, float b, float a, FrameBuffer* fbo)
{
	switch (Utils::SelectedGraphicsAPI) {
	#if defined _WINDOWS
	case GRAPHICS_API_DIRECTX11:
		RenderEngine::Canvas.DX->Clear11(r, g, b, a, fbo);
		break;
	case GRAPHICS_API_DIRECTX12:
		RenderEngine::Canvas.DX->Clear12(r, g, b, a, fbo);
		break;
	#endif
	case GRAPHICS_API_OPENGL:
		glClearColor(r, g, b, a);
		glClearStencil(0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		break;
	case GRAPHICS_API_VULKAN:
		RenderEngine::Canvas.VK->Clear(r, g, b, a);
		break;
	}
}

void RenderEngine::Close()
{
	InputManager::Reset();
	SceneManager::Clear();
	ShaderManager::Close();

	_DELETEP(Utils::EmptyCubemap);
	_DELETEP(Utils::EmptyTexture);

	_DELETEP(RenderEngine::Canvas.DX);
	_DELETEP(RenderEngine::Canvas.GL);
	_DELETEP(RenderEngine::Canvas.VK);

	if (RenderEngine::Canvas.Canvas != nullptr) {
		RenderEngine::Canvas.Canvas->DestroyChildren();
		RenderEngine::Canvas.Canvas->Destroy();
	}
}

void RenderEngine::Draw()
{
	// WATER RENDER PASSES
	for (auto water : RenderEngine::Waters)
	{
		if (water == nullptr)
			continue;

		Water* parent = dynamic_cast<Water*>(water->Parent);

		if (water->Parent == nullptr)
			continue;

		glm::vec3 position       = water->Position();
		glm::vec3 scale          = water->Scale();
		float     cameraDistance = ((RenderEngine::Camera->Position().y - position.y) * 2.0f);

		// WATER REFLECTION PASS - ABOVE WATER
		RenderEngine::Camera->MoveBy(glm::vec3(0.0f, -cameraDistance, 0.0f));
		RenderEngine::Camera->InvertPitch();

		parent->FBO()->BindReflection();

		glm::vec3 clipMax = glm::vec3(scale.x,  scale.y,     scale.z);
		glm::vec3 clipMin = glm::vec3(-scale.x, position.y, -scale.z);

		RenderEngine::clear(0.0f, 0.0f, 1.0f, 1.0f, parent->FBO()->ReflectionFBO());
		RenderEngine::drawSkybox(true,      clipMax, clipMin);
		RenderEngine::drawTerrains(true,    clipMax, clipMin);
		RenderEngine::drawRenderables(true, clipMax, clipMin);
		
		parent->FBO()->UnbindReflection();

		RenderEngine::Camera->InvertPitch();
		RenderEngine::Camera->MoveBy(glm::vec3(0.0, cameraDistance, 0.0));

		// WATER REFRACTION PASS - BELOW WATER
		parent->FBO()->BindRefraction();

		clipMax = glm::vec3(scale.x,   position.y, scale.z);
		clipMin = glm::vec3(-scale.x, -scale.y,   -scale.z);

		RenderEngine::clear(0.0f, 0.0f, 1.0f, 1.0f, parent->FBO()->RefractionFBO());
		RenderEngine::drawSkybox(true,      clipMax, clipMin);
		RenderEngine::drawTerrains(true,    clipMax, clipMin);
		RenderEngine::drawRenderables(true, clipMax, clipMin);

		parent->FBO()->UnbindRefraction();
	}
    
    // DEFAULT RENDER PASS
    RenderEngine::clear(0.0f, 0.2f, 0.4f, 1.0f);
	RenderEngine::drawScene();

	switch (Utils::SelectedGraphicsAPI) {
		#if defined _WINDOWS
		case GRAPHICS_API_DIRECTX11: RenderEngine::Canvas.DX->Present11();       break;
		case GRAPHICS_API_DIRECTX12: RenderEngine::Canvas.DX->Present12();       break;
		#endif
		case GRAPHICS_API_OPENGL:    RenderEngine::Canvas.Canvas->SwapBuffers(); break;
		case GRAPHICS_API_VULKAN:    RenderEngine::Canvas.VK->Present();         break;
	}
}

int RenderEngine::drawBoundingVolumes()
{
    if (!RenderEngine::DrawBoundingVolume)
		return -1;

	uint16_t oldDrawMode = RenderEngine::DrawMode;

	switch (Utils::SelectedGraphicsAPI) {
	#if defined _WINDOWS
	case GRAPHICS_API_DIRECTX11:
	case GRAPHICS_API_DIRECTX12:
		RenderEngine::DrawMode = D3D_PRIMITIVE_TOPOLOGY_LINESTRIP;
		break;
	#endif
	case GRAPHICS_API_OPENGL:
		RenderEngine::DrawMode = GL_LINE_STRIP;

		glEnable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
		glDisable(GL_BLEND);
		break;
	case GRAPHICS_API_VULKAN:
		break;
	default:
		break;
	}

	ShaderProgram* shaderProgram = RenderEngine::setShaderProgram(true, SHADER_ID_SOLID);

	for (auto renderable : RenderEngine::Renderables)
        RenderEngine::drawMesh(renderable->GetBoundingVolume(), shaderProgram);

	RenderEngine::DrawMode = oldDrawMode;
	RenderEngine::setShaderProgram(false);

    return 0;
}    

int RenderEngine::drawHUDs(bool enableClipping, const glm::vec3 &clipMax, const glm::vec3 &clipMin)
{
	if (RenderEngine::HUDs.empty())
		return -1;

	switch (Utils::SelectedGraphicsAPI) {
	#if defined _WINDOWS
	case GRAPHICS_API_DIRECTX11:
		break;
	case GRAPHICS_API_DIRECTX12:
		break;
	#endif
	case GRAPHICS_API_OPENGL:
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE); glCullFace(GL_BACK); glFrontFace(GL_CCW);
		glEnable(GL_BLEND);     glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		break;
	case GRAPHICS_API_VULKAN:
		break;
	default:
		break;
	}

	ShaderProgram* shaderProgram = RenderEngine::setShaderProgram(true, SHADER_ID_HUD);

	for (auto hud : RenderEngine::HUDs)
		RenderEngine::drawMesh(hud, shaderProgram, enableClipping, clipMax, clipMin);

	RenderEngine::setShaderProgram(false);

	return 0;
}

int RenderEngine::drawRenderables(bool enableClipping, const glm::vec3 &clipMax, const glm::vec3 &clipMin)
{
	// TODO: VULKAN
	if (RenderEngine::Renderables.empty())
		return -1;

	switch (Utils::SelectedGraphicsAPI) {
	#if defined _WINDOWS
	case GRAPHICS_API_DIRECTX11:
	case GRAPHICS_API_DIRECTX12:
		break;
	#endif
	case GRAPHICS_API_OPENGL:
		glEnable(GL_DEPTH_TEST); glDepthFunc(GL_LESS);
		glEnable(GL_CULL_FACE);  glCullFace(GL_BACK); glFrontFace(GL_CCW);
		glDisable(GL_BLEND);
		break;
	case GRAPHICS_API_VULKAN:
		//RenderEngine::Canvas.Vulkan->Draw(nullptr, ShaderManager::Programs[SHADER_ID_DEFAULT]);
		break;
	default:
		break;
	}

	ShaderProgram* shaderProgram = RenderEngine::setShaderProgram(true, SHADER_ID_DEFAULT);

	for (auto renderable : RenderEngine::Renderables)
		RenderEngine::drawMesh(renderable, shaderProgram, enableClipping, clipMax, clipMin);

	RenderEngine::setShaderProgram(false);

	return 0;
}

int RenderEngine::drawSelected()
{
	Mesh* selectedMesh = nullptr;

	for (auto mesh : RenderEngine::Renderables) {
		if (mesh->IsSelected()) {
			selectedMesh = mesh;
			break;
		}
	}

	if (selectedMesh == nullptr)
		return -1;

	uint16_t oldDrawMode = RenderEngine::DrawMode;

	switch (Utils::SelectedGraphicsAPI) {
	#if defined _WINDOWS
	case GRAPHICS_API_DIRECTX11:
	case GRAPHICS_API_DIRECTX12:
		RenderEngine::DrawMode = D3D_PRIMITIVE_TOPOLOGY_LINESTRIP;
		break;
	#endif
	case GRAPHICS_API_OPENGL:
		RenderEngine::DrawMode = GL_LINE_STRIP;

		glDisable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
		glDisable(GL_BLEND);
		break;
	case GRAPHICS_API_VULKAN:
		break;
	default:
		break;
	}

	ShaderProgram* shaderProgram = RenderEngine::setShaderProgram(true, SHADER_ID_SOLID);

	RenderEngine::drawMesh(selectedMesh, shaderProgram);

	RenderEngine::setShaderProgram(false);
	RenderEngine::DrawMode = oldDrawMode;

	return 0;
}

int RenderEngine::drawSkybox(bool enableClipping, const glm::vec3 &clipMax, const glm::vec3 &clipMin)
{
	if (RenderEngine::Skybox == nullptr)
		return -1;

	switch (Utils::SelectedGraphicsAPI) {
	#if defined _WINDOWS
	case GRAPHICS_API_DIRECTX11:
		break;
	case GRAPHICS_API_DIRECTX12:
		break;
	#endif
	case GRAPHICS_API_OPENGL:
		glEnable(GL_DEPTH_TEST); glDepthFunc(GL_LEQUAL);
		glDisable(GL_CULL_FACE);
		glDisable(GL_BLEND);
		break;
	case GRAPHICS_API_VULKAN:
		break;
	default:
		break;
	}

	ShaderProgram* shaderProgram = RenderEngine::setShaderProgram(true, SHADER_ID_SKYBOX);
    RenderEngine::drawMesh(RenderEngine::Skybox, shaderProgram, enableClipping, clipMax, clipMin);
	RenderEngine::setShaderProgram(false);

	return 0;
}

int RenderEngine::drawTerrains(bool enableClipping, const glm::vec3 &clipMax, const glm::vec3 &clipMin)
{
	if (RenderEngine::Terrains.empty())
		return -1;

	switch (Utils::SelectedGraphicsAPI) {
	#if defined _WINDOWS
	case GRAPHICS_API_DIRECTX11:
		break;
	case GRAPHICS_API_DIRECTX12:
		break;
	#endif
	case GRAPHICS_API_OPENGL:
		glEnable(GL_DEPTH_TEST); glDepthFunc(GL_LESS);
		glEnable(GL_CULL_FACE);  glCullFace(GL_BACK); glFrontFace(GL_CCW);
		glDisable(GL_BLEND);
		break;
	case GRAPHICS_API_VULKAN:
		break;
	default:
		break;
	}

	ShaderProgram* shaderProgram = RenderEngine::setShaderProgram(true, SHADER_ID_TERRAIN);

	for (auto terrain : RenderEngine::Terrains)
		RenderEngine::drawMesh(terrain, shaderProgram, enableClipping, clipMax, clipMin);

	RenderEngine::setShaderProgram(false);

	return 0;
}

int RenderEngine::drawWaters(bool enableClipping, const glm::vec3 &clipMax, const glm::vec3 &clipMin)
{
	if (RenderEngine::Waters.empty())
		return -1;

	switch (Utils::SelectedGraphicsAPI) {
	#if defined _WINDOWS
	case GRAPHICS_API_DIRECTX11:
		break;
	case GRAPHICS_API_DIRECTX12:
		break;
	#endif
	case GRAPHICS_API_OPENGL:
		glEnable(GL_DEPTH_TEST); glDepthFunc(GL_LESS);
		glEnable(GL_CULL_FACE);  glCullFace(GL_BACK); glFrontFace(GL_CCW);
		glDisable(GL_BLEND);
		break;
	case GRAPHICS_API_VULKAN:
		break;
	default:
		break;
	}

	ShaderProgram* shaderProgram = RenderEngine::setShaderProgram(true, SHADER_ID_WATER);

	for (auto water : RenderEngine::Waters)
		RenderEngine::drawMesh(water, shaderProgram, enableClipping, clipMax, clipMin);

	RenderEngine::setShaderProgram(false);

	return 0;
}
    
void RenderEngine::drawScene(bool enableClipping, const glm::vec3 &clipMax, const glm::vec3 &clipMin)
{
	RenderEngine::drawSelected();
	RenderEngine::drawBoundingVolumes();
	RenderEngine::drawSkybox(enableClipping,      clipMax, clipMin);
	RenderEngine::drawTerrains(enableClipping,    clipMax, clipMin);
	RenderEngine::drawWaters(enableClipping,      clipMax, clipMin);
	RenderEngine::drawRenderables(enableClipping, clipMax, clipMin);
    RenderEngine::drawHUDs(enableClipping,        clipMax, clipMin);
}

void RenderEngine::drawMesh(Mesh* mesh, ShaderProgram* shaderProgram, bool enableClipping, const glm::vec3 &clipMax, const glm::vec3 &clipMin)
{
	switch (Utils::SelectedGraphicsAPI) {
	#if defined _WINDOWS
	case GRAPHICS_API_DIRECTX11:
		RenderEngine::drawMeshDX11(mesh, shaderProgram, enableClipping, clipMax, clipMin);
		break;
	case GRAPHICS_API_DIRECTX12:
		RenderEngine::drawMeshDX12(mesh, shaderProgram, enableClipping, clipMax, clipMin);
		break;
	#endif
	case GRAPHICS_API_OPENGL:
		RenderEngine::drawMeshGL(mesh, shaderProgram, enableClipping, clipMax, clipMin);
		break;
	case GRAPHICS_API_VULKAN:
		RenderEngine::drawMeshVulkan(mesh, shaderProgram, enableClipping, clipMax, clipMin);
		break;
	default:
		break;
	}
}

int RenderEngine::drawMeshDX11(Mesh* mesh, ShaderProgram* shaderProgram, bool enableClipping, const glm::vec3 &clipMax, const glm::vec3 &clipMin)
{
	return RenderEngine::Canvas.DX->Draw11(mesh, shaderProgram, enableClipping, clipMax, clipMin);
}

int RenderEngine::drawMeshDX12(Mesh* mesh, ShaderProgram* shaderProgram, bool enableClipping, const glm::vec3 &clipMax, const glm::vec3 &clipMin)
{
	return RenderEngine::Canvas.DX->Draw12(mesh, shaderProgram, enableClipping, clipMax, clipMin);
}

int RenderEngine::drawMeshGL(Mesh* mesh, ShaderProgram* shaderProgram, bool enableClipping, const glm::vec3 &clipMax, const glm::vec3 &clipMin)
{
    if ((RenderEngine::Camera == nullptr) || (shaderProgram == nullptr) || (shaderProgram->Program() < 1) || (mesh == nullptr) || (mesh->IBO() < 1))
		return -1;

	shaderProgram->UpdateAttribsGL(mesh);
	shaderProgram->UpdateUniformsGL(mesh, enableClipping, clipMax, clipMin);
       
    // DRAW
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->IBO());
		glDrawElements(RenderEngine::DrawMode, (GLsizei)mesh->NrOfIndices(), GL_UNSIGNED_INT, nullptr);
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

int RenderEngine::drawMeshVulkan(Mesh* mesh, ShaderProgram* shaderProgram, bool enableClipping, const glm::vec3 &clipMax, const glm::vec3 &clipMin)
{
	return RenderEngine::Canvas.VK->Draw(mesh, shaderProgram, enableClipping, clipMax, clipMin);
}

int RenderEngine::Init(WindowFrame* window, const wxSize &size)
{
	int result;

	RenderEngine::Canvas.AspectRatio = (float)((float)size.GetHeight() / (float)size.GetWidth());
	RenderEngine::Canvas.Position    = wxDefaultPosition;
	RenderEngine::Canvas.Size        = size;
	RenderEngine::Canvas.Window      = window;

	#if defined _WINDOWS
		result = RenderEngine::SetGraphicsAPI(GRAPHICS_API_DIRECTX11);
	#else
		result = RenderEngine::SetGraphicsAPI(GRAPHICS_API_OPENGL);
	#endif

	if (result != 0)
		return -1;

	return 0;
}

int RenderEngine::initResources()
{
	wxString              emptyFile  = Utils::RESOURCE_IMAGES["emptyTexture"];
	std::vector<wxString> emptyFiles = { emptyFile, emptyFile, emptyFile, emptyFile, emptyFile, emptyFile };

	Utils::EmptyTexture = new Texture(emptyFile);
	Utils::EmptyCubemap = new Texture(emptyFiles);

	// TODO: FIX FOR VULKAN
	if (Utils::SelectedGraphicsAPI == GRAPHICS_API_VULKAN)
		return 0;
	
	if (!Utils::EmptyTexture->IsOK() || !Utils::EmptyCubemap->IsOK())
		return -1;

	return 0;
}

void RenderEngine::RemoveMesh(Mesh* mesh)
{
    if (mesh->Parent->Type() == COMPONENT_HUD) {
		auto index = std::find(RenderEngine::HUDs.begin(), RenderEngine::HUDs.end(), mesh);

        if (index != RenderEngine::HUDs.end())
			RenderEngine::HUDs.erase(index);
    } else if (mesh->Parent->Type() == COMPONENT_SKYBOX) {
        RenderEngine::Skybox = nullptr;
    } else if (mesh->Parent->Type() == COMPONENT_TERRAIN) {
		auto index = std::find(RenderEngine::Terrains.begin(), RenderEngine::Terrains.end(), mesh);

		if (index != RenderEngine::Terrains.end())
			RenderEngine::Terrains.erase(index);
    } else if (mesh->Parent->Type() == COMPONENT_WATER) {
		auto index = std::find(RenderEngine::Waters.begin(), RenderEngine::Waters.end(), mesh);

		if (index != RenderEngine::Waters.end())
			RenderEngine::Waters.erase(index);
    } else {
		auto index = std::find(RenderEngine::Renderables.begin(), RenderEngine::Renderables.end(), mesh);

		if (index != RenderEngine::Renderables.end())
			RenderEngine::Renderables.erase(index);
    }
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
		RenderEngine::Canvas.VK->UpdateSwapChain();
}

void RenderEngine::SetCanvasSize(int width, int height)
{
	RenderEngine::Canvas.Size = wxSize(width, height);
	RenderEngine::Canvas.Canvas->SetSize(RenderEngine::Canvas.Size);
	RenderEngine::Camera->UpdateProjection();
	glViewport(0, 0, width, height);
}

void RenderEngine::SetDrawMode(const wxString &mode)
{
	switch (Utils::SelectedGraphicsAPI) {
	#if defined _WINDOWS
	case GRAPHICS_API_DIRECTX11:
	case GRAPHICS_API_DIRECTX12:
		if (mode == Utils::DRAW_MODES[DRAW_MODE_FILLED])
			RenderEngine::DrawMode = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		else if (mode == Utils::DRAW_MODES[DRAW_MODE_WIREFRAME])
			RenderEngine::DrawMode = D3D_PRIMITIVE_TOPOLOGY_LINESTRIP;

		break;
	#endif
	case GRAPHICS_API_OPENGL:
		if (mode == Utils::DRAW_MODES[DRAW_MODE_FILLED])
			RenderEngine::DrawMode = GL_TRIANGLES;
		else if (mode == Utils::DRAW_MODES[DRAW_MODE_WIREFRAME])
			RenderEngine::DrawMode = GL_LINE_STRIP;

		break;
	}
}

int RenderEngine::SetGraphicsAPI(const wxString &api)
{
	GraphicsAPI lastAPI = Utils::SelectedGraphicsAPI;
	int         result  = -1;

	if (api == "DirectX 11")
		result = RenderEngine::SetGraphicsAPI(GRAPHICS_API_DIRECTX11);
	else if (api == "DirectX 12")
		result = RenderEngine::SetGraphicsAPI(GRAPHICS_API_DIRECTX12);
	else if (api == "OpenGL")
		result = RenderEngine::SetGraphicsAPI(GRAPHICS_API_OPENGL);
	else if (api == "Vulkan")
		result = RenderEngine::SetGraphicsAPI(GRAPHICS_API_VULKAN);

	if (result != 0)
	{
		wxMessageBox("ERROR: Failed to initialize the " + api + " graphics engine.");

		RenderEngine::SetGraphicsAPI(lastAPI);
		RenderEngine::Canvas.Window->SetGraphicsAPI(lastAPI);
	}

	return result;
}

int RenderEngine::SetGraphicsAPI(GraphicsAPI api)
{
	int result                 = -1;
	RenderEngine::Ready        = false;
	Utils::SelectedGraphicsAPI = api;

	// CLEAR SCENE AND FREE MEMORY
	RenderEngine::Close();

	// RE-CREATE THE CANVAS
	wxGLAttributes attribs;

	attribs.Defaults();
	attribs.EndList();

	RenderEngine::Canvas.Canvas = new wxGLCanvas(
		RenderEngine::Canvas.Window, attribs, ID_CANVAS, RenderEngine::Canvas.Position, RenderEngine::Canvas.Size
	);

	RenderEngine::Canvas.Window->SetCanvas(RenderEngine::Canvas.Canvas);

	// RE-CREATE THE GRAPHICS CONTEXT
	switch (api) {
	#if defined _WINDOWS
	case GRAPHICS_API_DIRECTX11:
	case GRAPHICS_API_DIRECTX12:
		RenderEngine::Canvas.DX = new DXContext(api, RenderEngine::Canvas.Window->VSyncEnable->GetValue());

		if (!RenderEngine::Canvas.DX->IsOK())
			return -2;

		break;
	#endif
	case GRAPHICS_API_OPENGL:
		RenderEngine::Canvas.GL = new wxGLContext(RenderEngine::Canvas.Canvas);

		if (!RenderEngine::Canvas.GL->IsOK())
			return -2;

		RenderEngine::Canvas.Canvas->SetCurrent(*RenderEngine::Canvas.GL);

		glewInit();
		glViewport(0, 0, RenderEngine::Canvas.Size.GetWidth(), RenderEngine::Canvas.Size.GetHeight());
		
		RenderEngine::SetVSync(RenderEngine::Canvas.Window->VSyncEnable->GetValue());
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_TEXTURE_CUBE_MAP);

		RenderEngine::GPU.Renderer = glGetString(GL_RENDERER);
		RenderEngine::GPU.Vendor   = glGetString(GL_VENDOR);
		RenderEngine::GPU.Version  = wxString("OpenGL ").append(glGetString(GL_VERSION));

		break;
	case GRAPHICS_API_VULKAN:
		RenderEngine::Canvas.VK = new VKContext(api, RenderEngine::Canvas.Window->VSyncEnable->GetValue());

		if (!RenderEngine::Canvas.VK->IsOK())
			return -2;

		break;
	default:
		Utils::SelectedGraphicsAPI = GRAPHICS_API_UNKNOWN;
		RenderEngine::Canvas.Window->SetStatusText("GRAPHICS_API_UNKNOWN: INVALID API");
		return -2;
	}

	// RE-INITIALIZE ENGINE MODULES AND RESOURCES
	if (InputManager::Init() != 0)
		return -3;

	if (ShaderManager::Init() != 0)
		return -4;

	if (RenderEngine::initResources() != 0)
		return -5;

	RenderEngine::SetDrawMode(RenderEngine::Canvas.Window->SelectedDrawMode());

	RenderEngine::Ready = true;

	return 0;
}

ShaderProgram* RenderEngine::setShaderProgram(bool enable, ShaderID program)
{
	if (Utils::SelectedGraphicsAPI == GRAPHICS_API_OPENGL)
		glUseProgram(enable ? ShaderManager::Programs[program]->Program() : 0);

	return (enable ? ShaderManager::Programs[program] : nullptr);
}

void RenderEngine::SetVSync(bool enable)
{
	switch (Utils::SelectedGraphicsAPI) {
	#if defined _WINDOWS
	case GRAPHICS_API_DIRECTX11:
	case GRAPHICS_API_DIRECTX12:
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
		break;
	default:
		break;
	}
}
