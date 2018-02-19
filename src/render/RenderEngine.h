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
	// https://developer.mozilla.org/en-US/docs/Web/API/WebGL_API/Constants
	// POINTS         0x0000
	// LINES          0x0001
	// LINE_LOOP	  0x0002
	// LINE_STRIP	  0x0003
	// TRIANGLES	  0x0004	
	// TRIANGLE_STRIP 0x0005
	// TRIANGLE_FAN   0x0006
	static uint16_t           DrawMode;
	static Camera*            Camera;
	static GLCanvas           Canvas;
	static GPUDescription     GPU;
	static bool               DrawBoundingVolume;
	static std::vector<Mesh*> HUDs;
	static std::vector<Mesh*> Renderables;
	static Mesh*              Skybox;
	static std::vector<Mesh*> Terrains;
	static std::vector<Mesh*> Waters;

public:
	static void Draw();
	static int  Init(WindowFrame* window, const wxSize &size);
	static void RemoveMesh(Mesh* mesh);
	static void SetAspectRatio(const wxString &ratio);
	static void SetCanvasSize(int width, int height);
	static void SetDrawMode(const wxString &mode);
	static int  SetGraphicsAPI(const wxString &api);
	static int  SetGraphicsAPI(GraphicsAPI api);
	static void SetVSYNC(bool enable);

private:
	static void           clear(float r, float g, float b, float a, FrameBuffer* fbo = nullptr);
	static int            drawBoundingVolumes();
	static int            drawSelected();
	static int            drawHUDs(bool        enableClipping = false, const glm::vec3 &clipMax = glm::vec3(0.0f, 0.0f, 0.0f), const glm::vec3 &clipMin = glm::vec3(0.0f, 0.0f, 0.0f));
	static int            drawRenderables(bool enableClipping = false, const glm::vec3 &clipMax = glm::vec3(0.0f, 0.0f, 0.0f), const glm::vec3 &clipMin = glm::vec3(0.0f, 0.0f, 0.0f));
	static int            drawSkybox(bool      enableClipping = false, const glm::vec3 &clipMax = glm::vec3(0.0f, 0.0f, 0.0f), const glm::vec3 &clipMin = glm::vec3(0.0f, 0.0f, 0.0f));
	static int            drawTerrains(bool    enableClipping = false, const glm::vec3 &clipMax = glm::vec3(0.0f, 0.0f, 0.0f), const glm::vec3 &clipMin = glm::vec3(0.0f, 0.0f, 0.0f));
	static int            drawWaters(bool      enableClipping = false, const glm::vec3 &clipMax = glm::vec3(0.0f, 0.0f, 0.0f), const glm::vec3 &clipMin = glm::vec3(0.0f, 0.0f, 0.0f));
	static void           drawScene(bool       enableClipping = false, const glm::vec3 &clipMax = glm::vec3(0.0f, 0.0f, 0.0f), const glm::vec3 &clipMin = glm::vec3(0.0f, 0.0f, 0.0f));
	static void           drawMesh(Mesh*     mesh, ShaderProgram* shaderProgram, bool enableClipping = false, const glm::vec3 &clipMax = glm::vec3(0.0f, 0.0f, 0.0f), const glm::vec3 &clipMin = glm::vec3(0.0f, 0.0f, 0.0f));
	static int            drawMeshDX11(Mesh* mesh, ShaderProgram* shaderProgram, bool enableClipping = false, const glm::vec3 &clipMax = glm::vec3(0.0f, 0.0f, 0.0f), const glm::vec3 &clipMin = glm::vec3(0.0f, 0.0f, 0.0f));
	static int            drawMeshDX12(Mesh* mesh, ShaderProgram* shaderProgram, bool enableClipping = false, const glm::vec3 &clipMax = glm::vec3(0.0f, 0.0f, 0.0f), const glm::vec3 &clipMin = glm::vec3(0.0f, 0.0f, 0.0f));
	static int            drawMeshGL(Mesh*   mesh, ShaderProgram* shaderProgram, bool enableClipping = false, const glm::vec3 &clipMax = glm::vec3(0.0f, 0.0f, 0.0f), const glm::vec3 &clipMin = glm::vec3(0.0f, 0.0f, 0.0f));
	static int            initResources();
	static ShaderProgram* setShaderProgram(bool enable, ShaderID program = SHADER_ID_UNKNOWN);

};

#endif
