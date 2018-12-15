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
	static Camera*                 CameraMain;
	static GLCanvas                Canvas;
	static GPUDescription          GPU;
	static bool                    DrawBoundingVolume;
	static std::vector<Component*> HUDs;
	static std::vector<Component*> LightSources;
	static bool                    Ready;
	static std::vector<Component*> Renderables;
	static GraphicsAPI             SelectedGraphicsAPI;
	static Mesh*                   Skybox;
	static std::vector<Component*> Terrains;
	static std::vector<Component*> Waters;

public:
	static DrawModeType drawMode;

public:
	static void     Close();
	static void     Draw();
	static uint16_t GetDrawMode();
	static int      Init(WindowFrame* window, const wxSize &size);
	static int      RemoveMesh(Component* mesh);
	static void     SetAspectRatio(const wxString &ratio);
	static void     SetCanvasSize(int width, int height);
	static void     SetDrawMode(DrawModeType mode);
	static void     SetDrawMode(const wxString &mode);
	static int      SetGraphicsAPI(const wxString &api);
	static void     SetVSync(bool enable);

private:
	static void           clear(float r, float g, float b, float a, FrameBuffer* fbo = nullptr, VkCommandBuffer cmdBuffer = nullptr);
	static void           createWaterFBOs();
	static int            drawBoundingVolumes();
	static int            drawSelected();
	static int            drawHUDs(const DrawProperties         &properties = {});
	static int            drawLightSources(const DrawProperties &properties = {});
	static int            drawRenderables(const DrawProperties  &properties = {}, VkCommandBuffer cmdBuffer = nullptr);
	static int            drawSkybox(const DrawProperties       &properties = {}, VkCommandBuffer cmdBuffer = nullptr);
	static int            drawTerrains(const DrawProperties     &properties = {}, VkCommandBuffer cmdBuffer = nullptr);
	static int            drawWaters(const DrawProperties       &properties = {});
	static int            drawMeshDX11(Component* mesh, ShaderProgram* shaderProgram, const DrawProperties &properties = {});
	static int            drawMeshDX12(Component* mesh, ShaderProgram* shaderProgram, const DrawProperties &properties = {});
	static int            drawMeshGL(Component*   mesh, ShaderProgram* shaderProgram, const DrawProperties &properties = {});
	static int            drawMeshVK(Component*   mesh, ShaderProgram* shaderProgram, const DrawProperties &properties = {}, VkCommandBuffer cmdBuffer = nullptr);
	static void           drawMesh(Component*     mesh, ShaderProgram* shaderProgram, const DrawProperties &properties = {}, VkCommandBuffer cmdBuffer = nullptr);
	static void           drawMeshes(const std::vector<Component*> meshes, ShaderID shaderID, const DrawProperties &properties = {}, VkCommandBuffer cmdBuffer = nullptr);
	static void           drawScene(const DrawProperties &properties = {});
	static int            initResources();
	static int            setGraphicsAPI(GraphicsAPI api);
	static int            setGraphicsApiCanvas();
	static int            setGraphicsApiDX(GraphicsAPI api);
	static int            setGraphicsApiGL();
	static int            setGraphicsApiVK();
	static ShaderProgram* setShaderProgram(bool enable, ShaderID program = SHADER_ID_UNKNOWN);

};

#endif
