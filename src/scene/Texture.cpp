#include "Texture.h"

// 2D TEXTURE FROM IMAGE
Texture::Texture(wxImage* image, bool repeat, bool flipY, bool transparent, const glm::vec2 &scale)
{
	if (image != nullptr)
	{
		this->flipY       = flipY;
		this->repeat      = repeat;
		this->Scale       = scale;
		this->transparent = transparent;

		switch (Utils::SelectedGraphicsAPI) {
		#if defined _WINDOWS
		case GRAPHICS_API_DIRECTX11:
		case GRAPHICS_API_DIRECTX12:
			this->loadTextureImagesDX({ image });
			break;
		#endif
		case GRAPHICS_API_OPENGL:
			this->type = GL_TEXTURE_2D;
			glCreateTextures(this->type, 1, &this->id);
			this->loadTextureImageGL(image);
			break;
		case GRAPHICS_API_VULKAN:
			this->loadTextureImageVK(image);
			break;
		}
	}
}

// 2D TEXTURE FROM IMAGE FILE
Texture::Texture(const wxString &imageFile, bool repeat, bool flipY, bool transparent, const glm::vec2 &scale)
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
		this->transparent = transparent;

		switch (Utils::SelectedGraphicsAPI) {
		#if defined _WINDOWS
		case GRAPHICS_API_DIRECTX11:
		case GRAPHICS_API_DIRECTX12:
			this->loadTextureImagesDX({ image });
			break;
		#endif
		case GRAPHICS_API_OPENGL:
			this->type = GL_TEXTURE_2D;
			glCreateTextures(this->type, 1, &this->id);
			this->loadTextureImageGL(image);
			break;
		case GRAPHICS_API_VULKAN:
			this->loadTextureImageVK(image);
			//this->loadTextureImagesVK({ image });
			break;
		}

		image->Destroy();
	}
}

// CUBEMAP TEXTURE FROM 6 IMAGE FILES
Texture::Texture(const std::vector<wxString> &imageFiles, bool repeat, bool flipY, bool transparent, const glm::vec2 &scale)
{
	wxImage*              image;
	std::vector<wxImage*> images;

	if ((int)imageFiles.size() == MAX_TEXTURES)
	{
		for (int i = 0; i < MAX_TEXTURES; i++) {
			if ((image = Utils::LoadImageFile(imageFiles[i])) != nullptr)
				images.push_back(image);
		}
	}

	if ((int)images.size() == MAX_TEXTURES)
	{
		this->flipY       = flipY;
		this->imageFiles  = imageFiles;
		this->Scale       = scale;
		this->transparent = transparent;

		switch (Utils::SelectedGraphicsAPI) {
		#if defined _WINDOWS
		case GRAPHICS_API_DIRECTX11:
		case GRAPHICS_API_DIRECTX12:
			this->loadTextureImagesDX(images);
			break;
		#endif
		case GRAPHICS_API_OPENGL:
			this->type = GL_TEXTURE_CUBE_MAP;

			glCreateTextures(this->type, 1, &this->id);

			for (int i = 0; i < MAX_TEXTURES; i++)
				this->loadTextureImageGL(images[i], true, i);

			break;
		case GRAPHICS_API_VULKAN:
			for (int i = 0; i < MAX_TEXTURES; i++)
				this->loadTextureImageVK(images[i], true, i);
			//this->loadTextureImagesVK(images);
			break;
		}
	}

	for (auto img : images) {
		if (img != nullptr)
			img->Destroy();
	}
}

// 2D FRAMEBUFFER TEXTURE (DIRECTX 11)
Texture::Texture(D3D11_FILTER filter, DXGI_FORMAT format, int width, int height)
{
	D3D11_SAMPLER_DESC samplerDesc11 = {};

	this->repeat = true;
	this->Scale  = glm::vec2(1.0f, 1.0f);

	this->colorBufferViewPort11.TopLeftX = 0.0f;
	this->colorBufferViewPort11.TopLeftY = 0.0f;
	this->colorBufferViewPort11.Width    = (FLOAT)width;
	this->colorBufferViewPort11.Height   = (FLOAT)height;
	this->colorBufferViewPort11.MinDepth = 0.0f;
	this->colorBufferViewPort11.MaxDepth = 1.0f;

	samplerDesc11.Filter   = filter;
	samplerDesc11.AddressU = samplerDesc11.AddressV = samplerDesc11.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;

	RenderEngine::Canvas.DX->CreateTextureBuffer11(
		width, height, format, &this->colorBuffer11, &this->resource11, &this->srv11, samplerDesc11, &this->samplerState11
	);
}

