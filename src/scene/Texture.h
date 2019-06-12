#ifndef S3DE_GLOBALS_H
	#include "../globals.h"
#endif

#ifndef S3DE_TEXTURE_H
#define S3DE_TEXTURE_H

class Texture
{
public:
	Texture(wxImage* image, bool repeat = false, bool flipY = false, bool transparent = false, const glm::vec2 &scale = { 1.0f, 1.0f });
	Texture(const wxString &imageFile, bool srgb = false, bool repeat = false, bool flipY = false, bool transparent = false, const glm::vec2 &scale = { 1.0f, 1.0f });
	Texture(const std::vector<wxString> &imageFiles, bool repeat = false, bool flipY = false, bool transparent = false, const glm::vec2 &scale = { 1.0f, 1.0f });
	Texture(FBOType fboType, TextureType textureType, VkFormat imageFormat, const wxSize &size);
	Texture(GLint format, TextureType textureType, const wxSize &size);
	Texture();
	~Texture();

	#if defined _WINDOWS
		Texture(FBOType fboType, TextureType textureType, DXGI_FORMAT format, int width, int height);
	#endif

public:
	VkFramebuffer       BufferVK;
	VkFramebuffer       DepthBuffers[MAX_LIGHT_SOURCES];
	VkImageView         DepthViews[MAX_LIGHT_SOURCES];
	VkImage             Image;
	VkDeviceMemory      ImageMemory;
	VkImageView         ImageView;
	VkSampler           Sampler;
	VkSamplerCreateInfo SamplerInfo;
	glm::vec2           Scale;

	#if defined _WINDOWS
		ID3D11RenderTargetView*         ColorBuffer11;
		ID3D11DepthStencilView*         DepthBuffer11;
		ID3D12DescriptorHeap*           ColorBuffer12;
		ID3D12DescriptorHeap*           DepthBuffer12;
		ID3D11Texture2D*                Resource11;
		ID3D12Resource*                 Resource12;
		D3D12_SAMPLER_DESC              SamplerDesc12;
		ID3D11SamplerState*             SamplerState11;
		ID3D11ShaderResourceView*       SRV11;
		D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc12;
#endif

private:
	VkViewport            bufferViewPort;
	bool                  flipY;
	GLuint                id;
	std::vector<wxString> imageFiles;
	uint32_t              mipLevels;
	bool                  repeat;
	wxSize                size;
	bool                  srgb;
	TextureType           type;
	bool                  transparent;
	GLenum                glType;

	#if defined _WINDOWS
		D3D11_VIEWPORT      bufferViewPort11;
		ID3D11SamplerState* samplerState11;
		D3D12_VIEWPORT      bufferViewPort12;
	#endif

public:
	VkViewport  BufferViewPort();
	bool        FlipY();
	GLuint      ID();
	wxString    ImageFile(int index = 0);
	bool        IsOK();
	uint32_t    MipLevels();
	bool        Repeat();
	void        SetFlipY(bool newFlipY);
	void        SetRepeat(bool newRepeat);
	void        SetTransparent(bool newTransparent);
	wxSize      Size();
	bool        SRGB();
	bool        Transparent();
	TextureType Type();
	GLenum      TypeGL();

	#if defined _WINDOWS
		D3D11_VIEWPORT BufferViewPort11();
		D3D12_VIEWPORT BufferViewPort12();
	#endif

private:
	void loadTextureImageGL(wxImage* image, bool cubemap = false, int index = 0);
	void loadTextureImagesVK(const std::vector<wxImage*> &images);
	void reload();
	void setAlphaBlendingGL(bool enable);
	void setFilteringGL(bool mipmap = true);
	void setFilteringVK(VkSamplerCreateInfo &samplerInfo);
	void setWrappingGL();
	void setWrappingVK(VkSamplerCreateInfo &samplerInfo);
	void setWrappingCubemapGL();
	void setWrappingCubemapVK(VkSamplerCreateInfo &samplerInfo);

	#if defined _WINDOWS
		void loadTextureImagesDX(const std::vector<wxImage*> &images);
		void setFilteringDX11(D3D11_SAMPLER_DESC &samplerDesc);
		void setFilteringDX12(D3D12_SAMPLER_DESC &samplerDesc);
		void setWrappingDX11(D3D11_SAMPLER_DESC &samplerDesc);
		void setWrappingDX12(D3D12_SAMPLER_DESC &samplerDesc);
		void setWrappingCubemapDX11(D3D11_SAMPLER_DESC &samplerDesc);
		void setWrappingCubemapDX12(D3D12_SAMPLER_DESC &samplerDesc);
	#endif

};

#endif
