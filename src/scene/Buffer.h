#ifndef S3DE_GLOBALS_H
#include "../globals.h"
#endif

#ifndef S3DE_BUFFER_H
#define S3DE_BUFFER_H

/**
* Constant Buffers
*/

struct CBLight
{
	CBLight(LightSource* lightSource);
	CBLight() {}

	glm::vec4 Active         = {};
	glm::vec4 Ambient        = {};
	glm::vec4 Angles         = {};
	glm::vec4 Attenuation    = {};
	glm::vec4 Diffuse        = {};
	glm::vec4 Direction      = {};
	glm::vec4 Position       = {};
	glm::vec4 Specular       = {};
	glm::mat4 ViewProjection = {};

	//sampler2D   DepthMapTexture2D;
	//samplerCube DepthMapTextureCube;
};

struct CBMatrix
{
	CBMatrix(Component* mesh, bool removeTranslation);
	CBMatrix(LightSource* lightSource, Component* mesh);
	CBMatrix() {}

	glm::mat4 Normal     = {};
	glm::mat4 Model      = {};
	glm::mat4 View       = {};
	glm::mat4 Projection = {};
	glm::mat4 MVP        = {};
};

struct CBColor
{
	CBColor(const glm::vec4 &color);
	CBColor() {}

	glm::vec4 Color = {};
};

struct CBDefault
{
	CBDefault(Component* mesh, const DrawProperties &properties);
	CBDefault() {}

	CBLight LightSources[MAX_LIGHT_SOURCES];

	glm::vec4 IsTextured[MAX_TEXTURES];
	glm::vec4 TextureScales[MAX_TEXTURES];

	glm::vec4 CameraPosition = {};

	glm::vec4 MeshSpecular = {};
	glm::vec4 MeshDiffuse  = {};

	glm::vec4 ClipMax        = {};
	glm::vec4 ClipMin        = {};
	glm::vec4 EnableClipping = {};

	glm::vec4 EnableSRGB = {};
};

struct CBHUD
{
	CBHUD(const glm::vec4 &color, bool transparent);
	CBHUD() {}

	glm::vec4 MaterialColor = {};
	glm::vec4 IsTransparent = {};
};

struct CBWater
{
	CBWater(const CBDefault &default, float moveFactor, float waveStrength);
	CBWater() {}

	float     MoveFactor   = 0;
	float     WaveStrength = 0;
	glm::vec2 Padding1     = {};

	CBDefault DB = {};
};

#if defined _WINDOWS

struct CBLightDX
{
	CBLightDX(LightSource* lightSource);
	CBLightDX(const CBLight &light);
	CBLightDX() {}

	DirectX::XMFLOAT4 Active         = {};
	DirectX::XMFLOAT4 Ambient        = {};
	DirectX::XMFLOAT4 Angles         = {};
	DirectX::XMFLOAT4 Attenuation    = {};
	DirectX::XMFLOAT4 Diffuse        = {};
	DirectX::XMFLOAT4 Direction      = {};
	DirectX::XMFLOAT4 Position       = {};
	DirectX::XMFLOAT4 Specular       = {};
	DirectX::XMMATRIX ViewProjection = {};
};

struct CBMatrixDX
{
	CBMatrixDX(const CBMatrix &matrices);
	CBMatrixDX() {}

	DirectX::XMMATRIX Normal     = {};
	DirectX::XMMATRIX Model      = {};
	DirectX::XMMATRIX View       = {};
	DirectX::XMMATRIX Projection = {};
	DirectX::XMMATRIX MVP        = {};
};

struct CBColorDX
{
	CBColorDX(const CBMatrix &matrices, const glm::vec4 &color);
	CBColorDX() {}

	CBMatrixDX        MB    = {};
	DirectX::XMFLOAT4 Color = {};
};

struct CBDefaultDX
{
	CBDefaultDX(const CBMatrix &matrices, Component* mesh, const glm::vec3 &clipMax = {}, const glm::vec3 &clipMin = {}, bool enableClipping = false);
	CBDefaultDX(const CBDefault &default, const CBMatrix &matrices);
	CBDefaultDX() {}

	CBMatrixDX MB = {};

