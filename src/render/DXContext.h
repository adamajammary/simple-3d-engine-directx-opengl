#if defined _WINDOWS

#ifndef GE3D_GLOBALS_H
	#include "../globals.h"
#endif

#ifndef GE3D_DXCONTEXT_H
#define GE3D_DXCONTEXT_H

class DXContext
{
public:
	DXContext(GraphicsAPI api, bool vsync = true);
	~DXContext();

private:
	ID3D11RenderTargetView*    colorBuffer;
	ID3D12Resource*            colorBuffers[NR_OF_FRAMEBUFFERS];
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
	UINT                       multiSampleCount;
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
	void Bind11(ID3D11RenderTargetView* colorBuffer, ID3D11DepthStencilView* depthStencilBuffer, D3D11_VIEWPORT &viewPort);
	void Bind12(ID3D12Resource* colorBufferResource, CD3DX12_CPU_DESCRIPTOR_HANDLE* colorBufferHandle, CD3DX12_CPU_DESCRIPTOR_HANDLE* depthStencilHandle, D3D12_VIEWPORT& viewPort, D3D12_RECT &scissorRect);
	void Unbind11();
	void Unbind12(ID3D12Resource* colorBufferResource);
	void Clear11(float r, float g, float b, float a);
	void Clear12(float r, float g, float b, float a);
	void Clear11(float r, float g, float b, float a, FrameBuffer* fbo);
	void Clear12(float r, float g, float b, float a, FrameBuffer* fbo);
	int  CreateConstantBuffers11(Buffer* buffer);
	int  CreateConstantBuffers12(Buffer* buffer);
	int  CreateIndexBuffer11(std::vector<unsigned int> &indices, Buffer* buffer);
	int  CreateIndexBuffer12(std::vector<unsigned int> &indices, Buffer* buffer);
	int  CreateShader11(const wxString &file, ID3DBlob** vs, ID3DBlob** fs, ID3D11VertexShader** shaderVS, ID3D11PixelShader** shaderFS);
	int  CreateShader12(const wxString &file, ID3DBlob** vs, ID3DBlob** fs);
	int  CreateTexture11(const std::vector<BYTE*> &pixels, DXGI_FORMAT format, D3D11_SAMPLER_DESC &samplerDesc, Texture* texture);
	int  CreateTexture12(const std::vector<BYTE*> &pixels, DXGI_FORMAT format, Texture* texture);
	int  CreateTextureBuffer11(DXGI_FORMAT format, D3D11_SAMPLER_DESC &samplerDesc, Texture* texture);
	int  CreateTextureBuffer12(DXGI_FORMAT format, Texture* texture);
	int  CreateVertexBuffer11(const std::vector<float> &vertices, const std::vector<float> &normals, const std::vector<float> &texCoords, Buffer* buffer);
	int  CreateVertexBuffer12(const std::vector<float> &vertices, const std::vector<float> &normals, const std::vector<float> &texCoords, Buffer* buffer);
	int  Draw11(Mesh* mesh, ShaderProgram* shaderProgram, const DrawProperties &properties = {});
	int  Draw12(Mesh* mesh, ShaderProgram* shaderProgram, const DrawProperties &properties = {});
	bool IsOK();
	void Present11();
	void Present12();
	void SetVSync(bool enable);

private:
	int                      commandsExecute();
	int                      commandsInit();
	int                      compileShader(const wxString &file, ID3DBlob** vs, ID3DBlob** fs);
	int                      createPipeline(ShaderProgram* shaderProgram, ID3D12PipelineState** pipeline, ID3D12RootSignature** rootSignature, bool fbo, const std::vector<D3D12_INPUT_ELEMENT_DESC> &attribsDescs);
	int                      createRootSignature(ShaderProgram* shader, ID3D12RootSignature** rootSignature);
	IDXGIAdapter*            getAdapter11(IDXGIFactory*  factory);
	IDXGIAdapter1*           getAdapter12(IDXGIFactory4* factory);
	bool                     init11(bool vsync = true);
	bool                     init12(bool vsync = true);
	D3D11_BLEND_DESC         initColorBlending11(BOOL enableBlend);
	D3D12_BLEND_DESC         initColorBlending12(BOOL enableBlend);
	D3D11_DEPTH_STENCIL_DESC initDepthStencilBuffer11(BOOL enableDepth, D3D11_COMPARISON_FUNC compareOperation = D3D11_COMPARISON_LESS);
	D3D12_DEPTH_STENCIL_DESC initDepthStencilBuffer12(BOOL enableDepth, D3D12_COMPARISON_FUNC compareOperation = D3D12_COMPARISON_FUNC_LESS);
	D3D11_RASTERIZER_DESC    initRasterizer11(D3D11_CULL_MODE cullMode = D3D11_CULL_BACK,      D3D11_FILL_MODE fillMode = D3D11_FILL_SOLID);
	D3D12_RASTERIZER_DESC    initRasterizer12(D3D12_CULL_MODE cullMode = D3D12_CULL_MODE_BACK, D3D12_FILL_MODE fillMode = D3D12_FILL_MODE_SOLID);
	void                     release();
	void                     transitionResource(ID3D12Resource* resource, D3D12_RESOURCE_STATES oldState, D3D12_RESOURCE_STATES newState);
	void                     wait();

};

#endif
#endif
