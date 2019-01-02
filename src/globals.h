#ifndef S3DE_GLOBALS_H
#define S3DE_GLOBALS_H

#define _CRT_SECURE_NO_WARNINGS
#define UNICODE
#define _UNICODE

// C++
#include <fstream>
#include <map>
#include <set>

// DirectX
#if defined _WINDOWS
	#include <d3d11.h>
	#include <d3d12.h>
	#include <d3dx12.h>
	#include <d3dcompiler.h>
	#include <dxgi1_5.h>
	#include <DirectXMath.h>
#endif

// OpenGL
//#define GLEW_STATIC

#include <GL/glew.h>
#if defined _WINDOWS
	#include <wglext.h>
#else
	#include <glxext.h>
#endif

// Vulkan
#if defined _WINDOWS
	#define VK_USE_PLATFORM_WIN32_KHR
#endif
#include <vulkan/vulkan.h>

// GLM - OpenGL Mathematics
#define GLM_FORCE_RADIANS
//#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/transform.hpp>

// AssImp
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

// JSON
#include <json11-1.0.0/json11.hpp>

// LZMA SDK
#include <LzmaLib.h>

// LibNoise
#include <noise.h>

// WxWidgets
#define WXUSINGDLL

#include <wx/app.h>
#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/dataview.h>
#include <wx/dcmemory.h>
#include <wx/dcgraph.h>
#include <wx/event.h>
#include <wx/filedlg.h>
#include <wx/frame.h>
#include <wx/gbsizer.h>
#include <wx/glcanvas.h>
#include <wx/image.h>
#include <wx/listbox.h>
#include <wx/menu.h>
#include <wx/msgdlg.h>
#include <wx/notebook.h>
#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>
#include <wx/sizer.h>
#include <wx/statline.h>
#include <wx/stattext.h>
#include <wx/stopwatch.h>
#include <wx/webview.h>

struct CBMatrix;

class BoundingVolume;
class Buffer;
class Component;
class LightSource;
class DXContext;
class Mesh;
class ShaderProgram;
class Skybox;
class Terrain;
class Texture;
class VKContext;
class Water;
class WindowFrame;

static const uint32_t BUFFER_SIZE           = 1024;
static const uint32_t LZMA_OFFSET_ID        = 8;
static const uint32_t LZMA_OFFSET_SIZE      = 8;
static const uint32_t MAX_CONCURRENT_FRAMES = 2;
static const uint32_t MAX_LIGHT_SOURCES     = 13;
static const uint32_t MAX_TEXTURES          = 6;
static const uint32_t MAX_TEXTURE_SLOTS     = (MAX_TEXTURES + MAX_LIGHT_SOURCES + MAX_LIGHT_SOURCES);
static const uint32_t NR_OF_FRAMEBUFFERS    = 2;

#if defined _WINDOWS

static const unsigned int BYTE_ALIGN_BUFFER_DATA = 65536;
static const unsigned int BYTE_ALIGN_BUFFER_VIEW = 256;

#endif

enum Attrib
{
	ATTRIB_NORMAL, ATTRIB_POSITION, ATTRIB_TEXCOORDS, NR_OF_ATTRIBS
};

enum BoundingVolumeType
{
	BOUNDING_VOLUME_NONE, BOUNDING_VOLUME_BOX, BOUNDING_VOLUME_SPHERE, NR_OF_BOUNDING_VOLUMES
};

enum ComponentType
{
	COMPONENT_UNKNOWN = -1,
	COMPONENT_CAMERA,
	COMPONENT_HUD,
	COMPONENT_MESH,
	COMPONENT_MODEL,
	COMPONENT_SKYBOX,
	COMPONENT_TERRAIN,
	COMPONENT_WATER,
	COMPONENT_LIGHTSOURCE
};

enum DrawModeType
{
	DRAW_MODE_UNKNOWN = -1, DRAW_MODE_FILLED, DRAW_MODE_WIREFRAME, NR_OF_DRAW_MODES
};

enum FBOType
{
	FBO_UNKNOWN = -1, FBO_COLOR, FBO_DEPTH, NR_OF_FBO_TYPES
};

enum GraphicsAPI
{
	GRAPHICS_API_UNKNOWN = -1,
	GRAPHICS_API_OPENGL,
	GRAPHICS_API_VULKAN,
	GRAPHICS_API_DIRECTX11,
	GRAPHICS_API_DIRECTX12,
	NR_OF_GRAPHICS_ENGINES
};

