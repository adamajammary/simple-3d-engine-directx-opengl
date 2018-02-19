#ifdef _WINDOWS

#ifndef GE3D_GLOBALS_H
	#include "../globals.h"
#endif

#ifndef GE3D_DXCONTEXT_H
#define GE3D_DXCONTEXT_H

class DXContext
{
public:
	DXContext(GraphicsAPI api, bool vsync = true);
	DXContext();
	~DXContext();

private:
	ID3D11RenderTargetView*    colorBuffer;
	ID3D12Resource*            colorBuffers[2];
	ID3D12DescriptorHeap*      colorBufferHeap;
	UINT                       colorBufferIndex;
	UINT                       colorBufferSize;
	ID3D12CommandAllocator*    commandAllocator;
	ID3D12GraphicsCommandList* commandList;
	ID3D12CommandQueue*        commandQueue;
	ID3D11DepthStencilView*    depthStencilBuffer11;
	ID3D12Resource*            depthStencilBuffer12;
	ID3D12DescriptorHeap*      depthStencilBufferHeap;
	ID3D11DeviceContext*       deviceContext;
	ID3D12Fence*               fence;
	HANDLE                     fenceEvent;
	UINT64                     fenceValue;
	bool                       isOK;
	const UINT                 NR_OF_FRAMEBUFFERS = 2;
	ID3D12PipelineState*       pipelineState;
	ID3D11Device*              renderDevice11;
	ID3D12Device*              renderDevice12;
	D3D12_RECT                 scissorRect;
	IDXGISwapChain*            swapChain11;
	IDXGISwapChain3*           swapChain12;
	D3D11_VIEWPORT             viewPort11;
	D3D12_VIEWPORT             viewPort12;
	bool                       vSync;

public:
	void Bind11(ID3D11RenderTargetView* colorBuffer, ID3D11DepthStencilView* depthStencilBuffer, D3D11_VIEWPORT* viewPort);
	void Bind12(ID3D12Resource* colorBufferResource, CD3DX12_CPU_DESCRIPTOR_HANDLE* colorBufferHandle, CD3DX12_CPU_DESCRIPTOR_HANDLE* depthStencilHandle, D3D12_VIEWPORT* viewPort, D3D12_RECT* scissorRect);
	void Unbind11();
	void Unbind12(ID3D12Resource* colorBufferResource);
	void Clear11(float r, float g, float b, float a);
	void Clear12(float r, float g, float b, float a);
	void Clear11(float r, float g, float b, float a, FrameBuffer* fbo);
	void Clear12(float r, float g, float b, float a, FrameBuffer* fbo);
	int  CreateConstantBuffers11(Buffer* buffer);
	int  CreateConstantBuffers12(Buffer* buffer);
	int  CreateIndexBuffer11(std::vector<unsigned int> &indices, ID3D11Buffer**   indexBuffer);
	int  CreateIndexBuffer12(std::vector<unsigned int> &indices, ID3D12Resource** indexBuffer, D3D12_INDEX_BUFFER_VIEW &bufferView);
	int  CreateShader11(const wxString &file, ID3DBlob** vs, ID3DBlob** fs, ID3D11VertexShader** shaderVS, ID3D11PixelShader** shaderFS);
	int  CreateShader12(const wxString &file, ID3DBlob** vs, ID3DBlob** fs);
	int  CreateTexture11(const std::vector<uint8_t*> &pixels, int width, int height, DXGI_FORMAT format, ID3D11Texture2D** texture, ID3D11ShaderResourceView** srv, D3D11_SAMPLER_DESC &samplerDesc, ID3D11SamplerState** sampler);
	int  CreateTexture12(const std::vector<uint8_t*> &pixels, int width, int height, DXGI_FORMAT format, ID3D12Resource**  texture, D3D12_SHADER_RESOURCE_VIEW_DESC &srvDesc, D3D12_SAMPLER_DESC &samplerDesc);
	int  CreateTextureBuffer11(int width, int height, DXGI_FORMAT format, ID3D11RenderTargetView** colorBuffer, ID3D11Texture2D** texture, ID3D11ShaderResourceView** srv, D3D11_SAMPLER_DESC &samplerDesc, ID3D11SamplerState** sampler);
	int  CreateTextureBuffer12(int width, int height, DXGI_FORMAT format, ID3D12DescriptorHeap** colorBuffer, ID3D12Resource** texture, D3D12_SHADER_RESOURCE_VIEW_DESC &srvDesc, D3D12_SAMPLER_DESC &samplerDesc);
	int  CreateVertexBuffer11(std::vector<float> &vertices, std::vector<float> &normals, std::vector<float> &texCoords, ID3D11Buffer**   vertexBuffer, UINT &bufferStride, ID3D11InputLayout** inputLayouts, ID3D11RasterizerState** rasterizerStates, ID3D11DepthStencilState** depthStencilStates, ID3D11BlendState** blendStates);
	int  CreateVertexBuffer12(std::vector<float> &vertices, std::vector<float> &normals, std::vector<float> &texCoords, ID3D12Resource** vertexBuffer, UINT &bufferStride, D3D12_VERTEX_BUFFER_VIEW &bufferView, ID3D12PipelineState** pipelineStates, ID3D12RootSignature** rootSignatures);
	int  Draw11(Mesh* mesh, ShaderProgram* shaderProgram, bool enableClipping = false, const glm::vec3 &clipMax = glm::vec3(0.0f, 0.0f, 0.0f), const glm::vec3 &clipMin = glm::vec3(0.0f, 0.0f, 0.0f));
	int  Draw12(Mesh* mesh, ShaderProgram* shaderProgram, bool enableClipping = false, const glm::vec3 &clipMax = glm::vec3(0.0f, 0.0f, 0.0f), const glm::vec3 &clipMin = glm::vec3(0.0f, 0.0f, 0.0f));
	bool IsOK();
	void Present11();
	void Present12();
	void SetVSYNC(bool enable);

private:
	int                commandsExecute();
	int                commandsInit();
	void               commandsColorBufferPrepare(ID3D12Resource* colorBuffer);
	void               commandsColorBufferPresent(ID3D12Resource* colorBuffer);
	int                compileShader(const wxString &file, ID3DBlob** vs, ID3DBlob** fs);
	int                createRootSignature(ShaderProgram* shader, ID3D12RootSignature** rootSignature);
	IDXGIAdapter*      getAdapter11(IDXGIFactory*  factory);
	IDXGIAdapter1*     getAdapter12(IDXGIFactory4* factory);
	std::vector<float> getVertexBufferData(std::vector<float> &vertices, std::vector<float> &normals, std::vector<float> &texCoords);
	bool               init11(bool vsync = true);
	bool               init12(bool vsync = true);
	void               release();
	void               wait();

};

#endif
#endif