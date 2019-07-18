#include "Texture.h"

// 2D TEXTURE FROM IMAGE
Texture::Texture(wxImage* image, bool repeat, bool flipY, bool transparent, const glm::vec2 &scale)
{
	if (image != nullptr)
	{
		this->flipY       = flipY;
		this->repeat      = repeat;
		this->Scale       = scale;
		this->type        = TEXTURE_2D;
		this->transparent = transparent;

		switch (RenderEngine::SelectedGraphicsAPI) {
		#if defined _WINDOWS
		case GRAPHICS_API_DIRECTX11:
		case GRAPHICS_API_DIRECTX12:
			this->loadTextureImagesDX({ image });
			break;
		#endif
		case GRAPHICS_API_OPENGL:
			this->glType = GL_TEXTURE_2D;

			glEnable(this->glType);
			glCreateTextures(this->glType, 1, &this->id);

			if (this->id > 0)
				this->loadTextureImageGL(image);

			break;
		case GRAPHICS_API_VULKAN:
			this->loadTextureImagesVK({ image });
			break;
		default:
			throw;
		}
	}
	else
	{
		wxMessageBox(
			"ERROR: Failed to create a texture from image: wxImage is NULL",
			RenderEngine::Canvas.Window->GetTitle().c_str(), wxOK | wxICON_ERROR
		);
	}
}

// 2D TEXTURE FROM IMAGE FILE
Texture::Texture(const wxString& imageFile, bool repeat, bool flipY, bool transparent, const glm::vec2& scale)
{
	wxImage* image = nullptr;

	if (!imageFile.empty())
		image = Utils::LoadImageFile(imageFile);

	if (image != nullptr)
	{
		this->flipY       = flipY;
		this->imageFiles  = { imageFile };
		this->repeat      = repeat;
		this->Scale       = scale;
		this->type        = TEXTURE_2D;
		this->transparent = transparent;

		switch (RenderEngine::SelectedGraphicsAPI) {
		#if defined _WINDOWS
		case GRAPHICS_API_DIRECTX11:
		case GRAPHICS_API_DIRECTX12:
			this->loadTextureImagesDX({ image });
			break;
		#endif
		case GRAPHICS_API_OPENGL:
			this->glType = GL_TEXTURE_2D;

			glEnable(this->glType);
			glCreateTextures(this->glType, 1, &this->id);

			if (this->id > 0)
				this->loadTextureImageGL(image);

			break;
		case GRAPHICS_API_VULKAN:
			this->loadTextureImagesVK({ image });
			break;
		default:
			throw;
		}

		image->Destroy();
	}
	else
	{
		wxMessageBox(
			("ERROR: Failed to create a texture from image file: " + imageFile),
			RenderEngine::Canvas.Window->GetTitle().c_str(), wxOK | wxICON_ERROR
		);
	}
}

// CUBEMAP TEXTURE FROM 6 IMAGE FILES
Texture::Texture(const std::vector<wxString> &imageFiles, bool repeat, bool flipY, bool transparent, const glm::vec2 &scale)
{
	wxImage*              image;
	std::vector<wxImage*> images;

	if ((int)imageFiles.size() != MAX_TEXTURES)
		throw;

	for (int i = 0; i < MAX_TEXTURES; i++) {
		if ((image = Utils::LoadImageFile(imageFiles[i])) != nullptr)
			images.push_back(image);
	}

	if ((int)images.size() != MAX_TEXTURES)
		throw;

	this->flipY       = flipY;
	this->imageFiles  = imageFiles;
	this->Scale       = scale;
	this->type        = TEXTURE_CUBEMAP;
	this->transparent = transparent;

	switch (RenderEngine::SelectedGraphicsAPI) {
	#if defined _WINDOWS
	case GRAPHICS_API_DIRECTX11:
	case GRAPHICS_API_DIRECTX12:
		this->loadTextureImagesDX(images);
		break;
	#endif
	case GRAPHICS_API_OPENGL:
		this->glType = GL_TEXTURE_CUBE_MAP;

		glEnable(this->glType);
		glCreateTextures(this->glType, 1, &this->id);

		if (this->id > 0) {
			for (int i = 0; i < MAX_TEXTURES; i++)
				this->loadTextureImageGL(images[i], true, i);
		}

		break;
	case GRAPHICS_API_VULKAN:
		this->loadTextureImagesVK(images);
		break;
	default:
		throw;
	}

	for (auto img : images) {
		if (img != nullptr)
			img->Destroy();
	}
}