// 2D FRAMEBUFFER TEXTURE (DIRECTX 12)
Texture::Texture(D3D12_FILTER filter, DXGI_FORMAT format, int width, int height)
{
	this->repeat = true;
	this->Scale  = glm::vec2(1.0f, 1.0f);

	this->colorBufferViewPort12.TopLeftX = 0.0f;
	this->colorBufferViewPort12.TopLeftY = 0.0f;
	this->colorBufferViewPort12.Width    = (FLOAT)width;
	this->colorBufferViewPort12.Height   = (FLOAT)height;
	this->colorBufferViewPort12.MinDepth = 0.0f;
	this->colorBufferViewPort12.MaxDepth = 1.0f;

	this->samplerDesc12.Filter   = filter;
	this->samplerDesc12.AddressU = this->samplerDesc12.AddressV = this->samplerDesc12.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;

	RenderEngine::Canvas.DX->CreateTextureBuffer12(
		width, height, format, &this->colorBuffer12, &this->resource12, this->srvDesc12, this->samplerDesc12
	);
}

// 2D FRAMEBUFFER TEXTURE (OPENGL)
Texture::Texture(GLint filter, GLint formatIn, GLenum formatOut, GLenum dataType, GLenum attachment, int width, int height)
{
	this->repeat = true;
	this->Scale  = glm::vec2(1.0f, 1.0f);

	switch (Utils::SelectedGraphicsAPI) {
	case GRAPHICS_API_OPENGL:
		this->type = GL_TEXTURE_2D;

		glCreateTextures(this->type, 1, &this->id);

		glBindTexture(this->type,   this->id);
		glTexParameteri(this->type, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(this->type, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(this->type, GL_TEXTURE_MIN_FILTER, filter);
		glTexParameteri(this->type, GL_TEXTURE_MAG_FILTER, filter);
		glTexImage2D(this->type,    0, formatIn, width, height, 0, formatOut, dataType, nullptr);
		glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, this->type, this->id, 0);

		glBindTexture(this->type, 0);
		break;
	case GRAPHICS_API_VULKAN:
		break;
	}
}

Texture::Texture()
{
	switch (Utils::SelectedGraphicsAPI) {
	#if defined _WINDOWS
	case GRAPHICS_API_DIRECTX11:
		this->colorBuffer11  = nullptr;
		this->resource11     = nullptr;
		this->samplerState11 = nullptr;
		this->srv11          = nullptr;
		break;
	case GRAPHICS_API_DIRECTX12:
		this->colorBuffer12 = nullptr;
		this->resource12    = nullptr;
		break;
	#endif
	case GRAPHICS_API_OPENGL:
		this->id   = 0;
		this->type = GL_TEXTURE_2D;
		break;
	case GRAPHICS_API_VULKAN:
		this->image       = nullptr;
		this->imageMemory = nullptr;
		break;
	}
}

Texture::~Texture()
{
	//switch (Utils::SelectedGraphicsAPI) {
	#if defined _WINDOWS
	//case GRAPHICS_API_DIRECTX11:
		_RELEASEP(this->colorBuffer11);
		_RELEASEP(this->resource11);
		_RELEASEP(this->samplerState11);
		_RELEASEP(this->srv11);
		//break;
	//case GRAPHICS_API_DIRECTX12:
		_RELEASEP(this->colorBuffer12);
		_RELEASEP(this->resource12);
		//break;
	#endif
	//case GRAPHICS_API_OPENGL:
	if (this->id > 0)
	{
		glDeleteTextures(1, &this->id);

		this->id   = 0;
		this->type = GL_TEXTURE_2D;
	}
		//break;
	//case GRAPHICS_API_VULKAN:
	RenderEngine::Canvas.VK->DestroyTexture(&this->image, &this->imageMemory);
		//break;
	//}

	this->imageFiles.clear();
}

ID3D11RenderTargetView* Texture::ColorBuffer11()
{
	return this->colorBuffer11;
}

ID3D12DescriptorHeap* Texture::ColorBuffer12()
{
	return this->colorBuffer12;
}

D3D11_VIEWPORT* Texture::ColorBufferViewPort11()
{
	return &this->colorBufferViewPort11;
}

D3D12_VIEWPORT* Texture::ColorBufferViewPort12()
{
	return &this->colorBufferViewPort12;
}

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
	switch (Utils::SelectedGraphicsAPI) {
		#if defined _WINDOWS
		case GRAPHICS_API_DIRECTX11: return (this->resource11 != nullptr);
		case GRAPHICS_API_DIRECTX12: return (this->resource12 != nullptr);
		#endif
		case GRAPHICS_API_OPENGL:    return (this->id > 0);
		// TODO: VULKAN
		case GRAPHICS_API_VULKAN:    break;
	}

	return false;
}

