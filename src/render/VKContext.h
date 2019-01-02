#ifndef S3DE_GLOBALS_H
	#include "../globals.h"
#endif

#ifndef S3DE_VKCONTEXT_H
#define S3DE_VKCONTEXT_H

enum VKAttachmentDesc
{
	VK_COLOR_ATTACHMENT, VK_DEPTH_STENCIL_ATTACHMENT, VK_RESOLVE_ATTACHMENT, NR_OF_VK_ATTACHMENTS
};

enum VKQueueType
{
	VK_QUEUE_PRESENTATION, VK_QUEUE_GRAPHICS, NR_OF_VK_QUEUES
};

struct VKQueue
{
	int32_t Index = -1;
	VkQueue Queue = nullptr;

	VKQueue(int32_t index)
	{
		this->Index = index;
	}
};

struct VKSwapchain
{
private:
	VkDevice deviceContext;

public:
	std::vector<VkImage>     Images;
	std::vector<VkImageView> ImageViews;
	VkExtent2D               Size          = {};
	VkSurfaceFormatKHR       SurfaceFormat = {};
	VkSwapchainKHR           SwapChain     = nullptr;

	VKSwapchain(VkDevice deviceContext)
	{
		this->deviceContext = deviceContext;
	}

	~VKSwapchain()
	{
		for (auto imageView : this->ImageViews) {
			if (imageView != nullptr) {
				vkDestroyImageView(this->deviceContext, imageView, nullptr);
				imageView = nullptr;
			}
		}

		if (this->SwapChain != nullptr) {
			vkDestroySwapchainKHR(this->deviceContext, this->SwapChain, nullptr);
			this->SwapChain = nullptr;
		}
	}
};

struct VKSwapChainSupport
{
	VkSurfaceCapabilitiesKHR        SurfaceCapabilities = {};
	std::vector<VkSurfaceFormatKHR> SurfaceFormats;
	std::vector<VkPresentModeKHR>   PresentModes;
};

class VKContext
{
public:
	VKContext(bool vsync = true);
	~VKContext();

private:
	std::vector<VkImage>         colorImages;
	std::vector<VkDeviceMemory>  colorImageMemories;
	std::vector<VkImageView>     colorImageViews;
	std::vector<VkCommandBuffer> commandBuffers;
	VkCommandPool                commandPool;
	std::vector<VkImage>         depthBufferImages;
	std::vector<VkDeviceMemory>  depthBufferImageMemories;
	std::vector<VkImageView>     depthBufferImageViews;
	VkPhysicalDevice             device;
	VkDevice                     deviceContext;
	std::vector<VkFramebuffer>   frameBuffers;
	std::vector<VkFence>         frameFences;
	uint32_t                     frameIndex;
	uint32_t                     imageIndex;
	VkInstance                   instance;
	bool                         isOK;
	uint32_t                     multiSampleCount;
	std::vector<VKQueue*>        queues;
	VkRenderPass                 renderPasses[NR_OF_RENDER_PASSES];
	std::vector<VkSemaphore>     semDrawComplete;
	std::vector<VkSemaphore>     semImageAvailable;
	VkSurfaceKHR                 surface;
	VKSwapchain*                 swapChain;
	VKSwapChainSupport*          swapChainSupport;
	bool                         vSync;

