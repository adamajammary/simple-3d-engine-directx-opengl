#ifndef GE3D_GLOBALS_H
	#include "../globals.h"
#endif

#ifndef GE3D_VKCONTEXT_H
#define GE3D_VKCONTEXT_H

enum VKAttachmentDesc
{
	VK_COLOR_ATTACHMENT, VK_DEPTH_STENCIL_ATTACHMENT, NR_OF_VK_ATTACHMENTS
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
		//_DELETEP(this->Size);
		//_DELETEP(this->SurfaceFormat);

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
	VKContext(GraphicsAPI api, bool vsync = true);
	~VKContext();

private:
	VkPipelineColorBlendStateCreateInfo*    colorBlendInfo;
	std::vector<VkCommandBuffer>            commandBuffers;
	VkCommandPool                           commandPool;
	std::vector<VkImage>                    depthBufferImages;
	std::vector<VkDeviceMemory>             depthBufferImageMemories;
	std::vector<VkImageView>                depthBufferImageViews;
	VkPipelineDepthStencilStateCreateInfo*  depthStencilInfo;
	VkPhysicalDevice                        device;
	VkDevice                                deviceContext;
	std::vector<VkFramebuffer>              frameBuffers;
	std::vector<VkFence>                    frameFences;
	uint32_t                                frameIndex;
	uint32_t                                imageIndex;
	VkInstance                              instance;
	bool                                    isOK;
	VkPipelineMultisampleStateCreateInfo*   multisampleInfo;
	//VkPipelineLayout                        pipelineLayout;
	std::vector<VKQueue*>                   queues;
	VkPipelineRasterizationStateCreateInfo* rasterizationInfo;
	VkRenderPass                            renderPass;
	std::vector<VkSemaphore>                semDrawComplete;
	std::vector<VkSemaphore>                semImageAvailable;
	//VkPipelineShaderStageCreateInfo*        shaderStages[2];
	VkSurfaceKHR                            surface;
	VKSwapchain*                            swapChain;
	VKSwapChainSupport*                     swapChainSupport;
	//VkDescriptorSetLayout                   uniformLayout;
	//VkDescriptorPool                        uniformPool;
	//VkDescriptorSet                         uniformSet;
	VkPipelineViewportStateCreateInfo*      viewportState;
	bool                                    vSync;

