#include "Buffer.h"

Buffer::Buffer(std::vector<uint32_t> &indices)
{
	////this->bufferStride = 0;
	//this->id                = 0;
	//this->indexBuffer       = nullptr;
	//this->indexBufferMemory = nullptr;

	//#if defined _WINDOWS
	//	this->bufferDX11           = nullptr;
	//	this->bufferDX12           = nullptr;
	//	this->indexBufferViewDX12 = {};

	//	this->ConstantBuffersDX11[NR_OF_SHADERS]     = {};
	//	this->ConstantBuffersDX12[NR_OF_SHADERS]     = {};
	//	this->ConstantBufferHeapsDX12[NR_OF_SHADERS] = {};
	//	this->SamplerHeapsDX12[NR_OF_SHADERS]        = {};

	//	this->MatrixBufferValues  = {};
	//	this->LightBufferValues   = {};
	//	this->DefaultBufferValues = {};
	//	this->HUDBufferValues     = {};
	//	this->SkyboxBufferValues  = {};
	//	this->SolidBufferValues   = {};
	//	this->TerrainBufferValues = {};
	//	this->WaterBufferValues   = {};
	//#endif

	this->init();

	switch (Utils::SelectedGraphicsAPI) {
	#if defined _WINDOWS
	case GRAPHICS_API_DIRECTX11:
		RenderEngine::Canvas.DX->CreateIndexBuffer11(indices, &this->bufferDX11);
		break;
	case GRAPHICS_API_DIRECTX12:
		RenderEngine::Canvas.DX->CreateIndexBuffer12(indices, &this->bufferDX12, this->indexBufferViewDX12);
		break;
	#endif
	case GRAPHICS_API_OPENGL:
		glCreateBuffers(1, &this->id);

		if (id > 0) {
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, (indices.size() * sizeof(uint32_t)), &indices[0], GL_STATIC_DRAW);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		}

		break;
	case GRAPHICS_API_VULKAN:
		RenderEngine::Canvas.VK->CreateIndexBuffer(indices, &this->indexBuffer, &this->indexBufferMemory);
		break;
	default:
		break;
	}
}

