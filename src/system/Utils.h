#ifndef S3DE_GLOBALS_H
	#include "../globals.h"
#endif

#ifndef S3DE_UTILS_H
#define S3DE_UTILS_H

struct AssImpMesh
{
	aiMesh*        Mesh         = nullptr;
	Material       MeshMaterial = {};
	wxString       Name         = "";
	const aiScene* Scene        = nullptr;
	aiMatrix4x4    Transformation;
};

class Utils
{
private:
	Utils()  {}
	~Utils() {}

public:
	static const wxChar*                ALIGNMENTS[];
	static const wxString               APP_NAME;
	static const uint8_t                APP_VERSION_MAJOR;
	static const uint8_t                APP_VERSION_MINOR;
	static const uint8_t                APP_VERSION_PATCH;
	static const wxString               APP_VERSION;
	static const wxString               ASPECT_RATIOS[];
	static const wxChar*                BOUNDING_VOLUMES[];
	static const wxString               COPYRIGHT;
	static const wxString               TESTED;
	static const wxString               GOOGLE_ADS_URL;
	static const wxString               DRAW_MODES[];
	static const wxChar*                FONTS[];
	static const wxString               FOVS[];
	static const wxString               GRAPHIC_APIS[];
	static const std::vector<Icon>      ICONS_ENVIRONMENT;
	static const std::vector<Icon>      ICONS_GEOMETRY;
	static const std::vector<Icon>      ICONS_LIGHTS;
	static const std::vector<Icon>      ICONS_UI;
	static const wxString               IMAGE_FILE_FORMATS;
	static const wxString               MODEL_FILE_FORMATS;
	static const size_t                 NR_OF_GRAPHICS_APIS;
	static wxString                     PROPERTY_IDS[NR_OF_PROPERTY_IDS];
	static std::map<IconType, wxString> RESOURCE_MODELS;
	static std::map<wxString, wxString> RESOURCE_IMAGES;
	static const wxString               SCENE_FILE_FORMAT;
	static const std::vector<Resource>  SHADER_RESOURCES_DX;
	static const std::vector<Resource>  SHADER_RESOURCES_GL_VK;
	static const wxSize                 UI_ADS_SIZE;
	static const wxSize                 UI_LIST_BOX_SIZE;
	static const wxSize                 UI_RENDER_SIZE;
	static const wxSize                 UI_WINDOW_SIZE;
	static const wxSize                 UI_PROPS_SIZE;
	static const wxSize                 UI_TABS_SIZE;

#if defined _WINDOWS
	static const wxString REGKEY_MSIE_EMULATION;
#endif

public:
	static std::vector<uint8_t>     Compress(const std::vector<uint8_t>   &data);
	static std::vector<uint8_t>     Decompress(const std::vector<uint8_t> &data);
	static wxString                 GetGraphicsAPI(GraphicsAPI api);
	static GLenum                   GetImageFormat(const wxImage &image, bool srgb, bool in);
	static VkFormat                 GetImageFormatVK(const wxImage &image, bool srgb);
	static GLsizei                  GetStride(GLsizei size, GLenum arrayType);
	static wxString                 GetSubString(const wxString& string, size_t maxLength, const wxString& endChars);
	static std::vector<uint8_t>     LoadDataFile(const  wxString &file);
	static wxImage*                 LoadImageFile(const wxString &file, wxBitmapType type = wxBITMAP_TYPE_ANY);
	static std::vector<AssImpMesh*> LoadModelFile(const wxString &file);
	static std::vector<Component*>  LoadModelFile(const wxString &file, Component* parent);
	static wxString                 LoadTextFile(const  wxString &file);
	static wxString                 OpenFileDialog(const wxString &fileFormats, bool save);
	static wxString                 OpenFile(const wxString &fileFormats);
	static wxString                 SaveFile(const wxString &fileFormats);
	static int                      SaveDataToFile(const std::vector<uint8_t> &data, const wxString &file, uint64_t size = 0);
	static int                      SaveTextToFile(const wxString &text, const wxString &file);
	static uint8_t                  ToByte(uint64_t number, int byteIndex);
	static GLenum                   ToGlTextureType(TextureType textureType);
	static float                    ToRadians(float degrees);
	static float                    ToDegrees(float radians);
	static float                    ToFloat(bool boolean);
	static wxFloatProperty*         ToFloatProperty(float value, const wxString &label, const wxString &id, float min = -1.0f, float max = -1.0f);
	static json11::Json::array      ToJsonArray(const glm::vec2 &arr);
	static json11::Json::array      ToJsonArray(const glm::vec3 &arr);
	static json11::Json::array      ToJsonArray(const glm::vec4 &arr);
	static uint8_t*                 ToRGBA(const wxImage &image);
	static std::string              ToString(const std::vector<uint8_t> &data);
	static glm::vec2                ToVec2(const json11::Json::array &jsonArray);
	static glm::vec3                ToVec3(const json11::Json::array &jsonArray);
	static glm::vec4                ToVec4(const json11::Json::array &jsonArray);
	static glm::vec3                ToVec3Color(const wxColour &color);
	static glm::vec4                ToVec4Color(const wxColour &color);
	static glm::vec4                ToVec4Float(bool boolean);
	static glm::vec4                ToVec4Float(int integer);
	static glm::vec4                ToVec4Float(bool boolean, int integer);
	static std::vector<float>       ToVertexBufferData(const std::vector<float> &vertices, const std::vector<float> &normals, const std::vector<float> &texCoords);
	static VkImageViewType          ToVkImageViewType(TextureType textureType);
	static wxColour                 ToWxColour(const wxVariant &color);
	static wxColour                 ToWxColour(const glm::vec3 &color);
	static wxColour                 ToWxColour(const glm::vec4 &color);

	#if defined _WINDOWS
		static DXGI_FORMAT       GetImageFormatDXGI(const wxImage &image, bool srgb);
		static DirectX::XMFLOAT2 ToXMFLOAT2(const glm::vec2 &vector);
		static DirectX::XMFLOAT3 ToXMFLOAT3(const glm::vec3 &vector);
		static DirectX::XMFLOAT4 ToXMFLOAT4(bool boolean);
		static DirectX::XMFLOAT4 ToXMFLOAT4(int integer);
		static DirectX::XMFLOAT4 ToXMFLOAT4(bool boolean, int integer);
		static DirectX::XMFLOAT4 ToXMFLOAT4(const glm::vec3 &vector, float w);
		static DirectX::XMFLOAT4 ToXMFLOAT4(const glm::vec4 &vector);
		static DirectX::XMMATRIX ToXMMATRIX(const glm::mat4 &matrix);
	#endif

};

#endif
