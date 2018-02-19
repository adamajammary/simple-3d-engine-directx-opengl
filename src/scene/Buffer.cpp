#include "Buffer.h"

Buffer::Buffer(std::vector<unsigned int> &indices)
{
	this->bufferStride = 0;
	this->id           = 0;

	#ifdef _WINDOWS
		this->bufferDX11           = nullptr;
		this->bufferDX12           = nullptr;
		this->indexBufferViewDX12 = {};

		this->ConstantBuffersDX11[NR_OF_SHADERS]     = {};
		this->ConstantBuffersDX12[NR_OF_SHADERS]     = {};
		this->ConstantBufferHeapsDX12[NR_OF_SHADERS] = {};
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

	switch (Utils::SelectedGraphicsAPI) {
	#ifdef _WINDOWS
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
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, (indices.size() * sizeof(unsigned int)), &indices[0], GL_STATIC_DRAW);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		}

		break;
	case GRAPHICS_API_VULKAN:
		break;
	default:
		break;
	}
}

Buffer::Buffer(std::vector<float> &data)
{
	this->id = 0;
	glCreateBuffers(1, &this->id);

	if (id > 0) {
		glBindBuffer(GL_ARRAY_BUFFER, id);
		glBufferData(GL_ARRAY_BUFFER, (data.size() * sizeof(float)), &data[0], GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
}

Buffer::Buffer(std::vector<float> &vertices, std::vector<float> &normals, std::vector<float> &texCoords)
{
	this->bufferDX11           = nullptr;
	this->bufferDX12           = nullptr;
	this->bufferStride         = 0;
	this->vertexBufferViewDX12 = {};

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
	}
}

Buffer::Buffer()
{
	this->bufferDX11           = nullptr;
	this->bufferDX12           = nullptr;
	this->bufferStride         = 0;
	this->indexBufferViewDX12  = {};
	this->vertexBufferViewDX12 = {};
	this->id                   = 0;
}

Buffer::~Buffer()
{
	switch (Utils::SelectedGraphicsAPI) {
	#ifdef _WINDOWS
	case GRAPHICS_API_DIRECTX11:
		_RELEASEP(this->bufferDX11);
		_RELEASEP(this->bufferDX12);

		for (int i = 0; i < NR_OF_SHADERS; i++) {
			_RELEASEP(this->InputLayoutsDX11[i]);
			_RELEASEP(this->RasterizerStatesDX11[i]);
			_RELEASEP(this->RootSignaturesDX12[i]);
		}

		break;
	case GRAPHICS_API_DIRECTX12:
		break;
	#endif
	case GRAPHICS_API_OPENGL:
		glDeleteBuffers(1, &this->id);
		this->id = 0;
		break;
	case GRAPHICS_API_VULKAN:
		break;
	default:
		break;
	}
}

ID3D11Buffer* Buffer::BufferDX11()
{
	return this->bufferDX11;
}

UINT Buffer::BufferStride()
{
	return this->bufferStride;
}

GLuint Buffer::ID()
{
	return this->id;
}

D3D12_INDEX_BUFFER_VIEW Buffer::IndexBufferViewDX12()
{
	return this->indexBufferViewDX12;
}

D3D12_VERTEX_BUFFER_VIEW Buffer::VertexBufferViewDX12()
{
	return this->vertexBufferViewDX12;
}
