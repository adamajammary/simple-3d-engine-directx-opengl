#include "FrameBuffer.h"

FrameBuffer::FrameBuffer(const wxSize &size, FBOType fboType, TextureType textureType)
{
	this->size    = size;
	this->texture = nullptr;
	this->type    = fboType;

	switch (RenderEngine::SelectedGraphicsAPI) {
		#if defined _WINDOWS
		case GRAPHICS_API_DIRECTX11:
		case GRAPHICS_API_DIRECTX12: this->createTextureDX(textureType); break;
		#endif
		case GRAPHICS_API_OPENGL: this->createTextureGL(textureType); break;
		case GRAPHICS_API_VULKAN: this->createTextureVK(textureType); break;
		default: throw;
	}
}

FrameBuffer::FrameBuffer()
{
	this->fbo     = 0;
	this->size    = {};
	this->texture = nullptr;
	this->type    = FBO_UNKNOWN;
}

FrameBuffer::~FrameBuffer()
{
	this->size = {};

	_DELETEP(this->texture);

	if (this->fbo > 0) {
		glDeleteFramebuffers(1, &this->fbo);
		this->fbo = 0;
	}
}

void FrameBuffer::createTextureDX(TextureType textureType)
{
	switch (this->type) {
	case FBO_COLOR:
		this->texture = new Texture(
			this->type, textureType, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, this->size.GetWidth(), this->size.GetHeight()
		);
		break;
	case FBO_DEPTH:
		//DXGI_FORMAT_D32_FLOAT, DXGI_FORMAT_D32_FLOAT_S8X24_UINT, DXGI_FORMAT_D24_UNORM_S8_UINT
		this->texture = new Texture(
			this->type, textureType, DXGI_FORMAT_R32_TYPELESS, this->size.GetWidth(), this->size.GetHeight()
		);
		break;
	default:
		throw;
	}
}

void FrameBuffer::createTextureGL(TextureType textureType)
{
	glCreateFramebuffers(1, &this->fbo);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		wxString fboType;
		switch (this->type) {
			case FBO_COLOR: fboType = "color"; break;
			case FBO_DEPTH: fboType = "depth"; break;
			default: throw;
		}
		wxMessageBox("ERROR! Failed to create a " + fboType + " frame buffer", RenderEngine::Canvas.Window->GetTitle().c_str(), wxOK | wxICON_ERROR);
		throw;
	}

	this->Bind();

	GLenum buffers[1];
	GLenum texType;

	switch (textureType) {
		case TEXTURE_2D:      texType = GL_TEXTURE_2D;       break;
		case TEXTURE_CUBEMAP: texType = GL_TEXTURE_CUBE_MAP; break;
		default: throw;
	}

	switch (this->type) {
	case FBO_COLOR:
		this->texture = new Texture(
			GL_SRGB8, GL_RGB, GL_UNSIGNED_BYTE, texType, GL_COLOR_ATTACHMENT0,
			this->size.GetWidth(), this->size.GetHeight()
		);

		buffers[0] = GL_COLOR_ATTACHMENT0;
		glDrawBuffers(1, buffers);

		break;
	case FBO_DEPTH:
		this->texture = new Texture(
			GL_RGB8, GL_RGB, GL_FLOAT, texType, GL_DEPTH_ATTACHMENT,
			this->size.GetWidth(), this->size.GetHeight()
		);

		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);

		break;
	default:
		throw;
	}

	this->Unbind();
}

void FrameBuffer::createTextureVK(TextureType textureType)
{
	switch (this->type) {
	case FBO_COLOR:
		this->texture = new Texture(
			this->type, textureType, VK_FORMAT_R8G8B8A8_SRGB, this->size.GetWidth(), this->size.GetHeight()
		);
		break;
	case FBO_DEPTH:
		this->texture = new Texture(
			this->type, textureType, VK_FORMAT_R8G8B8A8_UNORM, this->size.GetWidth(), this->size.GetHeight()
		);
		break;
	default:
		throw;
	}
}

void FrameBuffer::Bind()
{
	switch (RenderEngine::SelectedGraphicsAPI) {
	#if defined _WINDOWS
	case GRAPHICS_API_DIRECTX11:
		RenderEngine::Canvas.DX->Bind11(this->type, this->texture);
		break;
	case GRAPHICS_API_DIRECTX12:
		RenderEngine::Canvas.DX->Bind12(this->type, this->texture);
		break;
	#endif
	case GRAPHICS_API_OPENGL:
		glBindTexture(GL_TEXTURE_2D,       0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

		glViewport(0, 0, this->size.GetWidth(), this->size.GetHeight());
		glBindFramebuffer(GL_FRAMEBUFFER,  this->fbo);

		break;
	case GRAPHICS_API_VULKAN:
		break;
	default:
		throw;
	}
}

void FrameBuffer::Unbind()
{
	switch (RenderEngine::SelectedGraphicsAPI) {
	#if defined _WINDOWS
	case GRAPHICS_API_DIRECTX11:
		RenderEngine::Canvas.DX->Unbind11();
		break;
	case GRAPHICS_API_DIRECTX12:
		RenderEngine::Canvas.DX->Unbind12(this->type, this->texture);
		break;
	#endif
	case GRAPHICS_API_OPENGL:
		glBindTexture(GL_TEXTURE_2D,       0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
		glBindFramebuffer(GL_FRAMEBUFFER,  0);

		glViewport(0, 0, RenderEngine::Canvas.Size.GetWidth(), RenderEngine::Canvas.Size.GetHeight());
		break;
	case GRAPHICS_API_VULKAN:
		break;
	default:
		throw;
	}
}

GLuint FrameBuffer::FBO()
{
    return this->fbo;
}

Texture* FrameBuffer::GetTexture()
{
	return this->texture;
}

wxSize FrameBuffer::Size()
{
	return this->size;
}

FBOType FrameBuffer::Type()
{
	return this->type;
}