void Texture::loadTextureImagesDX(const std::vector<wxImage*> &images)
{
	DXGI_FORMAT           format;
	std::vector<wxImage>  images2;
	std::vector<uint8_t*> pixels2;
	D3D11_SAMPLER_DESC    samplerDesc11 = {};

	for (auto image : images)
	{
		wxImage image2 = (this->flipY ? image->Mirror(false) : *image);
		images2.push_back(image2);
		pixels2.push_back(Utils::ToRGBA(&image2));
	}

	if (!images2.empty())
	{
		format            = Utils::GetImageFormatDXGI(&images2[0]);
		this->transparent = (this->transparent && images2[0].HasAlpha());

		if (this->transparent)
			this->setAlphaBlending(this->transparent);

		switch (Utils::SelectedGraphicsAPI) {
		case GRAPHICS_API_DIRECTX11:
			if (images.size() > 1) {
				this->setFilteringDX11(samplerDesc11);
				this->setWrappingCubemapDX11(samplerDesc11);
			} else {
				this->setFilteringDX11(samplerDesc11);
				this->setWrappingDX11(samplerDesc11);
			}

			RenderEngine::Canvas.DX->CreateTexture11(
				pixels2, images2[0].GetWidth(), images2[0].GetHeight(), format,
				&this->resource11, &this->srv11, samplerDesc11, &this->samplerState11
			);

			break;
		case GRAPHICS_API_DIRECTX12:
			if (images.size() > 1) {
				this->setFilteringDX12(this->samplerDesc12);
				this->setWrappingCubemapDX12(this->samplerDesc12);
			} else {
				this->setFilteringDX12(this->samplerDesc12);
				this->setWrappingDX12(this->samplerDesc12);
			}

			RenderEngine::Canvas.DX->CreateTexture12(
				pixels2, images2[0].GetWidth(), images2[0].GetHeight(), format,
				&this->resource12, this->srvDesc12, this->samplerDesc12
			);

			break;
		}

		if (this->transparent)
			this->setAlphaBlending(false);
	}

	for (auto pixels : pixels2)
		std::free(pixels);

	for (auto image : images2) {
		if (this->flipY)
			image.Destroy();
	}
}

void Texture::loadTextureImageGL(wxImage* image, bool cubemap, int index)
{
	wxImage  image2 = (this->flipY ? image->Mirror(false) : *image);
	uint8_t* pixels = (image2.HasAlpha() ? Utils::ToRGBA(&image2) : image2.GetData());
	GLenum   format = Utils::GetImageFormat(&image2);

	glBindTexture(this->type, this->id);

	this->transparent = (this->transparent && image2.HasAlpha());

	if (this->transparent)
		this->setAlphaBlending(true);

	if (cubemap)
	{
		this->setFilteringGL(false);
		this->setWrappingCubemapGL();

		// TEXTURE_CUBE_MAP_POSITIVE_X 0x8515   // RIGHT
		// TEXTURE_CUBE_MAP_NEGATIVE_X 0x8516   // LEFT
		// TEXTURE_CUBE_MAP_POSITIVE_Y 0x8517   // TOP
		// TEXTURE_CUBE_MAP_NEGATIVE_Y 0x8518   // BOTTOM
		// TEXTURE_CUBE_MAP_POSITIVE_Z 0x8519   // BACK  ???
		// TEXTURE_CUBE_MAP_NEGATIVE_Z 0x851A   // FRONT ???
		glTexImage2D(
			(GL_TEXTURE_CUBE_MAP_POSITIVE_X + index), 0, format,
			image2.GetWidth(), image2.GetHeight(), 0, format, GL_UNSIGNED_BYTE, pixels
		);
	}
	else
	{
		this->setFilteringGL(true);
		this->setWrappingGL();

		glTexImage2D(
			this->type, 0, format, image2.GetWidth(), image2.GetHeight(), 0, format, GL_UNSIGNED_BYTE, pixels
		);

		glGenerateMipmap(this->type);
	}
	
	if (this->transparent)
		this->setAlphaBlending(false);

	glBindTexture(this->type, 0);

	if (image2.HasAlpha())
		std::free(pixels);

	if (this->flipY)
		image2.Destroy();
}