enum IconType
{
	ID_ICON_UNKNOWN,
	ID_ICON_BROWSE,
	ID_ICON_PLANE,
	ID_ICON_CUBE,
	ID_ICON_UV_SPHERE,
	ID_ICON_ICO_SPHERE,
	ID_ICON_CYLINDER,
	ID_ICON_CONE,
	ID_ICON_TORUS,
	ID_ICON_MONKEY_HEAD,
	ID_ICON_SKYBOX,
	ID_ICON_TERRAIN,
	ID_ICON_WATER,
	ID_ICON_HUD,
	ID_ICON_QUAD,
	ID_ICON_LIGHT_DIRECTIONAL,
	ID_ICON_LIGHT_POINT,
	ID_ICON_LIGHT_SPOT,
	ID_CANVAS,
	ID_ASPECT_RATIO,
	ID_FOV,
	ID_DRAW_MODE,
	ID_DRAW_BOUNDING,
	ID_GRAPHICS_API,
	ID_VSYNC,
	ID_SRGB,
	ID_COMPONENTS,
	ID_CHILDREN,
	ID_SCENE_DETAILS,
	ID_SCENE_CLEAR,
	ID_SCENE_LOAD,
	ID_SCENE_SAVE,
	ID_REMOVE_COMPONENT,
	ID_REMOVE_CHILD,
	ID_TABS,
	ID_TABS_GEOMETRY,
	ID_TABS_LIGHTS
};

enum PropertyID
{
	PROPERTY_ID_NAME,
	PROPERTY_ID_LOCATION,
	PROPERTY_ID_LOCATION_,
	PROPERTY_ID_ROTATION,
	PROPERTY_ID_ROTATION_,
	PROPERTY_ID_SCALE,
	PROPERTY_ID_SCALE_,
	PROPERTY_ID_AUTO_ROTATION,
	PROPERTY_ID_AUTO_ROTATION_,
	PROPERTY_ID_ENABLE_AUTO_ROTATION,
	PROPERTY_ID_COLOR,
	PROPERTY_ID_SPEC_INTENSITY,
	PROPERTY_ID_SPEC_SHININESS,
	PROPERTY_ID_OPACITY,
	PROPERTY_ID_TRANSPARENCY,
	PROPERTY_ID_TEXT,
	PROPERTY_ID_TEXT_ALIGNMENT,
	PROPERTY_ID_TEXT_FONT,
	PROPERTY_ID_TEXT_SIZE,
	PROPERTY_ID_TEXT_COLOR,
	PROPERTY_ID_HUD_TEXTURE,
	PROPERTY_ID_HUD_REMOVE_TEXTURE,
	PROPERTY_ID_TEXTURE_,
	PROPERTY_ID_REMOVE_TEXTURE_,
	PROPERTY_ID_FLIP_TEXTURE_,
	PROPERTY_ID_REPEAT_TEXTURE_,
	PROPERTY_ID_TILING_U_,
	PROPERTY_ID_TILING_V_,
	PROPERTY_ID_BOUNDING_VOLUME,
	PROPERTY_ID_TERRAIN_SIZE,
	PROPERTY_ID_TERRAIN_OCTAVES,
	PROPERTY_ID_TERRAIN_REDISTRIBUTION,
	PROPERTY_ID_WATER_SPEED,
	PROPERTY_ID_WATER_WAVE_STRENGTH,
	PROPERTY_ID_LIGHT_ACTIVE,
	PROPERTY_ID_LIGHT_LOCATION,
	PROPERTY_ID_LIGHT_LOCATION_,
	PROPERTY_ID_LIGHT_AMBIENT,
	PROPERTY_ID_LIGHT_DIFFUSE,
	PROPERTY_ID_LIGHT_SPEC_INTENSITY,
	PROPERTY_ID_LIGHT_SPEC_SHININESS,
	PROPERTY_ID_LIGHT_DIRECTION,
	PROPERTY_ID_LIGHT_DIRECTION_,
	PROPERTY_ID_LIGHT_ATT_LINEAR,
	PROPERTY_ID_LIGHT_ATT_QUAD,
	PROPERTY_ID_LIGHT_ANGLE_INNER,
	PROPERTY_ID_LIGHT_ANGLE_OUTER,
	NR_OF_PROPERTY_IDS
};

enum RenderPass
{
	RENDER_PASS_DEFAULT, RENDER_PASS_FBO_COLOR, RENDER_PASS_FBO_DEPTH, NR_OF_RENDER_PASSES
};