	#if defined _DEBUG
		VkDebugReportCallbackEXT debugCallback; 
	#endif

public:
	void            Clear(float r, float g, float b, float a, FrameBuffer* fbo = nullptr, VkCommandBuffer cmdBuffer = nullptr);
	VkCommandBuffer CommandBufferBegin();
	void            CommandBufferEnd(VkCommandBuffer cmdBuffer);
	int             CreateIndexBuffer(const std::vector<uint32_t> &indices, Buffer* buffer);
	int             CreateShaderModule(const wxString &shaderFile, const wxString &stage, VkShaderModule* shaderModule);
	int             CreateTexture(const std::vector<uint8_t*> &imagePixels, Texture* texture, VkFormat imageFormat);
	int             CreateTextureBuffer(FBOType fboType, TextureType textureType, VkFormat imageFormat, Texture* texture);
	int             CreateVertexBuffer(const std::vector<float> &vertices, const std::vector<float> &normals, const std::vector<float> &texCoords, Buffer* buffer);
	void            DestroyBuffer(VkBuffer* buffer, VkDeviceMemory* bufferMemory);
	void            DestroyFramebuffer(VkFramebuffer* frameBuffer);
	void            DestroyPipeline(VkPipeline* pipeline);
	void            DestroyPipelineLayout(VkPipelineLayout* pipelineLayout);
	void            DestroyShaderModule(VkShaderModule* shaderModule);
	void            DestroyTexture(VkImage* image, VkDeviceMemory* imageMemory, VkImageView* textureImageView, VkSampler* sampler);
	void            DestroyUniformSet(VkDescriptorPool* uniformPool, VkDescriptorSetLayout* uniformLayout);
	int             Draw(Component* mesh, ShaderProgram* shaderProgram, const DrawProperties &properties = {});
	int             InitPipelines(Buffer* buffer);
	bool            IsOK();
	void            Present(VkCommandBuffer cmdBuffer = nullptr);
	void            ResetPipelines();
	bool            ResetSwapChain();
	void            SetVSync(bool enable);

private:
	void                                   blitImage(VkCommandBuffer cmdBuffer, VkImage image, int mipWidth, int mipHeight, int index);
	int                                    copyBuffer(VkBuffer sourceBuffer, VkBuffer destinationBuffer, VkDeviceSize bufferSize);
	int                                    copyBufferToImage(VkBuffer buffer, VkImage image, int colorComponents, uint32_t width, uint32_t height, uint32_t index = 0);
	int                                    copyImage(VkImage image, VkFormat imageFormat, uint32_t mipLevels, TextureType textureType, VkImageLayout oldLayout, VkImageLayout newLayout);
	int                                    createBuffer(VkDeviceSize size, VkBufferUsageFlags useFlags, VkMemoryPropertyFlags memoryFlags, VkBuffer* buffer, VkDeviceMemory* bufferMemory);
	int                                    createImage(uint32_t width, uint32_t height, uint32_t mipLevels, uint32_t sampleCount, VkFormat format, VkImageTiling tiling, VkImageUsageFlags useFlags, VkMemoryPropertyFlags memoryFlags, TextureType textureType, VkImage* image, VkDeviceMemory* imageMemory);
	VkSampler                              createImageSampler(float mipLevels, float sampleCount, VkSamplerCreateInfo &samplerInfo);
	VkImageView                            createImageView(VkImage image, VkFormat imageFormat, uint32_t mipLevels, TextureType textureType, VkImageAspectFlags aspectFlags);
	int                                    createMipMaps(VkImage image, VkFormat imageFormat, int width, int height, uint32_t mipLevels);
	int                                    createPipeline(ShaderProgram* shaderProgram, VkPipeline* pipeline, VkPipelineLayout pipelineLayout, FBOType fboType, const std::vector<VkVertexInputAttributeDescription> &attribsDescs, const VkVertexInputBindingDescription &attribsBindingDesc);
	int                                    createPipelineLayout(Buffer* buffer);
	int                                    createUniformBuffers(Buffer* buffer);
	int                                    createUniformLayout(VkDescriptorSetLayout* uniformLayout);
	int                                    createUniformPool(VkDescriptorPool* uniformPool);
	int                                    createUniformSet(Buffer* buffer);
	bool                                   deviceSupportsExtensions(VkPhysicalDevice device, const std::vector<const char*> &extensions);
	bool                                   deviceSupportsFeatures(VkPhysicalDevice device, const VkPhysicalDeviceFeatures &features);
	wxString                               getApiVersion(VkPhysicalDevice device);
	VkFormat                               getDepthBufferFormat();
	VkPhysicalDevice                       getDevice(const std::vector<const char*> &extensions, const VkPhysicalDeviceFeatures &features);
	wxString                               getDeviceName(VkPhysicalDevice device);
	std::vector<VKQueue*>                  getDeviceQueueSupport(VkPhysicalDevice device);
	VKSwapChainSupport*                    getDeviceSwapChainSupport(VkPhysicalDevice device);
	VkFormat                               getImageFormat(const std::vector<VkFormat> &formats, VkImageTiling imageTiling, VkFormatFeatureFlags features);
	uint32_t                               getMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
	uint32_t                               getMultiSampleCount();
	VkPresentModeKHR                       getPresentationMode(const std::vector<VkPresentModeKHR> &presentationModes);
	VkSurfaceFormatKHR                     getSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &surfaceFormats);
	VkExtent2D                             getSurfaceSize(const VkSurfaceCapabilitiesKHR &capabilities);
	VkPipelineColorBlendStateCreateInfo    initColorBlending(VkPipelineColorBlendAttachmentState &attachment, VkBool32 enableBlending);
	int                                    initColorImages();
	std::vector<VkCommandBuffer>           initCommandBuffers(uint32_t bufferCount);
	VkCommandPool                          initCommandPool();
	VkPipelineDepthStencilStateCreateInfo  initDepthStencilBuffer(VkBool32 enableDepth, VkCompareOp compareOperation = VK_COMPARE_OP_LESS);
	int                                    initDepthStencilImages();
	VkDevice                               initDeviceContext();
	std::vector<VkFramebuffer>             initFramebuffers();
	VkInstance                             initInstance();
	VkPipelineMultisampleStateCreateInfo   initMultisampling(uint32_t sampleCount);
	VkPipelineRasterizationStateCreateInfo initRasterizer(VkCullModeFlags cullMode = VK_CULL_MODE_BACK_BIT, VkPolygonMode polyMode = VK_POLYGON_MODE_FILL);
	VkRenderPass                           initRenderPass(VkFormat format, uint32_t sampleCount, VKAttachmentDesc attachmentDesc);
	VkSurfaceKHR                           initSurface();
	VKSwapchain*                           initSwapChain();
	bool                                   initSync();
	VkPipelineViewportStateCreateInfo      initViewport();
	bool                                   init(bool vsync = true);
	void                                   release();
	void                                   releaseSwapChain(bool releaseSupport);
	void                                   transitionImageLayout(VkCommandBuffer cmdBuffer, VkImageMemoryBarrier &imageMemBarrier, VkPipelineStageFlagBits destStage);
	bool                                   updateSwapChain(bool updateSupport);

	#if defined _DEBUG
		static VKAPI_ATTR VkBool32 VKAPI_CALL debugLog(VkDebugReportFlagsEXT f, VkDebugReportObjectTypeEXT ot, uint64_t o, size_t l, int32_t c, const char* lp, const char* m, void* ud);
	#endif

};

#endif