void Texture::loadTextureImagesVK(const std::vector<wxImage*> &images)
{
}

void Texture::loadTextureImageVK(wxImage* image, bool cubemap, int index)
{
	wxImage  image2 = (this->flipY ? image->Mirror(false) : *image);
	//uint8_t*     pixels = (image2.HasAlpha() ? Utils::ToRGBA(&image2) : image2.GetData());
	uint8_t* pixels = Utils::ToRGBA(&image2);
	//VkDeviceSize size   = (image2.GetWidth() * image2.GetHeight() * (image2.HasAlpha() ? 4 : 3));

	this->transparent = (this->transparent && image2.HasAlpha());

	//if (this->transparent)
	//	this->setAlphaBlending(true);

	RenderEngine::Canvas.VK->CreateTexture(
		(uint32_t)image2.GetWidth(), (uint32_t)image2.GetHeight(), pixels, &this->image, &this->imageMemory
	);

	//if (image2.HasAlpha())
	std::free(pixels);

	if (this->flipY)
		image2.Destroy();

	int x = 0;
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

ID3D12Resource* Texture::Resource12()
{
	return this->resource12;
}

ID3D11SamplerState* Texture::SamplerState11()
{
	return this->samplerState11;
}

const D3D12_SAMPLER_DESC* Texture::SamplerDesc12()
{
	return &this->samplerDesc12;
}

void Texture::setAlphaBlending(bool enable)
{
	switch (Utils::SelectedGraphicsAPI) {
	case GRAPHICS_API_OPENGL:
		if (enable) {
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		} else {
			glDisable(GL_BLEND);
		}
		break;
	case GRAPHICS_API_VULKAN:
		break;
	}
}

void Texture::setFilteringDX11(D3D11_SAMPLER_DESC &samplerDesc)
{
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
}

void Texture::setFilteringDX12(D3D12_SAMPLER_DESC &samplerDesc)
{
	samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
}

void Texture::setFilteringGL(bool mipmap)
{
	switch (Utils::SelectedGraphicsAPI) {
	case GRAPHICS_API_OPENGL:
		glTexParameteri(this->type, GL_TEXTURE_MIN_FILTER, (mipmap ? GL_NEAREST_MIPMAP_LINEAR : GL_LINEAR));
		glTexParameteri(this->type, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		break;
	case GRAPHICS_API_VULKAN:
		break;
	}
}

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

void Texture::setWrappingGL()
{
	switch (Utils::SelectedGraphicsAPI) {
	case GRAPHICS_API_OPENGL:
		glTexParameteri(this->type, GL_TEXTURE_WRAP_S, (this->repeat && !this->transparent ? GL_REPEAT : GL_CLAMP_TO_EDGE));
		glTexParameteri(this->type, GL_TEXTURE_WRAP_T, (this->repeat && !this->transparent ? GL_REPEAT : GL_CLAMP_TO_EDGE));
		break;
	case GRAPHICS_API_VULKAN:
		break;
	}
}

void Texture::setWrappingCubemapDX11(D3D11_SAMPLER_DESC &samplerDesc)
{
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = samplerDesc.AddressW = samplerDesc.AddressU;
}

void Texture::setWrappingCubemapDX12(D3D12_SAMPLER_DESC &samplerDesc)
{
	samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samplerDesc.AddressV = samplerDesc.AddressW = samplerDesc.AddressU;
}

void Texture::setWrappingCubemapGL()
{
	switch (Utils::SelectedGraphicsAPI) {
	case GRAPHICS_API_OPENGL:
		glTexParameteri(this->type, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(this->type, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(this->type, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		break;
	case GRAPHICS_API_VULKAN:
		break;
	}
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

ID3D11ShaderResourceView* Texture::SRV11()
{
	return this->srv11;
}

const D3D12_SHADER_RESOURCE_VIEW_DESC* Texture::SRVDesc12()
{
	return &this->srvDesc12;
}
bool Texture::Transparent()
{
	return this->transparent;
}

GLenum Texture::Type()
{
    return this->type;
}
