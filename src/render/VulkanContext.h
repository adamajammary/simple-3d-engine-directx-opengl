#ifndef GE3D_GLOBALS_H
	#include "../globals.h"
#endif

#ifndef GE3D_VULKANCONTEXT_H
#define GE3D_VULKANCONTEXT_H

enum VulkanQueueType { VULKAN_QUEUE_PRESENTATION, VULKAN_QUEUE_GRAPHICS, NR_OF_VULKAN_QUEUES };

struct VulkanQueue
{
	int32_t Index;
	VkQueue Queue = nullptr;

	VulkanQueue(int32_t index)
	{
		this->Index = index;
	}
};

/*struct VulkanQueues
{
	VulkanQueue* Graphics     = nullptr;
	VulkanQueue* Presentation = nullptr;
	size_t       NrOfQueues   = 0;
};*/

struct VulkanSwapchain
{
private:
	VkDevice deviceContext;

public:
	std::vector<VkImage>     Images;
	std::vector<VkImageView> ImageViews;
	VkExtent2D*              Size;
	VkSurfaceFormatKHR*      SurfaceFormat;
	VkSwapchainKHR           SwapChain;

	VulkanSwapchain(VkDevice deviceContext)
	{
		this->deviceContext = deviceContext;
	}

	~VulkanSwapchain()
	{
		_DELETEP(this->Size);
		_DELETEP(this->SurfaceFormat);

		//for (auto image : this->Images) {
		//	if (image != nullptr) {
		//		vkDestroyImage(this->deviceContext, image, nullptr);
		//		image = nullptr;
		//	}
		//}

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

struct VulkanSwapChainSupport
{
	VkSurfaceCapabilitiesKHR        SurfaceCapabilities;
	std::vector<VkSurfaceFormatKHR> SurfaceFormats;
	std::vector<VkPresentModeKHR>   PresentModes;
};

class VulkanContext
{
public:
	VulkanContext(GraphicsAPI api, bool vsync = true);
	~VulkanContext();

private:
	VkPipelineColorBlendStateCreateInfo*    colorBlending;
	std::vector<VkCommandBuffer>            commandBuffers;
	VkCommandPool                           commandPool;
	VkPipelineDepthStencilStateCreateInfo*  depthStencilBuffer;
	VkPhysicalDevice                        device;
	VkDevice                                deviceContext;
	std::vector<VkFramebuffer>              frameBuffers;
	std::vector<VkFence>                    frameFences;
	uint32_t                                frameIndex;
	uint32_t                                imageIndex;
	VkInstance                              instance;
	bool                                    isOK;
	VkPipelineMultisampleStateCreateInfo*   multisampling;
	VkPipelineLayout                        pipelineLayout;
	std::map<VulkanQueueType, VulkanQueue*> queues;
	VkPipelineRasterizationStateCreateInfo* rasterizer;
	VkRenderPass                            renderPass;
	std::vector<VkSemaphore>                semDrawComplete;
	std::vector<VkSemaphore>                semImageAvailable;
	VkPipelineShaderStageCreateInfo*        shaderStages[2];
	VkSurfaceKHR                            surface;
	VulkanSwapchain*                        swapChain;
	VulkanSwapChainSupport*                 swapChainSupport;
	VkDescriptorSetLayout                   uniformLayout;
	VkDescriptorPool                        uniformPool;
	VkDescriptorSet                         uniformSet;
	VkPipelineViewportStateCreateInfo*      viewportState;
	bool                                    vSync;

	#if defined _DEBUG
		VkDebugReportCallbackEXT debugCallback; 
	#endif

public:
	void       Clear(float r, float g, float b, float a);
	int        CreateIndexBuffer(const std::vector<uint32_t> &indices, VkBuffer* indexBuffer, VkDeviceMemory* indexBufferMemory);
	int        CreateShaderModule(const wxString &shaderFile, const wxString &stage, VkShaderModule* shaderModule);
	int        CreateTexture(uint32_t width, uint32_t height, uint8_t* imagePixels, VkImage* textureImage, VkDeviceMemory* textureImageMemory);
	int        CreateUniformBuffers(VkBuffer* uniformBuffer, VkDeviceMemory* uniformBufferMemory);
	int        CreateVertexBuffer(const std::vector<float> &vertices, const std::vector<float> &normals, const std::vector<float> &texCoords, VkPipeline* pipelines, VkBuffer* vertexBuffer, VkDeviceMemory* vertexBufferMemory);
	void       DestroyBuffer(VkBuffer* buffer, VkDeviceMemory* bufferMemory);
	void       DestroyPipeline(VkPipeline* pipeline);
	void       DestroyShaderModule(VkShaderModule* shaderModule);
	void       DestroyTexture(VkImage* image, VkDeviceMemory* imageMemory);
	int        Draw(Mesh* mesh, ShaderProgram* shaderProgram, bool enableClipping = false, const glm::vec3 &clipMax = glm::vec3(0.0f, 0.0f, 0.0f), const glm::vec3 &clipMin = glm::vec3(0.0f, 0.0f, 0.0f));
	bool       IsOK();
	void       Present();
	void       SetVSync(bool enable);
	bool       UpdateSwapChain();

private:
	VkCommandBuffer                         commandBufferBegin();
	void                                    commandBufferEnd(VkCommandBuffer commandBuffer);
	int                                     copyBuffer(VkBuffer sourceBuffer, VkBuffer destinationBuffer, VkDeviceSize bufferSize);
	int                                     copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
	int                                     copyImage(VkImage image, VkFormat imageFormat, VkImageLayout oldLayout, VkImageLayout newLayout);
	int                                     createBuffer(VkDeviceSize size, VkBufferUsageFlags useFlags, VkMemoryPropertyFlags memoryFlags, VkBuffer* buffer, VkDeviceMemory* bufferMemory);
	int                                     createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags useFlags, VkMemoryPropertyFlags memoryFlags, VkImage* textureImage, VkDeviceMemory* textureImageMemory);
	bool                                    deviceSupportsExtensions(VkPhysicalDevice device, const std::vector<const char*> &extensions);
	wxString                                getApiVersion(VkPhysicalDevice device);
	VkPhysicalDevice                        getDevice(const std::vector<const char*> &extensions);
	wxString                                getDeviceName(VkPhysicalDevice device);
	std::map<VulkanQueueType, VulkanQueue*> getDeviceQueueSupport(VkPhysicalDevice device);
	VulkanSwapChainSupport*                 getDeviceSwapChainSupport(VkPhysicalDevice device);
	uint32_t                                getMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
	VkPresentModeKHR                        getPresentationMode(const std::vector<VkPresentModeKHR> &presentationModes);
	VkSurfaceFormatKHR*                     getSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &sufaceFormats);
	VkExtent2D*                             getSurfaceSize(const VkSurfaceCapabilitiesKHR &capabilities);
	VkPipelineColorBlendStateCreateInfo*    initColorBlending();
	std::vector<VkCommandBuffer>            initCommandBuffers(uint32_t bufferCount);
	VkCommandPool                           initCommandPool();
	VkPipelineDepthStencilStateCreateInfo*  initDepthStencilBuffer();
	VkDevice                                initDeviceContext();
	std::vector<VkFramebuffer>              initFramebuffers();
	VkInstance                              initInstance();
	VkPipelineMultisampleStateCreateInfo*   initMultisampling();
	VkPipelineRasterizationStateCreateInfo* initRasterizer();
	bool                                    initPipeline(ShaderProgram* shaderProgram, VkPipeline &pipeline, VkVertexInputBindingDescription attribsBindingDesc, VkVertexInputAttributeDescription attribsDesc[NR_OF_ATTRIBS]);
	VkPipelineLayout                        initPipelineLayout();
	VkRenderPass                            initRenderPass();
	VkSurfaceKHR                            initSurface();
	VulkanSwapchain*                        initSwapChain(VulkanSwapChainSupport* swapChainSupport, VkSurfaceKHR surface);
	bool                                    initSync();
	//VkDescriptorSetLayout                   initUniformLayout(uint32_t binding, VkShaderStageFlags shaderFlags);
	VkDescriptorSetLayout                   initUniformLayout();
	VkDescriptorPool                        initUniformPool();
	VkDescriptorSet                         initUniformSet();
	VkPipelineViewportStateCreateInfo*      initViewportState();
	bool                                    init(bool vsync = true);
	void                                    release();

	#if defined _DEBUG
		static VKAPI_ATTR VkBool32 VKAPI_CALL debugLog(VkDebugReportFlagsEXT f, VkDebugReportObjectTypeEXT ot, uint64_t o, size_t l, int32_t c, const char* lp, const char* m, void* ud);
	#endif

};

#endif