	#if defined _DEBUG
		VkDebugReportCallbackEXT debugCallback; 
	#endif

public:
	void       Clear(float r, float g, float b, float a);
	int        CreateIndexBuffer(const std::vector<uint32_t> &indices, VkBuffer* indexBuffer, VkDeviceMemory* indexBufferMemory);
	int        CreatePipelineLayout(VkPipelineLayout* pipelineLayout, VkDescriptorSetLayout uniformLayout);
	int        CreateShaderModule(const wxString &shaderFile, const wxString &stage, VkShaderModule* shaderModule);
	int        CreateTexture(uint32_t width, uint32_t height, const std::vector<uint8_t*> &imagePixels, VkImage* textureImage, VkDeviceMemory* textureImageMemory, VkImageView* textureImageView, VkSampler* sampler, VkSamplerCreateInfo &samplerInfo);
	int        CreateUniformBuffers(VkBuffer* uniformBuffer, VkDeviceMemory* uniformBufferMemory);
	int        CreateUniformSet(VkDescriptorSet* uniformSet, VkDescriptorPool* uniformPool, VkDescriptorSetLayout* uniformLayout);
	int        CreateVertexBuffer(const std::vector<float> &vertices, const std::vector<float> &normals, const std::vector<float> &texCoords, VkPipeline* pipelines, VkPipelineLayout pipelineLayout, VkBuffer* vertexBuffer, VkDeviceMemory* vertexBufferMemory);
	void       DestroyBuffer(VkBuffer* buffer, VkDeviceMemory* bufferMemory);
	void       DestroyPipeline(VkPipeline* pipeline);
	void       DestroyPipelineLayout(VkPipelineLayout* pipelineLayout);
	void       DestroyShaderModule(VkShaderModule* shaderModule);
	void       DestroyTexture(VkImage* image, VkDeviceMemory* imageMemory, VkImageView* textureImageView, VkSampler* sampler);
	void       DestroyUniformSet(VkDescriptorPool* uniformPool, VkDescriptorSetLayout* uniformLayout);
	int        Draw(Mesh* mesh, ShaderProgram* shaderProgram, bool enableClipping = false, const glm::vec3 &clipMax = glm::vec3(0.0f, 0.0f, 0.0f), const glm::vec3 &clipMin = glm::vec3(0.0f, 0.0f, 0.0f));
	bool       IsOK();
	void       Present();
	void       ResetPipelines();
	bool       ResetSwapChain();
	void       SetVSync(bool enable);

private:
	VkCommandBuffer                         commandBufferBegin();
	void                                    commandBufferEnd(VkCommandBuffer commandBuffer);
	int                                     copyBuffer(VkBuffer sourceBuffer, VkBuffer destinationBuffer, VkDeviceSize bufferSize);
	int                                     copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
	int                                     copyImage(VkImage image, VkFormat imageFormat, VkImageLayout oldLayout, VkImageLayout newLayout);
	int                                     createBuffer(VkDeviceSize size, VkBufferUsageFlags useFlags, VkMemoryPropertyFlags memoryFlags, VkBuffer* buffer, VkDeviceMemory* bufferMemory);
	int                                     createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags useFlags, VkMemoryPropertyFlags memoryFlags, VkImage* textureImage, VkDeviceMemory* textureImageMemory);
	VkImageView                             createImageView(VkImage image, VkFormat imageFormat, VkImageAspectFlags aspectFlags);
	int                                     createPipeline(ShaderProgram* shaderProgram, VkPipeline* pipeline, VkPipelineLayout pipelineLayout, VkVertexInputBindingDescription attribsBindingDesc, VkVertexInputAttributeDescription attribsDesc[NR_OF_ATTRIBS]);
	int                                     createUniformLayout(VkDescriptorSetLayout* uniformLayout);
	int                                     createUniformPool(VkDescriptorPool* uniformPool);
	bool                                    deviceSupportsExtensions(VkPhysicalDevice device, const std::vector<const char*> &extensions);
	bool                                    deviceSupportsFeatures(VkPhysicalDevice device, const VkPhysicalDeviceFeatures &features);
	wxString                                getApiVersion(VkPhysicalDevice device);
	VkFormat                                getDepthBufferFormat();
	VkPhysicalDevice                        getDevice(const std::vector<const char*> &extensions, const VkPhysicalDeviceFeatures &features);
	wxString                                getDeviceName(VkPhysicalDevice device);
	std::vector<VKQueue*>                   getDeviceQueueSupport(VkPhysicalDevice device);
	VKSwapChainSupport*                     getDeviceSwapChainSupport(VkPhysicalDevice device);
	VkFormat                                getImageFormat(const std::vector<VkFormat> &formats, VkImageTiling imageTiling, VkFormatFeatureFlags features);
	uint32_t                                getMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
	VkPresentModeKHR                        getPresentationMode(const std::vector<VkPresentModeKHR> &presentationModes);
	VkSurfaceFormatKHR                      getSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &surfaceFormats);
	VkExtent2D                              getSurfaceSize(const VkSurfaceCapabilitiesKHR &capabilities);
	VkPipelineColorBlendStateCreateInfo*    initColorBlending();
	std::vector<VkCommandBuffer>            initCommandBuffers(uint32_t bufferCount);
	VkCommandPool                           initCommandPool();
	VkPipelineDepthStencilStateCreateInfo*  initDepthStencilBuffer();
	bool                                    initDepthStencilImages(std::vector<VkImage> &images, std::vector<VkDeviceMemory> &imageMemories, std::vector<VkImageView> &imageViews);
	VkDevice                                initDeviceContext();
	std::vector<VkFramebuffer>              initFramebuffers();
	VkInstance                              initInstance();
	VkPipelineMultisampleStateCreateInfo*   initMultisampling();
	VkPipelineRasterizationStateCreateInfo* initRasterizer();
	VkRenderPass                            initRenderPass();
	VkSurfaceKHR                            initSurface();
	VKSwapchain*                            initSwapChain(VKSwapChainSupport* swapChainSupport, VkSurfaceKHR surface);
	bool                                    initSync();
	//VkDescriptorSetLayout                   initUniformLayout(uint32_t binding, VkShaderStageFlags shaderFlags);
	VkPipelineViewportStateCreateInfo*      initViewportState();
	bool                                    init(bool vsync = true);
	void                                    release();
	void                                    releaseSwapChain(bool releaseSupport);
	bool                                    updateSwapChain(bool updateSupport);

	#if defined _DEBUG
		static VKAPI_ATTR VkBool32 VKAPI_CALL debugLog(VkDebugReportFlagsEXT f, VkDebugReportObjectTypeEXT ot, uint64_t o, size_t l, int32_t c, const char* lp, const char* m, void* ud);
	#endif

};

#endif
