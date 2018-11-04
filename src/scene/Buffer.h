#ifndef GE3D_GLOBALS_H
#include "../globals.h"
#endif

#ifndef GE3D_BUFFER_H
#define GE3D_BUFFER_H

class Buffer
{
public:
	Buffer(std::vector<uint32_t> &indices);
	Buffer(std::vector<float> &data);
	Buffer(std::vector<float> &vertices, std::vector<float> &normals, std::vector<float> &texCoords);
	Buffer();
	~Buffer();

public:
	UINT            BufferStride;
	VkBuffer        IndexBuffer;
	VkDeviceMemory  IndexBufferMemory;
	VKPipeline      Pipeline;
	VKUniform       Uniform;
	VkBuffer        VertexBuffer;
	VkDeviceMemory  VertexBufferMemory;

	#if defined _WINDOWS
		ID3D11BlendState*        BlendStatesDX11[NR_OF_SHADERS];
		ID3D11DepthStencilState* DepthStencilStatesDX11[NR_OF_SHADERS];
		ID3D11InputLayout*       InputLayoutsDX11[NR_OF_SHADERS];
		ID3D11RasterizerState*   RasterizerStatesDX11[NR_OF_SHADERS];
		ID3D12PipelineState*     PipelineStatesDX12[NR_OF_SHADERS];
		ID3D12PipelineState*     PipelineStatesFBODX12[NR_OF_SHADERS];
		ID3D12RootSignature*     RootSignaturesDX12[NR_OF_SHADERS];

		ID3D11Buffer*         ConstantBuffersDX11[NR_OF_SHADERS];
		ID3D12Resource*       ConstantBuffersDX12[NR_OF_SHADERS];
		ID3D12DescriptorHeap* ConstantBufferHeapsDX12[NR_OF_SHADERS];
		ID3D12DescriptorHeap* SamplerHeapsDX12[NR_OF_SHADERS];

		DXMatrixBuffer    MatrixBufferValues;
		DXLightBuffer     LightBufferValues;
		DXDefaultBuffer   DefaultBufferValues;
		DXHUDBuffer       HUDBufferValues;
		DXSkyboxBuffer    SkyboxBufferValues;
		DXWireframeBuffer WireframeBufferValues;
		DXTerrainBuffer   TerrainBufferValues;
		DXWaterBuffer     WaterBufferValues;

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
