#ifndef S3DE_GLOBALS_H
	#include "../globals.h"
#endif

#ifndef S3DE_RENDERENGINE_H
#define S3DE_RENDERENGINE_H

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
	static bool                    EnableSRGB;
	static std::vector<Component*> HUDs;
	static std::vector<Component*> LightSources;
	static bool                    Ready;
	static std::vector<Component*> Renderables;
	static GraphicsAPI             SelectedGraphicsAPI;
	static Mesh*                   Skybox;
	static std::vector<Component*> Terrains;
	static std::vector<Component*> Waters;

private:
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
	static void           clear(const glm::vec4 &colorRGBA, FrameBuffer* fbo = nullptr, VkCommandBuffer cmdBuffer = nullptr);
	static void           createDepthFBO();
	static void           createWaterFBOs();
	static int            drawBoundingVolumes();
	static int            drawSelected();
	static int            drawHUDs();
	static int            drawLightSources();
	static int            drawRenderables(DrawProperties &properties = DrawProperties());
	static int            drawSkybox(DrawProperties      &properties = DrawProperties());
	static int            drawTerrains(DrawProperties    &properties = DrawProperties());
	static int            drawWaters();
	static int            drawMeshDX11(Component* mesh, ShaderProgram* shaderProgram, DrawProperties &properties);
	static int            drawMeshDX12(Component* mesh, ShaderProgram* shaderProgram, DrawProperties &properties);
	static int            drawMeshGL(Component*   mesh, ShaderProgram* shaderProgram, DrawProperties &properties);
	static int            drawMeshVK(Component*   mesh, ShaderProgram* shaderProgram, DrawProperties &properties);
	static void           drawMesh(Component*     mesh, ShaderProgram* shaderProgram, DrawProperties &properties);
	static void           drawMeshes(const std::vector<Component*> meshes, DrawProperties &properties);
	static void           drawScene();
	static int            initResources();
	static void           setDrawSettingsGL(ShaderID shaderID);
	static int            setGraphicsAPI(GraphicsAPI api);
	static int            setGraphicsApiCanvas();
	static int            setGraphicsApiDX(GraphicsAPI api);
	static int            setGraphicsApiGL();
	static int            setGraphicsApiVK();
	static ShaderProgram* setShaderProgram(bool enable, ShaderID program = SHADER_ID_UNKNOWN);

};

#endif