// FRAMEBUFFER TEXTURE (DIRECTX)
Texture::Texture(FBOType fboType, TextureType textureType, DXGI_FORMAT format, const wxSize &size)
{
	int                result;
	D3D11_SAMPLER_DESC samplerDesc11 = {};

	this->repeat      = true;
	this->Scale       = { 1.0f, 1.0f };
	this->size        = size;
	this->type        = textureType;
	this->transparent = false;

	switch (RenderEngine::SelectedGraphicsAPI) {
	#if defined _WINDOWS
	case GRAPHICS_API_DIRECTX11:
		this->bufferViewPort11.TopLeftX = 0.0f;
		this->bufferViewPort11.TopLeftY = 0.0f;
		this->bufferViewPort11.Width    = (FLOAT)this->size.GetWidth();
		this->bufferViewPort11.Height   = (FLOAT)this->size.GetHeight();
		this->bufferViewPort11.MinDepth = 0.0f;
		this->bufferViewPort11.MaxDepth = 1.0f;

		samplerDesc11.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;

		switch (this->type) {
		case TEXTURE_2D_ARRAY:
			samplerDesc11.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
			samplerDesc11.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
			samplerDesc11.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;

			// SET BORDER VALUES FOR CLAMP WRAPPING
			for (int i = 0; i < 4; i++)
				samplerDesc11.BorderColor[i] = 1.0f;

			break;
		default:
			samplerDesc11.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
			samplerDesc11.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
			samplerDesc11.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;

			break;
		}

		result = RenderEngine::Canvas.DX->CreateTextureBuffer11(fboType, format, samplerDesc11, this);

		if (result < 0)
			wxMessageBox("ERROR: Failed to create a texture frame buffer.", RenderEngine::Canvas.Window->GetTitle().c_str(), wxOK | wxICON_ERROR);

		break;
	case GRAPHICS_API_DIRECTX12:
		this->bufferViewPort12.TopLeftX = 0.0f;
		this->bufferViewPort12.TopLeftY = 0.0f;
		this->bufferViewPort12.Width    = (FLOAT)this->size.GetWidth();
		this->bufferViewPort12.Height   = (FLOAT)this->size.GetHeight();
		this->bufferViewPort12.MinDepth = 0.0f;
		this->bufferViewPort12.MaxDepth = 1.0f;

		this->SamplerDesc12.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;

		switch (this->type) {
		case TEXTURE_2D_ARRAY:
			this->SamplerDesc12.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
			this->SamplerDesc12.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
			this->SamplerDesc12.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;

			// SET BORDER VALUES FOR CLAMP WRAPPING
			for (int i = 0; i < 4; i++)
				this->SamplerDesc12.BorderColor[i] = 1.0f;

			break;
		default:
			this->SamplerDesc12.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
			this->SamplerDesc12.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
			this->SamplerDesc12.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;

			break;
		}

		result = RenderEngine::Canvas.DX->CreateTextureBuffer12(fboType, format, this);

		if (result < 0)
			wxMessageBox("ERROR: Failed to create a texture frame buffer.", RenderEngine::Canvas.Window->GetTitle().c_str(), wxOK | wxICON_ERROR);

		break;
	#endif
	default:
		throw;
	}
}

// FRAMEBUFFER TEXTURE (VULKAN)
Texture::Texture(FBOType fboType, TextureType textureType, VkFormat imageFormat, const wxSize &size)
{
	this->repeat      = true;
	this->Scale       = { 1.0f, 1.0f };
	this->size        = size;
	this->type        = textureType;
	this->transparent = false;

	this->bufferViewPort.x        = 0.0f;
	this->bufferViewPort.y        = 0.0f;
	this->bufferViewPort.width    = (float)this->size.GetWidth();
	this->bufferViewPort.height   = (float)this->size.GetHeight();
	this->bufferViewPort.minDepth = 0.0f;
	this->bufferViewPort.maxDepth = 1.0f;

	this->SamplerInfo.magFilter = VK_FILTER_NEAREST;
	this->SamplerInfo.minFilter = VK_FILTER_NEAREST;

	switch (this->type) {
	case TEXTURE_2D:
		this->SamplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		this->SamplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;

		break;
	case TEXTURE_2D_ARRAY:
		this->SamplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		this->SamplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;

		// SET BORDER VALUES FOR CLAMP WRAPPING
		this->SamplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

		break;
	case TEXTURE_CUBEMAP:
	case TEXTURE_CUBEMAP_ARRAY:
		this->SamplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		this->SamplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		this->SamplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;

		break;
	default:
		throw;
	}

	int result = RenderEngine::Canvas.VK->CreateTextureBuffer(fboType, imageFormat, this);

	if (result < 0)
		wxMessageBox("ERROR: Failed to create a texture frame buffer.", RenderEngine::Canvas.Window->GetTitle().c_str(), wxOK | wxICON_ERROR);
}