enum ShaderID
{
	SHADER_ID_UNKNOWN = -1,
	SHADER_ID_COLOR,
	SHADER_ID_DEFAULT,
	SHADER_ID_DEPTH,
	SHADER_ID_HUD,
	SHADER_ID_SKYBOX,
	SHADER_ID_TERRAIN,
	SHADER_ID_WATER,
	SHADER_ID_WIREFRAME,
	NR_OF_SHADERS
};

enum TextureType
{
	TEXTURE_UNKNOWN = -1, TEXTURE_2D, TEXTURE_CUBEMAP, NR_OF_TEXTURE_TYPES
};

enum UniformBufferTypeGL
{
	UBO_GL_MATRIX,
	UBO_GL_COLOR,
	UBO_GL_DEFAULT,
	UBO_GL_HUD,
	UBO_GL_TERRAIN,
	UBO_GL_WATER,
	UBO_GL_TEXTURES0,  UBO_GL_TEXTURES1,  UBO_GL_TEXTURES2,  UBO_GL_TEXTURES3,  UBO_GL_TEXTURES4,  UBO_GL_TEXTURES5,
	UBO_GL_TEXTURES6,  UBO_GL_TEXTURES7,  UBO_GL_TEXTURES8,  UBO_GL_TEXTURES9,  UBO_GL_TEXTURES10, UBO_GL_TEXTURES11,
	UBO_GL_TEXTURES12, UBO_GL_TEXTURES13, UBO_GL_TEXTURES14, UBO_GL_TEXTURES15, UBO_GL_TEXTURES16, UBO_GL_TEXTURES17,
	UBO_GL_TEXTURES18, UBO_GL_TEXTURES19, UBO_GL_TEXTURES20, UBO_GL_TEXTURES21, UBO_GL_TEXTURES22, UBO_GL_TEXTURES23,
	UBO_GL_TEXTURES24, UBO_GL_TEXTURES25, UBO_GL_TEXTURES26, UBO_GL_TEXTURES27, UBO_GL_TEXTURES28, UBO_GL_TEXTURES29,
	UBO_GL_TEXTURES30, UBO_GL_TEXTURES31,
	NR_OF_UBOS_GL
};

enum UniformBinding
{
	UBO_BINDING_MATRIX, UBO_BINDING_DEFAULT, UBO_BINDING_TEXTURES, UBO_BINDING_DEPTH_2D, UBO_BINDING_DEPTH_CUBEMAPS, NR_OF_UBO_BINDINGS
};

enum UniformBufferTypeVK
{
	UBO_VK_MATRIX,
	UBO_VK_COLOR,
	UBO_VK_DEFAULT,
	UBO_VK_HUD,
	UBO_VK_TERRAIN,
	UBO_VK_WATER,
	NR_OF_UBOS_VK
};

struct DrawProperties
{
	glm::vec3       ClipMax            = {};
	glm::vec3       ClipMin            = {};
	bool            DrawBoundingVolume = false;
	bool            DrawSelected       = false;
	bool            EnableClipping     = false;
	FBOType         FboType            = FBO_UNKNOWN;
	LightSource*    Light              = nullptr;
	ShaderID        Shader             = SHADER_ID_UNKNOWN;
	VkCommandBuffer VKCommandBuffer    = nullptr;
};

struct GPUDescription
{
	wxString Renderer = "";
	wxString Vendor   = "";
	wxString Version  = "";
};

struct Icon
{
	wxString File  = "";
	IconType ID    = ID_ICON_UNKNOWN;
	wxString Title = "";
};

struct MouseState
{
	wxPoint Position = wxPoint(0, 0);
};

struct Resource
{
	wxString File   = "";
	wxString Name   = "";
	wxString Result = "";
};

struct Time
{
	long Hours = 0, Minutes = 0, Seconds = 0, MilliSeconds = 0, Total = 0;

	Time(long ms)
	{
		this->Hours        = (ms / 3600000);
		this->Minutes      = ((ms % 3600000) / 60000);
		this->Seconds      = (((ms % 3600000) % 60000) / 1000);
		this->MilliSeconds = (((ms % 3600000) % 60000) % 1000);
		this->Total        = ms;
	}
};

struct VKPipeline
{
	VkPipelineLayout Layout;
	VkPipeline       Pipelines[NR_OF_SHADERS];
	VkPipeline       PipelinesColorFBO[NR_OF_SHADERS];
	//VkPipeline       PipelinesDepthFBO[NR_OF_SHADERS];
};

