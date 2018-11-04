#include "Buffer.h"

Buffer::Buffer(std::vector<uint32_t> &indices)
{
	this->init();

	switch (RenderEngine::SelectedGraphicsAPI) {
	#if defined _WINDOWS
	case GRAPHICS_API_DIRECTX11:
		RenderEngine::Canvas.DX->CreateIndexBuffer11(indices, this);
		break;
	case GRAPHICS_API_DIRECTX12:
		RenderEngine::Canvas.DX->CreateIndexBuffer12(indices, this);
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
		RenderEngine::Canvas.VK->CreateIndexBuffer(indices, this);
		break;
	default:
		break;
	}
}

Buffer::Buffer(std::vector<float> &data)
{
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
	this->init();

	this->normals   = normals;
	this->texCoords = texCoords;
	this->vertices  = vertices;

	switch (RenderEngine::SelectedGraphicsAPI) {
	case GRAPHICS_API_DIRECTX11:
		RenderEngine::Canvas.DX->CreateVertexBuffer11(vertices, normals, texCoords, this);
		RenderEngine::Canvas.DX->CreateConstantBuffers11(this);
		break;
	case GRAPHICS_API_DIRECTX12:
		RenderEngine::Canvas.DX->CreateVertexBuffer12(vertices, normals, texCoords, this);
		RenderEngine::Canvas.DX->CreateConstantBuffers12(this);
		break;
	case GRAPHICS_API_VULKAN:
		RenderEngine::Canvas.VK->CreateVertexBuffer(vertices, normals, texCoords, this);
		RenderEngine::Canvas.VK->InitPipelines(this);
		break;
	}
}

Buffer::Buffer()
{
	this->init();
}

Buffer::~Buffer()
{
	#if defined _WINDOWS
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
			_RELEASEP(this->PipelineStatesFBODX12[i]);
			_RELEASEP(this->RootSignaturesDX12[i]);
		}

		_RELEASEP(this->VertexBufferDX11);
		_RELEASEP(this->VertexBufferDX12);
	#endif

	if (this->id > 0) {
		glDeleteBuffers(1, &this->id);
		this->id = 0;
	}

	for (uint32_t i = 0; i < NR_OF_UBOS_VK; i++)
		RenderEngine::Canvas.VK->DestroyBuffer(&this->Uniform.Buffers[i], &this->Uniform.BufferMemories[i]);

	for (uint32_t i = 0; i < NR_OF_SHADERS; i++) {
		RenderEngine::Canvas.VK->DestroyPipeline(&this->Pipeline.Pipelines[i]);
		RenderEngine::Canvas.VK->DestroyPipeline(&this->Pipeline.PipelinesFBO[i]);
	}

	RenderEngine::Canvas.VK->DestroyPipelineLayout(&this->Pipeline.Layout);
	RenderEngine::Canvas.VK->DestroyUniformSet(&this->Uniform.Pool, &this->Uniform.Layout);
	RenderEngine::Canvas.VK->DestroyBuffer(&this->IndexBuffer,  &this->IndexBufferMemory);
	RenderEngine::Canvas.VK->DestroyBuffer(&this->VertexBuffer, &this->VertexBufferMemory);
}

void Buffer::init()
{
	this->BufferStride       = 0;
	this->id                 = 0;
	this->IndexBuffer        = nullptr;
	this->IndexBufferMemory  = nullptr;
	this->Pipeline           = {};
	this->Uniform            = {};
	this->VertexBuffer       = nullptr;
	this->VertexBufferMemory = nullptr;

	#if defined _WINDOWS
		this->VertexBufferDX11     = nullptr;
		this->VertexBufferDX12     = nullptr;
		this->IndexBufferViewDX12  = {};
		this->VertexBufferViewDX12 = {};

		this->BlendStatesDX11[NR_OF_SHADERS]         = {};
		this->ConstantBuffersDX11[NR_OF_SHADERS]     = {};
		this->ConstantBuffersDX12[NR_OF_SHADERS]     = {};
		this->ConstantBufferHeapsDX12[NR_OF_SHADERS] = {};
		this->DepthStencilStatesDX11[NR_OF_SHADERS]  = {};
		this->InputLayoutsDX11[NR_OF_SHADERS]        = {};
		this->RasterizerStatesDX11[NR_OF_SHADERS]    = {};
		this->PipelineStatesDX12[NR_OF_SHADERS]      = {};
		this->PipelineStatesFBODX12[NR_OF_SHADERS]   = {};
		this->RootSignaturesDX12[NR_OF_SHADERS]      = {};
		this->SamplerHeapsDX12[NR_OF_SHADERS]        = {};

		this->MatrixBufferValues    = {};
		this->LightBufferValues     = {};
		this->DefaultBufferValues   = {};
		this->HUDBufferValues       = {};
		this->SkyboxBufferValues    = {};
		this->WireframeBufferValues = {};
		this->TerrainBufferValues   = {};
		this->WaterBufferValues     = {};
	#endif
}

GLuint Buffer::ID()
{
	return this->id;
}

size_t Buffer::Normals()
{
	return this->normals.size();
}

void Buffer::ResetPipelines()
{
	for (uint32_t i = 0; i < NR_OF_SHADERS; i++) {
		RenderEngine::Canvas.VK->DestroyPipeline(&Pipeline.Pipelines[i]);
		RenderEngine::Canvas.VK->DestroyPipeline(&Pipeline.PipelinesFBO[i]);
	}

	RenderEngine::Canvas.VK->DestroyBuffer(&this->VertexBuffer, &this->VertexBufferMemory);
	RenderEngine::Canvas.VK->CreateVertexBuffer(this->vertices, this->normals, this->texCoords, this);
}

size_t Buffer::TexCoords()
{
	return this->texCoords.size();
}

size_t Buffer::Vertices()
{
	return this->vertices.size();
}