	CBLightDX LightSources[MAX_LIGHT_SOURCES];

	DirectX::XMFLOAT4 IsTextured[MAX_TEXTURES];
	DirectX::XMFLOAT4 TextureScales[MAX_TEXTURES];

	DirectX::XMFLOAT4 CameraPosition = {};

	DirectX::XMFLOAT4 MeshSpecular = {};
	DirectX::XMFLOAT4 MeshDiffuse  = {};

	DirectX::XMFLOAT4 ClipMax        = {};
	DirectX::XMFLOAT4 ClipMin        = {};
	DirectX::XMFLOAT4 EnableClipping = {};

	DirectX::XMFLOAT4 EnableSRGB = {};
};

struct CBHUDDX
{
	CBHUDDX(const CBMatrix &matrices, const glm::vec4 &color, bool transparent);
	CBHUDDX() {}

	CBMatrixDX MB = {};

	DirectX::XMFLOAT4 MaterialColor = {};
	DirectX::XMFLOAT4 IsTransparent = {};
};

struct CBSkyboxDX
{
	CBSkyboxDX(const CBMatrix &matrices);
	CBSkyboxDX() {}

	CBMatrixDX MB = {};
};

struct CBWaterDX
{
	CBWaterDX(const CBMatrix &matrices, const CBDefault &default, float moveFactor, float waveStrength);
	CBWaterDX() {}

	float             MoveFactor    = 0;
	float             WaveStrength  = 0;
	DirectX::XMFLOAT2 Padding1      = {};

	CBDefaultDX DB = {};
};

#endif

class Buffer
{
public:
	Buffer(std::vector<uint32_t> &indices);
	Buffer(std::vector<float> &data);
	Buffer(std::vector<float> &vertices, std::vector<float> &normals, std::vector<float> &texCoords);
	Buffer();
	~Buffer();

public:
	UINT           BufferStride;
	VkBuffer       IndexBuffer;
	VkDeviceMemory IndexBufferMemory;
	VKPipeline     Pipeline;
	VKUniform      Uniform;
	VkBuffer       VertexBuffer;
	VkDeviceMemory VertexBufferMemory;

	#if defined _WINDOWS
		ID3D11BlendState*        BlendStatesDX11[NR_OF_SHADERS];
		ID3D11DepthStencilState* DepthStencilStatesDX11[NR_OF_SHADERS];
		ID3D11InputLayout*       InputLayoutsDX11[NR_OF_SHADERS];
		ID3D11RasterizerState*   RasterizerStatesDX11[NR_OF_SHADERS];
		ID3D12PipelineState*     PipelineStatesDX12[NR_OF_SHADERS];
		ID3D12PipelineState*     PipelineStatesColorFBODX12[NR_OF_SHADERS];
		ID3D12PipelineState*     PipelineStatesDepthFBODX12[NR_OF_SHADERS];
		ID3D12RootSignature*     RootSignaturesDX12[NR_OF_SHADERS];

		ID3D11Buffer*         ConstantBuffersDX11[NR_OF_SHADERS];
		ID3D12Resource*       ConstantBuffersDX12[NR_OF_SHADERS];
		ID3D12DescriptorHeap* ConstantBufferHeapsDX12[NR_OF_SHADERS];
		ID3D12DescriptorHeap* SamplerHeapsDX12[NR_OF_SHADERS];

		CBColorDX   ConstantBufferColor;
		CBDefaultDX ConstantBufferDefault;
		CBHUDDX     ConstantBufferHUD;
		CBSkyboxDX  ConstantBufferSkybox;
		CBWaterDX   ConstantBufferWater;

		ID3D11Buffer*            VertexBufferDX11;
		ID3D12Resource*          VertexBufferDX12;
		D3D12_INDEX_BUFFER_VIEW  IndexBufferViewDX12;
		D3D12_VERTEX_BUFFER_VIEW VertexBufferViewDX12;
#endif

private:
	GLuint             id;
	std::vector<float> normals;
	std::vector<float> texCoords;
	std::vector<float> vertices;

public:
	GLuint ID();
	size_t Normals();
	void   ResetPipelines();
	size_t TexCoords();
	size_t Vertices();

private:
	void init();

};

#endif