struct VKUniform
{
	VkBuffer              Buffers[NR_OF_UBOS_VK]        = {};
	VkDeviceMemory        BufferMemories[NR_OF_UBOS_VK] = {};
	VkDescriptorSetLayout Layout = {};
	VkDescriptorPool      Pool   = {};
	VkDescriptorSet       Set    = {};
};

struct GLCanvas
{
	float          AspectRatio = 0.0f;
	wxGLCanvas*    Canvas      = nullptr;
	DXContext*     DX          = nullptr;
	wxGLContext*   GL          = nullptr;
	wxPoint        Position    = wxPoint(0, 0);
	wxSize         Size        = wxSize(0, 0);
	VKContext*     VK          = nullptr;
	WindowFrame*   Window      = nullptr;
};

#ifndef _DELETEP
	#define _DELETEP(x) if (x != nullptr) { delete x; x = nullptr; }
#endif

#ifndef _RELEASEP
	#define _RELEASEP(x) if (x != nullptr) { x->Release(); x = nullptr; }
#endif

#ifndef S3DE_MATERIAL_H
	#include "scene/Material.h"
#endif
#ifndef S3DE_LIGHT_H
	#include "scene/Light.h"
#endif
#ifndef S3DE_UTILS_H
	#include "system/Utils.h"
#endif
#ifndef S3DE_NOISE_H
	#include "system/Noise.h"
#endif
#ifndef S3DE_INPUTMANAGER_H
	#include "input/InputManager.h"
#endif
#ifndef S3DE_RAYCAST_H
	#include "physics/RayCast.h"
#endif
#ifndef S3DE_PHYSICSENGINE_H
	#include "physics/PhysicsEngine.h"
#endif

#ifndef S3DE_FRAMEBUFFER_H
	#include "scene/FrameBuffer.h"
#endif
//#ifndef S3DE_MATERIAL_H
//	#include "scene/Material.h"
//#endif
//#ifndef S3DE_LIGHT_H
//	#include "scene/Light.h"
//#endif
#ifndef S3DE_DXCONTEXT_H
	#include "render/DXContext.h"
#endif
#ifndef S3DE_VKCONTEXT_H
	#include "render/VKContext.h"
#endif
#ifndef S3DE_COMPONENT_H
	#include "scene/Component.h"
#endif
#ifndef S3DE_CAMERA_H
	#include "scene/Camera.h"
#endif
#ifndef S3DE_RENDERENGINE_H
	#include "render/RenderEngine.h"
#endif
#ifndef S3DE_SHADERMANAGER_H
	#include "render/ShaderManager.h"
#endif
#ifndef S3DE_SHADERPROGRAM_H
	#include "render/ShaderProgram.h"
#endif
#ifndef S3DE_BUFFER_H
	#include "scene/Buffer.h"
#endif
//#ifndef S3DE_COMPONENT_H
//	#include "scene/Component.h"
//#endif
#ifndef S3DE_MESH_H
	#include "scene/Mesh.h"
#endif
#ifndef S3DE_BOUNDINGVOLUME_H
	#include "scene/BoundingVolume.h"
#endif
//#ifndef S3DE_CAMERA_H
//	#include "scene/Camera.h"
//#endif
#ifndef S3DE_HUD_H
	#include "scene/HUD.h"
#endif
#ifndef S3DE_MODEL_H
	#include "scene/Model.h"
#endif
#ifndef S3DE_LIGHTSOURCE_H
	#include "scene/LightSource.h"
#endif
#ifndef S3DE_SCENEMANAGER_H
	#include "scene/SceneManager.h"
#endif
#ifndef S3DE_SKYBOX_H
	#include "scene/Skybox.h"
#endif
#ifndef S3DE_TERRAIN_H
	#include "scene/Terrain.h"
#endif
#ifndef S3DE_TEXTURE_H
	#include "scene/Texture.h"
#endif
#ifndef S3DE_WATERFBO_H
	#include "scene/WaterFBO.h"
#endif
#ifndef S3DE_WATER_H
	#include "scene/Water.h"
#endif
#ifndef S3DE_TIMEMANAGER_H
	#include "time/TimeManager.h"
#endif
#ifndef S3DE_WINDOW_H
	#include "ui/Window.h"
#endif
#ifndef S3DE_WINDOWFRAME_H
	#include "ui/WindowFrame.h"
#endif

#endif
