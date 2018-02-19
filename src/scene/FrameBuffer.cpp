#include "FrameBuffer.h"

FrameBuffer::FrameBuffer(int width, int height)
{
	this->colorTexture = nullptr;
	this->size         = wxSize(width, height);

	switch (Utils::SelectedGraphicsAPI) {
	#ifdef _WINDOWS
	case GRAPHICS_API_DIRECTX11:
	case GRAPHICS_API_DIRECTX12:
		break;
	#endif
	case GRAPHICS_API_OPENGL:
		glCreateFramebuffers(1, &this->colorBuffer);
		break;
	case GRAPHICS_API_VULKAN:
		break;
	}
}

FrameBuffer::FrameBuffer()
{
	this->colorTexture = nullptr;
	this->colorBuffer  = 0;
	this->size         = wxSize(0, 0);
}

FrameBuffer::~FrameBuffer()
{
	this->size = wxSize(0, 0);

	_DELETEP(this->colorTexture);

	switch (Utils::SelectedGraphicsAPI) {
	#ifdef _WINDOWS
	case GRAPHICS_API_DIRECTX11:
	case GRAPHICS_API_DIRECTX12:
		break;
	#endif
	case GRAPHICS_API_OPENGL:
		glDeleteFramebuffers(1,  &this->colorBuffer);
		this->colorBuffer = 0;
		break;
	case GRAPHICS_API_VULKAN:
		break;
	}
}

void FrameBuffer::Bind()
{
	switch (Utils::SelectedGraphicsAPI) {
	#ifdef _WINDOWS
	case GRAPHICS_API_DIRECTX11:
		RenderEngine::Canvas.DX->Bind11(
			this->colorTexture->ColorBuffer11(), nullptr, this->colorTexture->ColorBufferViewPort11()
		);
		break;
	case GRAPHICS_API_DIRECTX12:
		RenderEngine::Canvas.DX->Bind12(
			this->colorTexture->Resource12(),
			&CD3DX12_CPU_DESCRIPTOR_HANDLE(this->colorTexture->ColorBuffer12()->GetCPUDescriptorHandleForHeapStart()),
			nullptr,
			this->colorTexture->ColorBufferViewPort12(), nullptr
		);
		break;
	#endif
	case GRAPHICS_API_OPENGL:
		glBindTexture(GL_TEXTURE_2D, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, this->colorBuffer);
		glViewport(0, 0, this->size.GetWidth(), this->size.GetHeight());
		break;
	case GRAPHICS_API_VULKAN:
		break;
	}
}

void FrameBuffer::Unbind()
{
	switch (Utils::SelectedGraphicsAPI) {
	#ifdef _WINDOWS
	case GRAPHICS_API_DIRECTX11:
		RenderEngine::Canvas.DX->Unbind11();
		break;
	case GRAPHICS_API_DIRECTX12:
		RenderEngine::Canvas.DX->Unbind12(this->colorTexture->Resource12());
		break;
	#endif
	case GRAPHICS_API_OPENGL:
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, RenderEngine::Canvas.Size.GetWidth(), RenderEngine::Canvas.Size.GetHeight());
		break;
	case GRAPHICS_API_VULKAN:
		break;
	}
}

void FrameBuffer::CreateColorTexture()
{
	GLenum buffers[1];

	switch (Utils::SelectedGraphicsAPI) {
	#ifdef _WINDOWS
	case GRAPHICS_API_DIRECTX11:
		this->colorTexture = new Texture(
			D3D11_FILTER_MIN_MAG_MIP_LINEAR, DXGI_FORMAT_R8G8B8A8_UNORM, this->size.GetWidth(), this->size.GetHeight()
		);
		break;
	case GRAPHICS_API_DIRECTX12:
		this->colorTexture = new Texture(
			D3D12_FILTER_MIN_MAG_MIP_LINEAR, DXGI_FORMAT_R8G8B8A8_UNORM, this->size.GetWidth(), this->size.GetHeight()
		);
		break;
	#endif
	case GRAPHICS_API_OPENGL:
		this->Bind();

		buffers[0] = GL_COLOR_ATTACHMENT0;
		glDrawBuffers(1, buffers);

		this->colorTexture = new Texture(
			GL_LINEAR, GL_RGB, GL_RGB, GL_UNSIGNED_BYTE, GL_COLOR_ATTACHMENT0, this->size.GetWidth(), this->size.GetHeight()
		);

		this->Unbind();
		break;
	case GRAPHICS_API_VULKAN:
		break;
	}
}

Texture* FrameBuffer::ColorTexture()
{
	return this->colorTexture;
}

GLuint FrameBuffer::ColorBuffer()
{
    return this->colorBuffer;
}

wxSize FrameBuffer::Size()
{
	return this->size;
}