Buffer::Buffer(std::vector<float> &data)
{
	//this->id = 0;

	this->init();

	glCreateBuffers(1, &this->id);

	if (id > 0) {
		glBindBuffer(GL_ARRAY_BUFFER, id);
		glBufferData(GL_ARRAY_BUFFER, (data.size() * sizeof(float)), &data[0], GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
}

Buffer::Buffer(std::vector<float> &vertices, std::vector<float> &normals, std::vector<float> &texCoords)
{
	//this->vertexBuffer       = nullptr;
	//this->vertexBufferMemory = nullptr;

	//#if defined _WINDOWS
	//	this->bufferDX11           = nullptr;
	//	this->bufferDX12           = nullptr;
	//	this->bufferStride         = 0;
	//	this->vertexBufferViewDX12 = {};
	//#endif

	this->init();

	switch (Utils::SelectedGraphicsAPI) {
	case GRAPHICS_API_DIRECTX11:
		RenderEngine::Canvas.DX->CreateVertexBuffer11(
			vertices, normals, texCoords, &this->bufferDX11, this->bufferStride,
			this->InputLayoutsDX11, this->RasterizerStatesDX11, this->DepthStencilStatesDX11, this->BlendStatesDX11
		);

		RenderEngine::Canvas.DX->CreateConstantBuffers11(this);

		break;
	case GRAPHICS_API_DIRECTX12:
		RenderEngine::Canvas.DX->CreateVertexBuffer12(
			vertices, normals, texCoords, &this->bufferDX12, this->bufferStride,
			this->vertexBufferViewDX12, this->PipelineStatesDX12, this->RootSignaturesDX12
		);

		RenderEngine::Canvas.DX->CreateConstantBuffers12(this);

		break;
	case GRAPHICS_API_VULKAN:
		RenderEngine::Canvas.VK->CreateVertexBuffer(vertices, normals, texCoords, this->pipelines, &this->vertexBuffer, &this->vertexBufferMemory);
		RenderEngine::Canvas.VK->CreateUniformBuffers(this->uniformBuffers, this->uniformBufferMemories);
		break;
	}
}

Buffer::Buffer()
{
	//this->id                 = 0;
	//this->vertexBuffer       = nullptr;
	//this->vertexBufferMemory = nullptr;

	//#if defined _WINDOWS
	//	this->bufferDX11           = nullptr;
	//	this->bufferDX12           = nullptr;
	//	this->bufferStride         = 0;
	//	this->indexBufferViewDX12  = {};
	//	this->vertexBufferViewDX12 = {};
	//#endif

	this->init();
}

Buffer::~Buffer()
{
	//switch (Utils::SelectedGraphicsAPI) {
	#if defined _WINDOWS
	//case GRAPHICS_API_DIRECTX11:
	for (uint32_t i = 0; i < NR_OF_SHADERS; i++)
	{
		_RELEASEP(this->ConstantBuffersDX11[i]);
		_RELEASEP(this->ConstantBuffersDX12[i]);
		_RELEASEP(this->SamplerHeapsDX12[i]);
		_RELEASEP(this->ConstantBufferHeapsDX12[i]);

		_RELEASEP(this->DepthStencilStatesDX11[i]);
		_RELEASEP(this->BlendStatesDX11[i]);
		_RELEASEP(this->RasterizerStatesDX11[i]);
		_RELEASEP(this->InputLayoutsDX11[i]);

		_RELEASEP(this->PipelineStatesDX12[i]);
		_RELEASEP(this->RootSignaturesDX12[i]);
	}

	_RELEASEP(this->bufferDX11);
	_RELEASEP(this->bufferDX12);

		//break;
	//case GRAPHICS_API_DIRECTX12:
		//break;
	#endif
	//case GRAPHICS_API_OPENGL:
	if (this->id > 0) {
		glDeleteBuffers(1, &this->id);
		this->id = 0;
	}
		//break;
	//case GRAPHICS_API_VULKAN:
	for (uint32_t i = 0; i < NR_OF_SHADERS; i++)
		RenderEngine::Canvas.VK->DestroyPipeline(&this->pipelines[i]);
		//break;
	//default:
		//break;
	//}

	for (uint32_t i = 0; i < NR_OF_UNIFORM_BUFFERS; i++)
		RenderEngine::Canvas.VK->DestroyBuffer(&this->uniformBuffers[i], &this->uniformBufferMemories[i]);

	RenderEngine::Canvas.VK->DestroyBuffer(&this->indexBuffer,   &this->indexBufferMemory);
	RenderEngine::Canvas.VK->DestroyBuffer(&this->vertexBuffer, &this->vertexBufferMemory);
}

void Buffer::init()
{
	this->bufferStride       = 0;
	this->id                 = 0;
	this->indexBuffer        = nullptr;
	this->indexBufferMemory  = nullptr;
	this->vertexBuffer       = nullptr;
	this->vertexBufferMemory = nullptr;

	this->pipelines[NR_OF_SHADERS]             = {};
	this->uniformBuffers[NR_OF_SHADERS]        = {};
	this->uniformBufferMemories[NR_OF_SHADERS] = {};

	#if defined _WINDOWS
		this->bufferDX11           = nullptr;
		this->bufferDX12           = nullptr;
		this->indexBufferViewDX12  = {};
		this->vertexBufferViewDX12 = {};

		this->BlendStatesDX11[NR_OF_SHADERS]         = {};
		this->ConstantBuffersDX11[NR_OF_SHADERS]     = {};
		this->ConstantBuffersDX12[NR_OF_SHADERS]     = {};
		this->ConstantBufferHeapsDX12[NR_OF_SHADERS] = {};
		this->DepthStencilStatesDX11[NR_OF_SHADERS]  = {};
		this->InputLayoutsDX11[NR_OF_SHADERS]        = {};
		this->RasterizerStatesDX11[NR_OF_SHADERS]    = {};
		this->PipelineStatesDX12[NR_OF_SHADERS]      = {};
		this->RootSignaturesDX12[NR_OF_SHADERS]      = {};
		this->SamplerHeapsDX12[NR_OF_SHADERS]        = {};

		this->MatrixBufferValues  = {};
		this->LightBufferValues   = {};
		this->DefaultBufferValues = {};
		this->HUDBufferValues     = {};
		this->SkyboxBufferValues  = {};
		this->SolidBufferValues   = {};
		this->TerrainBufferValues = {};
		this->WaterBufferValues   = {};
	#endif
}

UINT Buffer::BufferStride()
{
	return this->bufferStride;
}

GLuint Buffer::ID()
{
	return this->id;
}

VkBuffer Buffer::IndexBuffer()
{
	return this->indexBuffer;
}

VkPipeline Buffer::Pipeline(ShaderID shaderID)
{
	return (((shaderID >= 0) || (shaderID < NR_OF_SHADERS)) ? this->pipelines[shaderID] : nullptr);
}

VkBuffer Buffer::UniformBuffer(UniformBufferType uniformBuffer)
{
	return (((uniformBuffer >= 0) || (uniformBuffer < NR_OF_UNIFORM_BUFFERS)) ? this->uniformBuffers[uniformBuffer] : nullptr);
}

VkDeviceMemory Buffer::UniformBufferMemory(UniformBufferType uniformBuffer)
{
	return (((uniformBuffer >= 0) || (uniformBuffer < NR_OF_UNIFORM_BUFFERS)) ? this->uniformBufferMemories[uniformBuffer] : nullptr);
}

VkBuffer Buffer::VertexBuffer()
{
	return this->vertexBuffer;
}

#if defined _WINDOWS
ID3D11Buffer* Buffer::BufferDX11()
{
	return this->bufferDX11;
}

D3D12_INDEX_BUFFER_VIEW Buffer::IndexBufferViewDX12()
{
	return this->indexBufferViewDX12;
}

D3D12_VERTEX_BUFFER_VIEW Buffer::VertexBufferViewDX12()
{
	return this->vertexBufferViewDX12;
}
#endif
