#ifndef GE3D_GLOBALS_H
	#include "../globals.h"
#endif

#ifndef GE3D_RENDERENGINE_H
#define GE3D_RENDERENGINE_H

class RenderEngine
{
private:
	RenderEngine()  {}
	~RenderEngine() {}

public:
	static uint16_t           DrawMode;
	static Camera*            Camera;
	static GLCanvas           Canvas;
	static GPUDescription     GPU;
	static bool               DrawBoundingVolume;
	static std::vector<Mesh*> HUDs;
	static bool               Ready;
	static std::vector<Mesh*> Renderables;
	static Mesh*              Skybox;
	static std::vector<Mesh*> Terrains;
	static std::vector<Mesh*> Waters;

public:
	static void Close();
	static void Draw();
	static int  Init(WindowFrame* window, const wxSize &size);
	static void RemoveMesh(Mesh* mesh);
	static void SetAspectRatio(const wxString &ratio);
	static void SetCanvasSize(int width, int height);
	static void SetDrawMode(const wxString &mode);
	static int  SetGraphicsAPI(const wxString &api);
	static void SetVSync(bool enable);

private:
	static void           clear(float r, float g, float b, float a, FrameBuffer* fbo = nullptr);
	static void           createWaterFBOs();
	static int            drawBoundingVolumes();
	static int            drawSelected();
	static int            drawHUDs(bool        enableClipping = false, const glm::vec3 &clipMax = {}, const glm::vec3 &clipMin = {});
	static int            drawRenderables(bool enableClipping = false, const glm::vec3 &clipMax = {}, const glm::vec3 &clipMin = {});
	static int            drawSkybox(bool      enableClipping = false, const glm::vec3 &clipMax = {}, const glm::vec3 &clipMin = {});
	static int            drawTerrains(bool    enableClipping = false, const glm::vec3 &clipMax = {}, const glm::vec3 &clipMin = {});
	static int            drawWaters(bool      enableClipping = false, const glm::vec3 &clipMax = {}, const glm::vec3 &clipMin = {});
	static void           drawMesh(Mesh*       mesh, ShaderProgram* shaderProgram, bool enableClipping = false, const glm::vec3 &clipMax = {}, const glm::vec3 &clipMin = {});
	static int            drawMeshDX11(Mesh*   mesh, ShaderProgram* shaderProgram, bool enableClipping = false, const glm::vec3 &clipMax = {}, const glm::vec3 &clipMin = {});
	static int            drawMeshDX12(Mesh*   mesh, ShaderProgram* shaderProgram, bool enableClipping = false, const glm::vec3 &clipMax = {}, const glm::vec3 &clipMin = {});
	static int            drawMeshGL(Mesh*     mesh, ShaderProgram* shaderProgram, bool enableClipping = false, const glm::vec3 &clipMax = {}, const glm::vec3 &clipMin = {});
	static int            drawMeshVulkan(Mesh* mesh, ShaderProgram* shaderProgram, bool enableClipping = false, const glm::vec3 &clipMax = {}, const glm::vec3 &clipMin = {});
	static void           drawMeshes(const std::vector<Mesh*> meshes, ShaderID shaderID, bool drawBoundingVolume = false, bool enableClipping = false, const glm::vec3 &clipMax = {}, const glm::vec3 &clipMin = {});
	static void           drawScene(bool enableClipping = false, const glm::vec3 &clipMax = {}, const glm::vec3 &clipMin = {});
	static int            initResources();
	static int            setGraphicsAPI(GraphicsAPI api);
	static int            setGraphicsApiDX(GraphicsAPI api);
	static int            setGraphicsApiGL();
	static int            setGraphicsApiVK();
	static ShaderProgram* setShaderProgram(bool enable, ShaderID program = SHADER_ID_UNKNOWN);

};

#endif