// FRAMEBUFFER TEXTURE (OPENGL)
Texture::Texture(GLint format, TextureType textureType, const wxSize &size)
{
	this->repeat      = true;
	this->Scale       = { 1.0f, 1.0f };
	this->size        = size;
	this->transparent = false;
	this->type        = textureType;
	this->glType      = Utils::ToGlTextureType(this->type);

	glEnable(this->glType);
	glCreateTextures(this->glType, 1, &this->id);
	glBindTexture(this->glType, this->id);

	GLfloat borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	GLsizei width         = this->size.GetWidth();
	GLsizei height        = this->size.GetHeight();

	glTexParameteri(this->glType, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(this->glType, GL_TEXTURE_MAX_LEVEL,  0);

	glTexParameteri(this->glType, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(this->glType, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	switch (this->glType) {
	case GL_TEXTURE_2D:
		glTexParameteri(this->glType, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(this->glType, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glTexStorage2D(this->glType, 1, format, width, height);

		break;
	case GL_TEXTURE_CUBE_MAP:
		glTexParameteri(this->glType, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(this->glType, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(this->glType, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		for (GLenum i = 0; i < MAX_TEXTURES; i++)
			glTexStorage2D((GL_TEXTURE_CUBE_MAP_POSITIVE_X + i), 1, format, width, height);

		break;
	case GL_TEXTURE_2D_ARRAY:
		glTexParameteri(this->glType, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(this->glType, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		/*glTexParameteri(this->glType, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(this->glType, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

		// SET BORDER VALUES FOR CLAMP WRAPPING
		glTexParameterfv(this->glType, GL_TEXTURE_BORDER_COLOR, borderColor);*/

		glTexStorage3D(this->glType, 1, format, width, height, MAX_LIGHT_SOURCES);

		break;
	case GL_TEXTURE_CUBE_MAP_ARRAY:
		glTexParameteri(this->glType, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(this->glType, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(this->glType, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		glTexStorage3D(this->glType, 1, format, width, height, (MAX_LIGHT_SOURCES * MAX_TEXTURES));

		break;
	default:
		throw;
	}

	glBindTexture(this->glType, 0);
}

Texture::Texture()
{
	this->type = TEXTURE_2D;

	switch (RenderEngine::SelectedGraphicsAPI) {
	#if defined _WINDOWS
	case GRAPHICS_API_DIRECTX11:
		this->ColorBuffer11  = nullptr;
		this->Resource11     = nullptr;
		this->samplerState11 = nullptr;
		this->SRV11          = nullptr;

		for (int i = 0; i < MAX_LIGHT_SOURCES; i++)
			this->DepthBuffers11[i] = nullptr;

		break;
	case GRAPHICS_API_DIRECTX12:
		this->ColorBuffer12 = nullptr;
		this->Resource12    = nullptr;

		for (int i = 0; i < MAX_LIGHT_SOURCES; i++)
			this->DepthBuffers12[i] = nullptr;

		break;
	#endif
	case GRAPHICS_API_OPENGL:
		this->id   = 0;
		this->glType = GL_TEXTURE_2D;

		glEnable(this->glType);

		break;
	case GRAPHICS_API_VULKAN:
		this->Image       = nullptr;
		this->ImageMemory = nullptr;
		this->ImageView   = nullptr;
		this->Sampler     = nullptr;
		this->SamplerInfo = {};

		for (uint32_t i = 0; i < MAX_LIGHT_SOURCES; i++) {
			this->DepthViews[i]   = nullptr;
			this->DepthBuffers[i] = nullptr;
		}

		break;
	default:
		throw;
	}
}

Texture::~Texture()
{
	#if defined _WINDOWS
		_RELEASEP(this->ColorBuffer11);
		_RELEASEP(this->Resource11);
		_RELEASEP(this->samplerState11);
		_RELEASEP(this->SRV11);

		for (int i = 0; i < MAX_LIGHT_SOURCES; i++)
			_RELEASEP(this->DepthBuffers11[i]);

		_RELEASEP(this->ColorBuffer12);
		_RELEASEP(this->Resource12);

		for (int i = 0; i < MAX_LIGHT_SOURCES; i++)
			_RELEASEP(this->DepthBuffers12[i]);
	#endif

	if (this->id > 0)
		glDeleteTextures(1, &this->id);

	RenderEngine::Canvas.VK->DestroyTexture(&this->Image, &this->ImageMemory, &this->ImageView, &this->Sampler);
	RenderEngine::Canvas.VK->DestroyFramebuffer(&this->BufferVK);

	for (uint32_t i = 0; i < MAX_LIGHT_SOURCES; i++) {
		RenderEngine::Canvas.VK->DestroyTexture(nullptr, nullptr, &this->DepthViews[i], nullptr);
		RenderEngine::Canvas.VK->DestroyFramebuffer(&this->DepthBuffers[i]);
	}


	this->imageFiles.clear();
}

VkViewport Texture::BufferViewPort()
{
	return this->bufferViewPort;
}

#if defined _WINDOWS
D3D11_VIEWPORT Texture::BufferViewPort11()
{
	return this->bufferViewPort11;
}

D3D12_VIEWPORT Texture::BufferViewPort12()
{
	return this->bufferViewPort12;
}
#endif

bool Texture::FlipY()
{
	return this->flipY;
}

GLuint Texture::ID()
{
	return this->id;
}

wxString Texture::ImageFile(int index)
{
	if ((this->imageFiles.size() == 1) && (this->imageFiles[0] != Utils::RESOURCE_IMAGES["emptyTexture"]))
		return this->imageFiles[0];
	else if ((this->imageFiles.size() > 1) && (index < (int)this->imageFiles.size()) && (this->imageFiles[index] != Utils::RESOURCE_IMAGES["emptyTexture"]))
		return this->imageFiles[index];

	return "";
}

bool Texture::IsOK()
{
	switch (RenderEngine::SelectedGraphicsAPI) {
	#if defined _WINDOWS
	case GRAPHICS_API_DIRECTX11:
		return (this->Resource11 != nullptr);
	case GRAPHICS_API_DIRECTX12:
		return (this->Resource12 != nullptr);
	#endif
	case GRAPHICS_API_OPENGL:
		return (this->id > 0);
	case GRAPHICS_API_VULKAN:
		return ((this->ImageView != nullptr) && (this->Sampler != nullptr));
	default:
		throw;
	}

	return false;
}

#if defined _WINDOWS
void Texture::loadTextureImagesDX(const std::vector<wxImage*> &images)
{
	std::vector<wxImage>  images2;
	std::vector<uint8_t*> pixels2;

	for (auto image : images)
	{
		wxImage image2 = (this->flipY ? image->Mirror(false) : *image);
		images2.push_back(image2);
		pixels2.push_back(Utils::ToRGBA(image2));
	}

	if (!images2.empty())
	{
		DXGI_FORMAT        format        = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		D3D11_SAMPLER_DESC samplerDesc11 = {};

		this->size        = wxSize(images2[0].GetWidth(), images2[0].GetHeight());
		this->mipLevels   = ((uint32_t)(std::floor(std::log2(std::max(this->size.GetWidth(), this->size.GetHeight())))) + 1);
		this->type = (images.size() > 1 ? TEXTURE_CUBEMAP : TEXTURE_2D);
		this->transparent = (this->transparent && images2[0].HasAlpha());

		switch (RenderEngine::SelectedGraphicsAPI) {
		case GRAPHICS_API_DIRECTX11:
			this->setFilteringDX11(samplerDesc11);

			if (this->type == TEXTURE_CUBEMAP)
				this->setWrappingCubemapDX11(samplerDesc11);
			else
				this->setWrappingDX11(samplerDesc11);

			if (RenderEngine::Canvas.DX->CreateTexture11(FBO_UNKNOWN, pixels2, format, samplerDesc11, this) < 0)
				wxMessageBox(("ERROR: Texture::loadTextureImagesDX11: Failed to create a texture from image files."), RenderEngine::Canvas.Window->GetTitle().c_str(), wxOK | wxICON_ERROR);

			break;
		case GRAPHICS_API_DIRECTX12:
			this->setFilteringDX12(this->SamplerDesc12);

			if (this->type == TEXTURE_CUBEMAP)
				this->setWrappingCubemapDX12(this->SamplerDesc12);
			else
				this->setWrappingDX12(this->SamplerDesc12);

			if (RenderEngine::Canvas.DX->CreateTexture12(FBO_UNKNOWN, pixels2, format, this) < 0)
				wxMessageBox(("ERROR: Texture::loadTextureImagesDX12: Failed to create a texture from image files."), RenderEngine::Canvas.Window->GetTitle().c_str(), wxOK | wxICON_ERROR);

			break;
		default:
			throw;
		}
	}

	for (auto pixels : pixels2)
		std::free(pixels);

	for (auto image : images2) {
		if (this->flipY)
			image.Destroy();
	}
}
#endif

void Texture::loadTextureImageGL(wxImage* image, bool cubemap, int index)
{
	wxImage  image2    = (this->flipY ? image->Mirror(false) : *image);
	GLenum   formatIn  = GL_SRGB8_ALPHA8;
	GLenum   formatOut = GL_RGBA;
	uint8_t* pixels    = Utils::ToRGBA(image2);

	glBindTexture(this->glType, this->id);

	this->size        = wxSize(image2.GetWidth(), image2.GetHeight());
	this->mipLevels   = ((uint32_t)(std::floor(std::log2(std::max(this->size.GetWidth(), this->size.GetHeight())))) + 1);
	this->transparent = (this->transparent && image2.HasAlpha());

	if (this->transparent)
		this->setAlphaBlendingGL(true);

	if (cubemap)
	{
		glTexParameteri(this->glType, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(this->glType, GL_TEXTURE_MAX_LEVEL,  0);

		glTexImage2D(
			(GL_TEXTURE_CUBE_MAP_POSITIVE_X + index), 0, formatIn,
			image2.GetWidth(), image2.GetHeight(), 0, formatOut, GL_UNSIGNED_BYTE, pixels
		);

		this->setWrappingCubemapGL();
		this->setFilteringGL(false);
	}
	else
	{
		glTexParameteri(this->glType, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(this->glType, GL_TEXTURE_MAX_LEVEL,  this->mipLevels - 1);

		// https://www.khronos.org/opengl/wiki/Common_Mistakes#Automatic_mipmap_generation
		//glTexStorage2D(this->glType, this->mipLevels, formatIn, image2.GetWidth(), image2.GetHeight());
		//glTexSubImage2D(this->glType, 0, 0, 0, image2.GetWidth(), image2.GetHeight(), formatOut, GL_UNSIGNED_BYTE, pixels);
		glTexImage2D(this->glType, 0, formatIn, image2.GetWidth(), image2.GetHeight(), 0, formatOut, GL_UNSIGNED_BYTE, pixels);

		glGenerateMipmap(this->glType);

		this->setWrappingGL();
		this->setFilteringGL(true);
	}
	
	if (this->transparent)
		this->setAlphaBlendingGL(false);

	glBindTexture(this->glType, 0);

	std::free(pixels);

	if (this->flipY)
		image2.Destroy();
}

void Texture::loadTextureImagesVK(const std::vector<wxImage*> &images)
{
	std::vector<wxImage>  images2;
	std::vector<uint8_t*> pixels2;

	for (auto image : images)
	{
		wxImage image2 = (this->flipY ? image->Mirror(false) : *image);
		images2.push_back(image2);
		pixels2.push_back(Utils::ToRGBA(image2));
	}

	if (!images2.empty())
	{
		VkFormat format = VK_FORMAT_R8G8B8A8_SRGB;

		this->size        = wxSize(images2[0].GetWidth(), images2[0].GetHeight());
		this->mipLevels   = ((uint32_t)(std::floor(std::log2(std::max(this->size.GetWidth(), this->size.GetHeight())))) + 1);
		this->transparent = (this->transparent && images2[0].HasAlpha());

		this->setFilteringVK(this->SamplerInfo);

		if (images.size() > 1)
			this->setWrappingCubemapVK(this->SamplerInfo);
		else
			this->setWrappingVK(this->SamplerInfo);

		if (RenderEngine::Canvas.VK->CreateTexture(pixels2, this, format) < 0)
			wxMessageBox(("ERROR: Texture::loadTextureImagesVK: Failed to create a texture from image files."), RenderEngine::Canvas.Window->GetTitle().c_str(), wxOK | wxICON_ERROR);
	}

	for (auto pixels : pixels2)
		std::free(pixels);

	for (auto image : images2) {
		if (this->flipY)
			image.Destroy();
	}
}

uint32_t Texture::MipLevels()
{
	return this->mipLevels;
}

void Texture::reload()
{
	wxImage* image = nullptr;

	if (!this->imageFiles.empty() && (this->id > 0))
		image = Utils::LoadImageFile(this->imageFiles[0]);

	if (image != nullptr) {
		this->loadTextureImageGL(image);
		image->Destroy();
	}
}

bool Texture::Repeat()
{
	return this->repeat;
}

void Texture::setAlphaBlendingGL(bool enable)
{
	if (enable) {
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	} else {
		glDisable(GL_BLEND);
	}
}

#if defined _WINDOWS
void Texture::setFilteringDX11(D3D11_SAMPLER_DESC &samplerDesc)
{
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
}

void Texture::setFilteringDX12(D3D12_SAMPLER_DESC &samplerDesc)
{
	samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
}
#endif

void Texture::setFilteringGL(bool mipmap)
{
	glTexParameteri(this->glType, GL_TEXTURE_MIN_FILTER, (mipmap ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR));
	glTexParameteri(this->glType, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void Texture::setFilteringVK(VkSamplerCreateInfo &samplerInfo)
{
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
}

#if defined _WINDOWS
void Texture::setWrappingDX11(D3D11_SAMPLER_DESC &samplerDesc)
{
	samplerDesc.AddressU = (this->repeat && !this->transparent ? D3D11_TEXTURE_ADDRESS_WRAP : D3D11_TEXTURE_ADDRESS_CLAMP);
	samplerDesc.AddressV = samplerDesc.AddressW = samplerDesc.AddressU;
}

void Texture::setWrappingDX12(D3D12_SAMPLER_DESC &samplerDesc)
{
	samplerDesc.AddressU = (this->repeat && !this->transparent ? D3D12_TEXTURE_ADDRESS_MODE_WRAP : D3D12_TEXTURE_ADDRESS_MODE_CLAMP);
	samplerDesc.AddressV = samplerDesc.AddressW = samplerDesc.AddressU;
}
#endif

void Texture::setWrappingGL()
{
	glTexParameteri(this->glType, GL_TEXTURE_WRAP_S, (this->repeat && !this->transparent ? GL_REPEAT : GL_CLAMP_TO_EDGE));
	glTexParameteri(this->glType, GL_TEXTURE_WRAP_T, (this->repeat && !this->transparent ? GL_REPEAT : GL_CLAMP_TO_EDGE));
}

void Texture::setWrappingVK(VkSamplerCreateInfo &samplerInfo)
{
	samplerInfo.addressModeU = (this->repeat && !this->transparent ? VK_SAMPLER_ADDRESS_MODE_REPEAT : VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
	samplerInfo.addressModeV = samplerInfo.addressModeW = samplerInfo.addressModeU;
}

#if defined _WINDOWS
void Texture::setWrappingCubemapDX11(D3D11_SAMPLER_DESC &samplerDesc)
{
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = samplerDesc.AddressW = samplerDesc.AddressU;
}
#endif

void Texture::setWrappingCubemapDX12(D3D12_SAMPLER_DESC &samplerDesc)
{
	samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samplerDesc.AddressV = samplerDesc.AddressW = samplerDesc.AddressU;
}

void Texture::setWrappingCubemapGL()
{
	glTexParameteri(this->glType, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(this->glType, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(this->glType, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}

void Texture::setWrappingCubemapVK(VkSamplerCreateInfo &samplerInfo)
{
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.addressModeV = samplerInfo.addressModeW = samplerInfo.addressModeU;
}

void Texture::SetFlipY(bool newFlipY)
{
	this->flipY = newFlipY;
	this->reload();
}

void Texture::SetRepeat(bool newRepeat)
{
	this->repeat = newRepeat;
	this->reload();
}

void Texture::SetTransparent(bool newTransparent)
{
	this->transparent = newTransparent;
	this->reload();
}

wxSize Texture::Size()
{
	return this->size;
}

bool Texture::Transparent()
{
	return this->transparent;
}

TextureType Texture::Type()
{
	return this->type;
}

GLenum Texture::TypeGL()
{
    return this->glType;
}
