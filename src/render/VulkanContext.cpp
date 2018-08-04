#include "VulkanContext.h"

VulkanContext::VulkanContext(GraphicsAPI api, bool vsync)
{
	this->isOK = this->init(vsync);

	if (!this->isOK)
		this->release();
}

VulkanContext::~VulkanContext()
{
	this->release();
}

void VulkanContext::Clear(float r, float g, float b, float a)
{
	//VkImageMemoryBarrier     barrierClearToPresent = {};
	//VkImageMemoryBarrier     barrierPresentToClear = {};
	//VkClearColorValue        clearColor            = { r, g, b, a };
	//VkImageSubresourceRange  imageSubResourceRange = {};
	VkCommandBufferBeginInfo beginCommandInfo = {};
	VkClearValue             clearColor       = { r, g, b, a };
	VkRenderPassBeginInfo    renderPassInfo   = {};

	// WAIT FOR LAST FRAME TO FINISH
	vkWaitForFences(this->deviceContext, 1, &this->frameFences[this->frameIndex], VK_TRUE, UINT64_MAX);
	vkResetFences(this->deviceContext,   1, &this->frameFences[this->frameIndex]);

	VkResult result = vkAcquireNextImageKHR(
		this->deviceContext, this->swapChain->SwapChain, UINT64_MAX,
		this->semImageAvailable[this->frameIndex], nullptr, &this->imageIndex
	);

	if (result == VK_ERROR_OUT_OF_DATE_KHR)
		this->UpdateSwapChain();

	// START RECORDING COMMAND BUFFER
	beginCommandInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginCommandInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

	vkBeginCommandBuffer(this->commandBuffers[this->imageIndex], &beginCommandInfo);

	//// SETUP UP MEMORY BARRIERS FOR TRANSITIONING BETWEEN CLEAR (WRITE) AND PRESENTATION (READ) STATES
	//imageSubResourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	//imageSubResourceRange.levelCount = 1;
	//imageSubResourceRange.layerCount = 1;

	//barrierPresentToClear.sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	//barrierPresentToClear.srcAccessMask       = VK_ACCESS_MEMORY_READ_BIT;
	//barrierPresentToClear.dstAccessMask       = VK_ACCESS_TRANSFER_WRITE_BIT;
	//barrierPresentToClear.oldLayout           = VK_IMAGE_LAYOUT_UNDEFINED;
	//barrierPresentToClear.newLayout           = VK_IMAGE_LAYOUT_GENERAL;
	//barrierPresentToClear.srcQueueFamilyIndex = this->queues[VULKAN_QUEUE_PRESENTATION]->Index;
	//barrierPresentToClear.dstQueueFamilyIndex = this->queues[VULKAN_QUEUE_PRESENTATION]->Index;
	//barrierPresentToClear.image               = this->swapChain->Images[this->imageIndex];
	//barrierPresentToClear.subresourceRange    = imageSubResourceRange;

	//barrierClearToPresent.sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	//barrierClearToPresent.srcAccessMask       = VK_ACCESS_TRANSFER_WRITE_BIT;
	//barrierClearToPresent.dstAccessMask       = VK_ACCESS_MEMORY_READ_BIT;
	//barrierClearToPresent.oldLayout           = VK_IMAGE_LAYOUT_GENERAL;
	//barrierClearToPresent.newLayout           = VK_IMAGE_LAYOUT_GENERAL;
	//barrierClearToPresent.srcQueueFamilyIndex = this->queues[VULKAN_QUEUE_PRESENTATION]->Index;
	//barrierClearToPresent.dstQueueFamilyIndex = this->queues[VULKAN_QUEUE_PRESENTATION]->Index;
	//barrierClearToPresent.image               = this->swapChain->Images[this->imageIndex];
	//barrierClearToPresent.subresourceRange    = imageSubResourceRange;

	//vkCmdPipelineBarrier(
	//	this->commandBuffers[this->imageIndex],
	//	VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
	//	0, 0, NULL, 0, NULL, 1, &barrierPresentToClear
	//);
	//	
	//vkCmdClearColorImage(
	//	this->commandBuffers[this->imageIndex], this->swapChain->Images[this->imageIndex],
	//	VK_IMAGE_LAYOUT_GENERAL, &clearColor, 1, &imageSubResourceRange
	//);
	//	
	//vkCmdPipelineBarrier(
	//	this->commandBuffers[this->imageIndex],
	//	VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
	//	0, 0, NULL, 0, NULL, 1, &barrierClearToPresent
	//);

	// START RENDER PASS
	renderPassInfo.sType             = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.framebuffer       = this->frameBuffers[this->imageIndex];
	renderPassInfo.renderPass        = this->renderPass;
	renderPassInfo.renderArea.extent = *this->swapChain->Size;
	renderPassInfo.pClearValues      = &clearColor;
	renderPassInfo.clearValueCount   = 1;
	//renderPassInfo.renderArea.offset = { 0, 0 };

	vkCmdBeginRenderPass(this->commandBuffers[this->imageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
}

VkCommandBuffer VulkanContext::commandBufferBegin()
{
    //VkCommandBufferAllocateInfo  commandAllocInfo = {};
	//VkCommandBuffer             commandBuffer    = nullptr;
	VkCommandBufferBeginInfo     commandInfo    = {};
	std::vector<VkCommandBuffer> commandBuffers = this->initCommandBuffers(1);

	if (commandBuffers.empty() || (commandBuffers[0] == nullptr))
		return nullptr;

	//commandAllocInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	//commandAllocInfo.commandBufferCount = 1;
	//commandAllocInfo.commandPool        = this->commandPool;
	//commandAllocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    //vkAllocateCommandBuffers(this->deviceContext, &commandAllocInfo, &commandBuffer);

	commandInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	commandInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffers[0], &commandInfo);

    return commandBuffers[0];
}

void VulkanContext::commandBufferEnd(VkCommandBuffer commandBuffer)
{
	VkSubmitInfo queueSubmitInfo = {};

	vkEndCommandBuffer(commandBuffer);

	queueSubmitInfo.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	queueSubmitInfo.commandBufferCount = 1;
	queueSubmitInfo.pCommandBuffers    = &commandBuffer;

	vkQueueSubmit(this->queues[VULKAN_QUEUE_GRAPHICS]->Queue, 1, &queueSubmitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(this->queues[VULKAN_QUEUE_GRAPHICS]->Queue);

	vkFreeCommandBuffers(this->deviceContext, this->commandPool, 1, &commandBuffer);
}

int VulkanContext::copyBuffer(VkBuffer sourceBuffer, VkBuffer destinationBuffer, VkDeviceSize bufferSize)
{
	//VkCommandBufferBeginInfo     copyBeginInfo  = {};
	//std::vector<VkCommandBuffer> copyBuffers    = this->initCommandBuffers(1);
	//VkBufferCopy copyRegion = { 0, 0, bufferSize };
	//VkSubmitInfo copySubmitInfo = {};

	//if (copyBuffers.empty() || (copyBuffers[0] == nullptr))
	//	return -1;

	//copyBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	//copyBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	//copyRegion.size = bufferSize;

	//vkBeginCommandBuffer(copyBuffers[0], &copyBeginInfo);
	VkCommandBuffer commandBuffer = this->commandBufferBegin();

	if (commandBuffer != nullptr)
	{
		VkBufferCopy copyRegion = { 0, 0, bufferSize };

		vkCmdCopyBuffer(commandBuffer, sourceBuffer, destinationBuffer, 1, &copyRegion);
		this->commandBufferEnd(commandBuffer);
	}

	//copySubmitInfo.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	//copySubmitInfo.commandBufferCount = 1;
	//copySubmitInfo.pCommandBuffers    = &copyBuffers[0];

	//vkQueueSubmit(this->queues[VULKAN_QUEUE_GRAPHICS]->Queue, 1, &copySubmitInfo, VK_NULL_HANDLE);
	//vkQueueWaitIdle(this->queues[VULKAN_QUEUE_GRAPHICS]->Queue);

	//vkFreeCommandBuffers(this->deviceContext, this->commandPool, 1, &copyBuffers[0]);

	return 0;
}

int VulkanContext::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
{
	VkCommandBuffer commandBuffer = this->commandBufferBegin();

	if (commandBuffer == nullptr)
		return -1;

	VkBufferImageCopy copyRegion = {};

	copyRegion.imageExtent                 = { width, height, 1 };
	copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	copyRegion.imageSubresource.layerCount = 1;

	//copyRegion.bufferOffset                    = 0;
	//copyRegion.bufferRowLength                 = 0;
	//copyRegion.bufferImageHeight               = 0;
	//copyRegion.imageOffset                     = { 0, 0, 0 };
	//copyRegion.imageSubresource.baseArrayLayer = 0;
	//copyRegion.imageSubresource.mipLevel       = 0;

	vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

	this->commandBufferEnd(commandBuffer);

	return 0;
}

int VulkanContext::copyImage(VkImage image, VkFormat imageFormat, VkImageLayout oldLayout, VkImageLayout newLayout)
{
	VkCommandBuffer commandBuffer = this->commandBufferBegin();

	if (commandBuffer == nullptr)
		return -1;

	VkImageMemoryBarrier imageMemBarrier        = {};
	VkPipelineStageFlags pipelineStageSrcFlags  = 0;
	VkPipelineStageFlags pipelineStageDestFlags = 0;

	imageMemBarrier.sType                       = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imageMemBarrier.oldLayout                   = oldLayout;
	imageMemBarrier.newLayout                   = newLayout;
	imageMemBarrier.srcQueueFamilyIndex         = VK_QUEUE_FAMILY_IGNORED;
	imageMemBarrier.dstQueueFamilyIndex         = VK_QUEUE_FAMILY_IGNORED;
	imageMemBarrier.image                       = image;
	imageMemBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageMemBarrier.subresourceRange.levelCount = 1;
	imageMemBarrier.subresourceRange.layerCount = 1;
	//imageMemBarrier.srcAccessMask                   = 0;
	//imageMemBarrier.dstAccessMask                   = 0;
	//imageMemBarrier.subresourceRange.baseMipLevel   = 0;
	//imageMemBarrier.subresourceRange.baseArrayLayer = 0;

	// TO DESTINATION
	if ((oldLayout == VK_IMAGE_LAYOUT_UNDEFINED) && (newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL))
	{
		imageMemBarrier.srcAccessMask = 0;
		imageMemBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		pipelineStageSrcFlags  = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		pipelineStageDestFlags = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	// TO SHADER
	else if ((oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) && (newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL))
	{
		imageMemBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		imageMemBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		pipelineStageSrcFlags  = VK_PIPELINE_STAGE_TRANSFER_BIT;
		pipelineStageDestFlags = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	// UNKNOWN TRANSITION
	else
	{
		return -2;
	}

	//vkCmdPipelineBarrier(commandBuffer, 0, 0, 0, 0, nullptr, 0, nullptr, 1, &imageMemBarrier);
	vkCmdPipelineBarrier(commandBuffer, pipelineStageSrcFlags, pipelineStageDestFlags, 0, 0, nullptr, 0, nullptr, 1, &imageMemBarrier);

	this->commandBufferEnd(commandBuffer);

	return 0;
}

int VulkanContext::createBuffer(
	VkDeviceSize          size,
	VkBufferUsageFlags    useFlags,
	VkMemoryPropertyFlags memoryFlags,
	VkBuffer*             buffer,
	VkDeviceMemory*       bufferMemory)
{
	VkBufferCreateInfo    bufferInfo    = {};
	VkMemoryAllocateInfo  bufferMemInfo = {};
	VkMemoryRequirements  bufferMemReq  = {};

	// VERTEX BUFFER
	bufferInfo.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	//vertexBufferInfo.flags       = 0;
	bufferInfo.size        = size;
	bufferInfo.usage       = useFlags;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(this->deviceContext, &bufferInfo, nullptr, buffer) != VK_SUCCESS)
		return -1;

	vkGetBufferMemoryRequirements(this->deviceContext, *buffer, &bufferMemReq);

	bufferMemInfo.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	bufferMemInfo.allocationSize  = bufferMemReq.size;
	bufferMemInfo.memoryTypeIndex = this->getMemoryType(bufferMemReq.memoryTypeBits, memoryFlags);

	if (bufferMemInfo.memoryTypeIndex < 0)
		return -2;

	if (vkAllocateMemory(this->deviceContext, &bufferMemInfo, nullptr, bufferMemory) != VK_SUCCESS)
		return -3;

	// PS! Offset must be divisible by memRequirements.alignment
	vkBindBufferMemory(this->deviceContext, *buffer, *bufferMemory, 0);

	return 0;
}

int VulkanContext::createImage(
	uint32_t              width,
	uint32_t              height,
	VkFormat              format,
	VkImageTiling         tiling,
	VkImageUsageFlags     useFlags,
	VkMemoryPropertyFlags memoryFlags,
	VkImage*              textureImage,
	VkDeviceMemory*       textureImageMemory
)
{
	VkMemoryAllocateInfo  imageAllocInfo = {};
	VkImageCreateInfo     imageInfo      = {};
	VkMemoryRequirements  imageMemReq    = {};

	imageInfo.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.arrayLayers   = 1;
	imageInfo.extent.width  = width;
	imageInfo.extent.height = height;
	imageInfo.extent.depth  = 1;
	imageInfo.format        = format;
	imageInfo.imageType     = VK_IMAGE_TYPE_2D;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.mipLevels     = 1;
	imageInfo.samples       = VK_SAMPLE_COUNT_1_BIT;	// MULTISAMPLING
	imageInfo.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.tiling        = tiling;
	imageInfo.usage         = useFlags;
	//imageInfo.flags         = 0;
	//imageInfo.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
	//imageInfo.tiling        = VK_IMAGE_TILING_LINEAR;

	if (vkCreateImage(this->deviceContext, &imageInfo, nullptr, textureImage) != VK_SUCCESS)
		return -1;

	// IMAGE MEMORY
	vkGetImageMemoryRequirements(this->deviceContext, *textureImage, &imageMemReq);

	imageAllocInfo.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	imageAllocInfo.allocationSize  = imageMemReq.size;
	imageAllocInfo.memoryTypeIndex = this->getMemoryType(imageMemReq.memoryTypeBits, memoryFlags);

	if (imageAllocInfo.memoryTypeIndex < 0)
		return -2;

	if (vkAllocateMemory(this->deviceContext, &imageAllocInfo, nullptr, textureImageMemory) != VK_SUCCESS)
		return -3;

	vkBindImageMemory(this->deviceContext, *textureImage, *textureImageMemory, 0);

	return 0;
}

int VulkanContext::CreateIndexBuffer(const std::vector<uint32_t> &indices, VkBuffer* indexBuffer, VkDeviceMemory* indexBufferMemory)
{
	VkBuffer              stagingBuffer         = nullptr;
	VkDeviceMemory        stagingBufferMemory   = nullptr;
	VkMemoryPropertyFlags stagingBufferMemFlags = (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	VkBufferUsageFlags    stagingBufferUseFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	void*                 indexBufferMemData   = nullptr;
	VkMemoryPropertyFlags indexBufferMemFlags  = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	VkBufferUsageFlags    indexBufferUseFlags  = (VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
	size_t                indexBufferSize      = (indices.size() * sizeof(uint32_t));

	// STAGING BUFFER
	if (this->createBuffer(indexBufferSize, stagingBufferUseFlags, stagingBufferMemFlags, &stagingBuffer, &stagingBufferMemory) < 0)
		return -1;

	// COPY DATA TO STAGE BUFFER
	vkMapMemory(this->deviceContext, stagingBufferMemory, 0, indexBufferSize, 0, &indexBufferMemData);
		memcpy(indexBufferMemData, indices.data(), (size_t)indexBufferSize);
	vkUnmapMemory(this->deviceContext, stagingBufferMemory);

	// VERTEX BUFFER
	if (this->createBuffer(indexBufferSize, indexBufferUseFlags, indexBufferMemFlags, indexBuffer, indexBufferMemory) < 0)
		return -2;

	// COPY DATA FROM STAGING TO VERTEX BUFFER (DEVICE LOCAL)
	this->copyBuffer(stagingBuffer, *indexBuffer, indexBufferSize);

	vkFreeMemory(this->deviceContext,    stagingBufferMemory, nullptr);
	vkDestroyBuffer(this->deviceContext, stagingBuffer,       nullptr);

	return 0;
}

int VulkanContext::CreateShaderModule(const wxString &shaderFile, const wxString &stage, VkShaderModule* shaderModule)
{
	#if defined _DEBUG
		wxArrayString output;
		wxString      glsValidator = wxString(std::getenv("VK_SDK_PATH")).append("/Bin/glslangValidator -V ");
		wxString      command      = wxString(glsValidator + shaderFile + " -S " + stage + " -o " + shaderFile + ".spv");
		long          result       = wxExecute(command, output, wxEXEC_SYNC);

		for (auto line : output)
			wxLogDebug("%s\n", line.c_str().AsChar());

		if (result != 0)
			return -1;
	#endif

	std::vector<uint8_t>     byteCode         = Utils::LoadDataFile(wxString(shaderFile + ".spv"));
	VkShaderModuleCreateInfo shaderModuleInfo = {};

	shaderModuleInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shaderModuleInfo.codeSize = byteCode.size();
	shaderModuleInfo.pCode    = reinterpret_cast<const uint32_t*>(byteCode.data());

	if (vkCreateShaderModule(this->deviceContext, &shaderModuleInfo, nullptr, shaderModule) != VK_SUCCESS)
		return -1;

	return 0;
}

int VulkanContext::CreateTexture(
	uint32_t        width,
	uint32_t        height,
	uint8_t*        imagePixels,
	VkImage*        textureImage,
	VkDeviceMemory* textureImageMemory
)
{
	VkBufferUsageFlags    bufferUseFlags      = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	VkMemoryPropertyFlags bufferMemFlags      = (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	//VkFormat              imageFormat         = (hasAlpha ? VK_FORMAT_R8G8B8A8_UNORM : VK_FORMAT_R8G8B8_UNORM);
	VkFormat              imageFormat         = VK_FORMAT_R8G8B8A8_UNORM;
	void*                 imageMemData        = nullptr;
	VkMemoryPropertyFlags imageUseFlags       = (VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
	VkMemoryPropertyFlags imageMemFlags       = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	//VkDeviceSize          imageSize           = (width * height * (hasAlpha ? 4 : 3));
	VkDeviceSize          imageSize           = (width * height * 4);
	VkBuffer              stagingBuffer       = nullptr;
	VkDeviceMemory        stagingBufferMemory = nullptr;

	// STAGING BUFFER
	if (this->createBuffer(imageSize, bufferUseFlags, bufferMemFlags, &stagingBuffer, &stagingBufferMemory) < 0)
		return -1;

	// COPY IMAGE DATA TO STAGE BUFFER
	vkMapMemory(this->deviceContext, stagingBufferMemory, 0, imageSize, 0, &imageMemData);
		memcpy(imageMemData, imagePixels, (size_t)imageSize);
	vkUnmapMemory(this->deviceContext, stagingBufferMemory);

	// CREATE IMAGE
	if (this->createImage(width, height, imageFormat, VK_IMAGE_TILING_OPTIMAL, imageUseFlags, imageMemFlags, textureImage, textureImageMemory) < 0)
		return -2;

	// TRANSITION IMAGE LAYOUT TO DESTINATION
	if (this->copyImage(*textureImage, imageFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) < 0)
		return -3;

	// COPY DATA FROM STAGING BUFFER TO IMAGE (DEVICE LOCAL)
	if (this->copyBufferToImage(stagingBuffer, *textureImage, width, height) < 0)
		return -4;

	// TRANSITION IMAGE LAYOUT TO SHADER
	if (this->copyImage(*textureImage, imageFormat, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) < 0)
		return -5;

	vkFreeMemory(this->deviceContext,    stagingBufferMemory, nullptr);
	vkDestroyBuffer(this->deviceContext, stagingBuffer,       nullptr);

	return 0;
}

int VulkanContext::CreateUniformBuffers(VkBuffer* uniformBuffer, VkDeviceMemory* uniformBufferMemory)
{
	//VkDeviceSize          bufferSize     = (sizeof(GLMatrixBuffer) + sizeof(GLDefaultBuffer));
	VkBufferUsageFlags    bufferUseFlags = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	VkMemoryPropertyFlags bufferMemFlags = (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	// TODO: VULKAN
	//for (int i = 0; i < NR_OF_SHADERS; i++)
	if (this->createBuffer(sizeof(GLMatrixBuffer), bufferUseFlags, bufferMemFlags, &uniformBuffer[UNIFORM_BUFFER_MATRIX], &uniformBufferMemory[UNIFORM_BUFFER_MATRIX]) < 0)
		return 1;

	if (this->createBuffer(sizeof(GLDefaultBuffer), bufferUseFlags, bufferMemFlags, &uniformBuffer[UNIFORM_BUFFER_DEFAULT], &uniformBufferMemory[UNIFORM_BUFFER_DEFAULT]) < 0)
		return -2;

	return 0;
}

int VulkanContext::CreateVertexBuffer(
	const std::vector<float> &vertices,
	const std::vector<float> &normals,
	const std::vector<float> &texCoords,
	VkPipeline*              pipelines,
	VkBuffer*                vertexBuffer,
	VkDeviceMemory*          vertexBufferMemory
)
{
	VkBuffer              stagingBuffer         = nullptr;
	VkDeviceMemory        stagingBufferMemory   = nullptr;
	VkMemoryPropertyFlags stagingBufferMemFlags = (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	VkBufferUsageFlags    stagingBufferUseFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	std::vector<float>    vertexBufferData      = Utils::ToVertexBufferData(vertices, normals, texCoords);
	void*                 vertexBufferMemData   = nullptr;
	VkMemoryPropertyFlags vertexBufferMemFlags  = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	VkBufferUsageFlags    vertexBufferUseFlags  = (VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
	size_t                vertexBufferSize      = (vertexBufferData.size() * sizeof(float));

	// STAGING BUFFER
	if (this->createBuffer(vertexBufferSize, stagingBufferUseFlags, stagingBufferMemFlags, &stagingBuffer, &stagingBufferMemory) < 0)
		return -1;

	// COPY DATA TO STAGE BUFFER
	vkMapMemory(this->deviceContext, stagingBufferMemory, 0, vertexBufferSize, 0, &vertexBufferMemData);
		memcpy(vertexBufferMemData, vertexBufferData.data(), (size_t)vertexBufferSize);
	vkUnmapMemory(this->deviceContext, stagingBufferMemory);

	// VERTEX BUFFER
	if (this->createBuffer(vertexBufferSize, vertexBufferUseFlags, vertexBufferMemFlags, vertexBuffer, vertexBufferMemory) < 0)
		return -2;

	// COPY DATA FROM STAGING TO VERTEX BUFFER (DEVICE LOCAL)
	if (this->copyBuffer(stagingBuffer, *vertexBuffer, vertexBufferSize) < 0)
		return -3;

	vkFreeMemory(this->deviceContext,    stagingBufferMemory, nullptr);
	vkDestroyBuffer(this->deviceContext, stagingBuffer,       nullptr);

	// SHADER ATTRIBS
	VkVertexInputBindingDescription   attribsBindingDesc         = {};
	VkVertexInputAttributeDescription attribsDesc[NR_OF_ATTRIBS] = {};
	uint32_t                          stride                     = 0;

	// NORMALS
	if (!normals.empty()) {
		//attribsCount++;
		attribsDesc[ATTRIB_NORMAL].location = ATTRIB_NORMAL;
		//attribsDesc[ATTRIB_NORMAL].binding  = 0;
		attribsDesc[ATTRIB_NORMAL].format   = VK_FORMAT_R32G32B32_SFLOAT;
		attribsDesc[ATTRIB_NORMAL].offset   = 0;

		stride += (3 * sizeof(float));
	}

	// POSITIONS
	if (!vertices.empty()) {
		//attribsCount++;
		attribsDesc[ATTRIB_POSITION].location = ATTRIB_POSITION;
		//attribsDesc[ATTRIB_POSITION].binding  = 0;
		attribsDesc[ATTRIB_POSITION].offset   = (3 * sizeof(float));
		attribsDesc[ATTRIB_POSITION].format   = VK_FORMAT_R32G32B32_SFLOAT;

		stride += (3 * sizeof(float));
	}

	// TEXTURE COORDINATES
	if (!texCoords.empty()) {
		//attribsCount++;
		attribsDesc[ATTRIB_TEXCOORDS].location = ATTRIB_TEXCOORDS;
		//attribsDesc[ATTRIB_TEXCOORDS].binding  = 0;
		attribsDesc[ATTRIB_TEXCOORDS].format   = VK_FORMAT_R32G32_SFLOAT;
		attribsDesc[ATTRIB_TEXCOORDS].offset   = (6 * sizeof(float));

		stride += (2 * sizeof(float));
	}

	//attribsBindingDesc.binding   = 0;
	attribsBindingDesc.stride    = stride;
	attribsBindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	//attribsBindingDesc.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;

	// RENDER PIPELINES
	// TODO: VULKAN
	//for (int i = 0; i < NR_OF_SHADERS; i++)
	for (int i = SHADER_ID_DEFAULT; i < (SHADER_ID_DEFAULT + 1); i++) {
		if (!this->initPipeline(ShaderManager::Programs[i], pipelines[i], attribsBindingDesc, attribsDesc))
			return -4;
	}

	return 0;
}

VKAPI_ATTR VkBool32 VKAPI_CALL VulkanContext::debugLog(VkDebugReportFlagsEXT f, VkDebugReportObjectTypeEXT ot, uint64_t o, size_t l, int32_t c, const char* lp, const char* m, void* ud)
{
	wxLogDebug("%s\n", m);
	return VK_FALSE;
}

void VulkanContext::DestroyBuffer(VkBuffer* buffer, VkDeviceMemory* bufferMemory)
{
	if (*bufferMemory != nullptr) {
		vkFreeMemory(this->deviceContext, *bufferMemory, nullptr);
		*bufferMemory = nullptr;
	}

	if (*buffer != nullptr) {
		vkDestroyBuffer(this->deviceContext, *buffer, nullptr);
		*buffer = nullptr;
	}
}

void VulkanContext::DestroyPipeline(VkPipeline* pipeline)
{
	if (*pipeline != nullptr) {
		vkDestroyPipeline(this->deviceContext, *pipeline, nullptr);
		*pipeline = nullptr;
	}
}

void VulkanContext::DestroyShaderModule(VkShaderModule* shaderModule)
{
	if (*shaderModule != nullptr) {
		vkDestroyShaderModule(this->deviceContext, *shaderModule, nullptr);
		*shaderModule = nullptr;
	}
}

void VulkanContext::DestroyTexture(VkImage* image, VkDeviceMemory* imageMemory)
{
	if (*imageMemory != nullptr) {
		vkFreeMemory(this->deviceContext, *imageMemory, nullptr);
		*imageMemory = nullptr;
	}

	if (*image != nullptr) {
		vkDestroyImage(this->deviceContext, *image, nullptr);
		*image = nullptr;
	}
}

bool VulkanContext::deviceSupportsExtensions(VkPhysicalDevice device, const std::vector<const char*> &extensions)
{
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> deviceExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, deviceExtensions.data());

	for (auto ext : extensions)
	{
		bool extFound = false;

		for (auto deviceExt : deviceExtensions) {
			if (strcmp(ext, deviceExt.extensionName) == 0) {
				extFound = true;
				break;
			}
		}

		if (!extFound)
			return false;
	}

	return true;
}

int VulkanContext::Draw(Mesh* mesh, ShaderProgram* shaderProgram, bool enableClipping, const glm::vec3 &clipMax, const glm::vec3 &clipMin)
{
	if ((RenderEngine::Camera == nullptr) || (mesh == nullptr) || (shaderProgram == nullptr))
		return -1;

	Buffer*  indexBuffer  = mesh->IndexBuffer();
	Buffer*  vertexBuffer = mesh->VertexBuffer();
	//ID3DBlob*  fragmentShader = shaderProgram->FS();
	//ID3DBlob*  vertexShader   = shaderProgram->VS();
	ShaderID shaderID     = shaderProgram->ID();

	//if ((indexBuffer == nullptr) || (vertexBuffer == nullptr) || (fragmentShader == nullptr) || (vertexShader == nullptr))
	if ((indexBuffer == nullptr) && (vertexBuffer == nullptr))
		return -2;

	VkPipeline   pipeline              = vertexBuffer->Pipeline(shaderID);
	VkBuffer     vertexBuffers[]       = { vertexBuffer->VertexBuffer() };
	VkDeviceSize vertexBufferOffsets[] = { 0 };

	if ((pipeline == nullptr) || (vertexBuffers[0] == nullptr))
		return -3;

	// UPDATE UNIFORM VALUES
	if (shaderProgram->UpdateUniformsVK(this->deviceContext, this->uniformSet, mesh, enableClipping, clipMax, clipMin) != 0)
		return -4;

	// BIND SHADER TO PIPELINE
	vkCmdBindPipeline(this->commandBuffers[this->imageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

	// BIND INDEX AND VERTEX BUFFERS
	vkCmdBindIndexBuffer(this->commandBuffers[this->imageIndex], indexBuffer->IndexBuffer(), 0, VK_INDEX_TYPE_UINT32);
	vkCmdBindVertexBuffers(this->commandBuffers[this->imageIndex], 0, 1, vertexBuffers, vertexBufferOffsets);
	
	// TODO: SLOW
	// BIND UNIFORMS
	vkCmdBindDescriptorSets(
		this->commandBuffers[this->imageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS,
		this->pipelineLayout, 0, 1, &this->uniformSet, 0, nullptr
	);

	// DRAW
	//vkCmdDraw(this->commandBuffers[this->imageIndex], 3, 1, 0, 0);
	//vkCmdDraw(this->commandBuffers[this->imageIndex], mesh->NrOfVertices(), 1, 0, 0);
	vkCmdDrawIndexed(this->commandBuffers[this->imageIndex], mesh->NrOfIndices(), 1, 0, 0, 0);

	return 0;
}

wxString VulkanContext::getApiVersion(VkPhysicalDevice device)
{
	if (device == nullptr)
		return wxT("");

	VkPhysicalDeviceProperties properties = {};
	vkGetPhysicalDeviceProperties(device, &properties);

	char apiVersion[BUFFER_SIZE] = {};

	snprintf(
		apiVersion, BUFFER_SIZE, "Vulkan %u.%u.%u",
		VK_VERSION_MAJOR(properties.apiVersion),
		VK_VERSION_MINOR(properties.apiVersion),
		VK_VERSION_PATCH(properties.apiVersion)
	);

	return wxString(apiVersion);
}

VkPhysicalDevice VulkanContext::getDevice(const std::vector<const char*> &extensions)
{
	if (this->instance == nullptr)
		return nullptr;

	uint32_t deviceCount = 0;
	VkResult result      = vkEnumeratePhysicalDevices(this->instance, &deviceCount, nullptr);

	if ((result != VK_SUCCESS) || (deviceCount < 1))
		return nullptr;

	std::vector<VkPhysicalDevice> devices(deviceCount);

	result = vkEnumeratePhysicalDevices(this->instance, &deviceCount, devices.data());

	if ((result != VK_SUCCESS) || devices.empty())
		return nullptr;

	VkPhysicalDevice physicalDevice = nullptr;

	//for (int i = 0; i == 0; i++)
	//for (int i = 1; i == 1; i++)
	for (auto device : devices)
	{
		//VkPhysicalDevice device = devices[i];


		this->swapChainSupport = this->getDeviceSwapChainSupport(device);

		if (this->swapChainSupport->PresentModes.empty() || this->swapChainSupport->SurfaceFormats.empty())
			continue;

		this->queues = this->getDeviceQueueSupport(device);

		if ((this->queues[VULKAN_QUEUE_GRAPHICS]->Index < 0) || (this->queues[VULKAN_QUEUE_PRESENTATION]->Index < 0))
			continue;

		if (!this->deviceSupportsExtensions(device, extensions))
			continue;

		physicalDevice = device;

		break;
	}

	return physicalDevice;
}

wxString VulkanContext::getDeviceName(VkPhysicalDevice device)
{
	if (device == nullptr)
		return wxT("");

	VkPhysicalDeviceProperties properties = {};
	vkGetPhysicalDeviceProperties(device, &properties);

	wxString vendor;

	switch (properties.vendorID) {
		case 0x1002: vendor = "AMD";    break;
		case 0x10DE: vendor = "NVIDIA"; break;
		case 0x8086: vendor = "Intel";  break;
		default:     vendor = wxT("");  break;
	}

	char adapterName[BUFFER_SIZE] = {};

	snprintf(
		adapterName, BUFFER_SIZE, "%s %s %u.%u.%u",
		vendor.data().AsChar(),
		properties.deviceName,
		VK_VERSION_MAJOR(properties.driverVersion),
		VK_VERSION_MINOR(properties.driverVersion),
		VK_VERSION_PATCH(properties.driverVersion)
	);

	return wxString(adapterName);
}

std::map<VulkanQueueType, VulkanQueue*> VulkanContext::getDeviceQueueSupport(VkPhysicalDevice device)
{
	std::map<VulkanQueueType, VulkanQueue*> queueSupport;

	uint32_t queueCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueCount, nullptr);

	std::vector<VkQueueFamilyProperties> queues(queueCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueCount, queues.data());

	for (int32_t i = 0; i < (int32_t)queueCount; i++)
	{
		if (queues[i].queueCount == 0)
			continue;

		if ((queues[0].queueFlags & VK_QUEUE_GRAPHICS_BIT) && (queueSupport[VULKAN_QUEUE_GRAPHICS] == nullptr))
			queueSupport[VULKAN_QUEUE_GRAPHICS] = new VulkanQueue(i);

		VkBool32 supportsPresentation = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, this->surface, &supportsPresentation);

		if (supportsPresentation && (queueSupport[VULKAN_QUEUE_PRESENTATION] == nullptr))
			queueSupport[VULKAN_QUEUE_PRESENTATION] = new VulkanQueue(i);

		if ((queueSupport[VULKAN_QUEUE_PRESENTATION] != nullptr) && (queueSupport[VULKAN_QUEUE_GRAPHICS] != nullptr))
			break;
	}

	return queueSupport;
}

VulkanSwapChainSupport* VulkanContext::getDeviceSwapChainSupport(VkPhysicalDevice device)
{
	VulkanSwapChainSupport* swapChainSupport = new VulkanSwapChainSupport();

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, this->surface, &swapChainSupport->SurfaceCapabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, this->surface, &formatCount, nullptr);

	if (formatCount > 0) {
		swapChainSupport->SurfaceFormats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, swapChainSupport->SurfaceFormats.data());
	}

	uint32_t presentCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentCount, nullptr);

	if (presentCount > 0) {
		swapChainSupport->PresentModes.resize(presentCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentCount, swapChainSupport->PresentModes.data());
	}

	return swapChainSupport;
}

uint32_t VulkanContext::getMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memoryProperties;
	vkGetPhysicalDeviceMemoryProperties(this->device, &memoryProperties);

	for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
		if ((typeFilter & (1 << i)) && ((memoryProperties.memoryTypes[i].propertyFlags & properties) == properties))
			return i;
	}

	return -1;
}

VkPresentModeKHR VulkanContext::getPresentationMode(const std::vector<VkPresentModeKHR> &presentationModes)
{
	std::vector<VkPresentModeKHR> prioritizedModes = {
		VK_PRESENT_MODE_MAILBOX_KHR,
		VK_PRESENT_MODE_IMMEDIATE_KHR,
		VK_PRESENT_MODE_FIFO_KHR,
		VK_PRESENT_MODE_FIFO_RELAXED_KHR
	};

	for (auto priMode : prioritizedModes) {
		if (std::find(presentationModes.begin(), presentationModes.end(), priMode) != presentationModes.end())
			return priMode;
	}

	return (!presentationModes.empty() ? presentationModes[0] : VK_PRESENT_MODE_FIFO_KHR);
}

VkSurfaceFormatKHR* VulkanContext::getSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &sufaceFormats)
{
	VkSurfaceFormatKHR* surfaceFormat = new VkSurfaceFormatKHR();

	surfaceFormat->format     = VK_FORMAT_B8G8R8A8_UNORM;
	surfaceFormat->colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

	for (auto format : sufaceFormats) {
		if ((format.format == VK_FORMAT_UNDEFINED) || ((format.format == surfaceFormat->format) && (format.colorSpace == surfaceFormat->colorSpace)))
			return surfaceFormat;
	}

	if (!sufaceFormats.empty()) {
		surfaceFormat->format     = sufaceFormats[0].format;
		surfaceFormat->colorSpace = sufaceFormats[0].colorSpace;
	} else {
		_DELETEP(surfaceFormat);
	}

	return surfaceFormat;
}

VkExtent2D* VulkanContext::getSurfaceSize(const VkSurfaceCapabilitiesKHR &capabilities)
{
	// Use the currently selected size if already detected
	if (capabilities.currentExtent.width != UINT32_MAX)
		return new VkExtent2D(capabilities.currentExtent);

	// Otherwise try to match the size as much as possible to the canvas size
	VkExtent2D* size = new VkExtent2D();

	uint32_t width     = (uint32_t)RenderEngine::Canvas.Size.GetWidth();
	uint32_t height    = (uint32_t)RenderEngine::Canvas.Size.GetHeight();
	uint32_t minWidth  = capabilities.minImageExtent.width;
	uint32_t minHeight = capabilities.minImageExtent.height;
	uint32_t maxWidth  = capabilities.maxImageExtent.width;
	uint32_t maxHeight = capabilities.maxImageExtent.height;

	size->width  = std::max(minWidth,  std::min(maxWidth,  width));
	size->height = std::max(minHeight, std::min(maxHeight, height));

	return size;
}

VkPipelineColorBlendStateCreateInfo* VulkanContext::initColorBlending()
{

	VkPipelineColorBlendAttachmentState* colorBlendAttachment = new VkPipelineColorBlendAttachmentState();
	VkPipelineColorBlendStateCreateInfo* colorBlending        = new VkPipelineColorBlendStateCreateInfo();

	colorBlendAttachment->blendEnable         = VK_TRUE;
	colorBlendAttachment->colorWriteMask      = (VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT);
	colorBlendAttachment->srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	//colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment->dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	//colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
	//colorBlendAttachment.colorBlendOp        = VK_BLEND_OP_ADD;
	colorBlendAttachment->srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	//colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	//colorBlendAttachment.alphaBlendOp        = VK_BLEND_OP_ADD;

	colorBlending->sType           = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	//colorBlending.logicOpEnable     = VK_FALSE;
	colorBlending->logicOp         = VK_LOGIC_OP_COPY;
	colorBlending->attachmentCount = 1;
	colorBlending->pAttachments    = colorBlendAttachment;
	//colorBlending.blendConstants[0] = 0.0f;
	//colorBlending.blendConstants[1] = 0.0f;
	//colorBlending.blendConstants[2] = 0.0f;
	//colorBlending.blendConstants[3] = 0.0f;

	return colorBlending;
}

std::vector<VkCommandBuffer> VulkanContext::initCommandBuffers(uint32_t bufferCount)
{
	// Command buffer allocation
	VkCommandBufferAllocateInfo  allocateInfo   = {};
	//std::vector<VkCommandBuffer> commandBuffers = std::vector<VkCommandBuffer>(this->frameBuffers.size());
	std::vector<VkCommandBuffer> commandBuffers = std::vector<VkCommandBuffer>(bufferCount);

	allocateInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocateInfo.commandBufferCount = (uint32_t)commandBuffers.size();
	allocateInfo.commandPool        = this->commandPool;
	//allocateInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

	if (vkAllocateCommandBuffers(this->deviceContext, &allocateInfo, commandBuffers.data()) != VK_SUCCESS)
		commandBuffers.clear();

	return commandBuffers;
}

VkCommandPool VulkanContext::initCommandPool()
{
	VkCommandPoolCreateInfo commandPoolInfo = {};
	VkCommandPool           commandPool     = nullptr;

	commandPoolInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolInfo.queueFamilyIndex = this->queues[VULKAN_QUEUE_GRAPHICS]->Index;
	commandPoolInfo.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	if (vkCreateCommandPool(this->deviceContext, &commandPoolInfo, nullptr, &commandPool) != VK_SUCCESS)
		return nullptr;

	return commandPool;
}

VkPipelineDepthStencilStateCreateInfo* VulkanContext::initDepthStencilBuffer()
{
	VkPipelineDepthStencilStateCreateInfo* depthStencilBuffer = new VkPipelineDepthStencilStateCreateInfo();

	//

	return depthStencilBuffer;
}

VkDevice VulkanContext::initDeviceContext()
{
	if ((this->instance == nullptr) || (this->surface == nullptr))
		return nullptr;

	std::vector<const char*> extensions = { "VK_KHR_swapchain" };

	this->device = this->getDevice(extensions);

	if (this->device == nullptr)
		return nullptr;

	std::vector<VkDeviceQueueCreateInfo> queueInfos;
	float                                queuePriority[] = { 1.0f };

	// Get the unique queue indices for graphics and presentation (they may be the same)
	std::set<uint32_t> uniqueQueueIndices = {
		(uint32_t)this->queues[VULKAN_QUEUE_PRESENTATION]->Index, (uint32_t)this->queues[VULKAN_QUEUE_GRAPHICS]->Index
	};

	for (auto index : uniqueQueueIndices)
	{
		VkDeviceQueueCreateInfo queueInfo = {};

		queueInfo.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueInfo.queueCount       = 1;
		queueInfo.queueFamilyIndex = index;
		queueInfo.pQueuePriorities = queuePriority;

		queueInfos.push_back(queueInfo);
	}

	// Create a device context (logical device) from a physical device
	VkDevice                 deviceContext = nullptr;
	VkDeviceCreateInfo       deviceInfo    = {};
	VkPhysicalDeviceFeatures features      = {};

	deviceInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceInfo.queueCreateInfoCount    = queueInfos.size();
	deviceInfo.pQueueCreateInfos       = queueInfos.data();
	deviceInfo.pEnabledFeatures        = &features;
	deviceInfo.ppEnabledExtensionNames = extensions.data();
	deviceInfo.enabledExtensionCount   = extensions.size();
	//deviceInfo.enabledLayerCount     = 0;

	if (vkCreateDevice(this->device, &deviceInfo, nullptr, &deviceContext) != VK_SUCCESS)
		return nullptr;

	// Create the graphics and presentation queues
	vkGetDeviceQueue(deviceContext, this->queues[VULKAN_QUEUE_GRAPHICS]->Index,     0, &this->queues[VULKAN_QUEUE_GRAPHICS]->Queue);
	vkGetDeviceQueue(deviceContext, this->queues[VULKAN_QUEUE_PRESENTATION]->Index, 0, &this->queues[VULKAN_QUEUE_PRESENTATION]->Queue);

	return deviceContext;
}

std::vector<VkFramebuffer> VulkanContext::initFramebuffers()
{
	std::vector<VkFramebuffer> frameBuffers = std::vector<VkFramebuffer>(this->swapChain->ImageViews.size());

	for (int i = 0; i < (int)this->swapChain->ImageViews.size(); i++)
	{
		VkImageView             attachments[]   = { this->swapChain->ImageViews[i] };
		VkFramebufferCreateInfo framebufferInfo = {};

		framebufferInfo.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments    = attachments;
		framebufferInfo.width           = this->swapChain->Size->width;
		framebufferInfo.height          = this->swapChain->Size->height;
		framebufferInfo.layers          = 1;
		framebufferInfo.renderPass      = this->renderPass;

		if (vkCreateFramebuffer(this->deviceContext, &framebufferInfo, nullptr, &frameBuffers[i]) != VK_SUCCESS)
			continue;

	}

	return frameBuffers;
}
VkInstance VulkanContext::initInstance()
{
	VkApplicationInfo        appInfo      = {};
	std::vector<const char*> extensions   = { "VK_KHR_surface" };
	std::vector<const char*> layers       = {};
	VkInstanceCreateInfo     instanceInfo = {};
	VkInstance               instance     = nullptr;

	#if defined _WINDOWS
		extensions.push_back("VK_KHR_win32_surface");
	#endif

	#if defined _DEBUG
		extensions.push_back("VK_EXT_debug_report");
		layers.push_back("VK_LAYER_LUNARG_standard_validation");
	#endif

	appInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.apiVersion         = VK_API_VERSION_1_0;
	appInfo.pApplicationName   = Utils::APP_NAME;
	appInfo.applicationVersion = VK_MAKE_VERSION(Utils::APP_VERSION_MAJOR, Utils::APP_VERSION_MINOR, Utils::APP_VERSION_PATCH);

	instanceInfo.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceInfo.pApplicationInfo        = &appInfo;
	instanceInfo.enabledExtensionCount   = extensions.size();
	instanceInfo.ppEnabledExtensionNames = extensions.data();
	instanceInfo.enabledLayerCount       = layers.size();
	instanceInfo.ppEnabledLayerNames     = (!layers.empty() ? layers.data() : nullptr);

	if (vkCreateInstance(&instanceInfo, nullptr, &instance) != VK_SUCCESS)
		return nullptr;

	#if defined _DEBUG
		VkDebugReportCallbackCreateInfoEXT debugInfo = {};

		debugInfo.sType       = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
		debugInfo.flags       = (VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT);
		debugInfo.pfnCallback = VulkanContext::debugLog;

		// Try to load the external method
		PFN_vkCreateDebugReportCallbackEXT createDebugReportCallback = VK_NULL_HANDLE;
		createDebugReportCallback = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");

		if (createDebugReportCallback == VK_NULL_HANDLE)
			return nullptr;

		if (createDebugReportCallback(instance, &debugInfo, nullptr, &this->debugCallback) != VK_SUCCESS)
			return nullptr;
	#endif

	return instance;
}

VkPipelineMultisampleStateCreateInfo* VulkanContext::initMultisampling()
{
	VkPipelineMultisampleStateCreateInfo* multisampling = new VkPipelineMultisampleStateCreateInfo();

	multisampling->sType                = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	//multisampling.sampleShadingEnable   = VK_FALSE;
	multisampling->rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling->minSampleShading     = 1.0f;
	//multisampling.pSampleMask           = nullptr;
	//multisampling.alphaToCoverageEnable = VK_FALSE;
	//multisampling.alphaToOneEnable      = VK_FALSE;

	return multisampling;
}

//VkPipeline VulkanContext::initPipeline(VkShaderModule vulkanVS, VkShaderModule vulkanFS)
bool VulkanContext::initPipeline(ShaderProgram* shaderProgram, VkPipeline &pipeline, VkVertexInputBindingDescription attribsBindingDesc, VkVertexInputAttributeDescription attribsDesc[NR_OF_ATTRIBS])
{
	if ((this->deviceContext == nullptr) || (shaderProgram == nullptr))
		return false;

	VkPipelineShaderStageCreateInfo fsStageInfo = {};
	VkPipelineShaderStageCreateInfo vsStageInfo = {};

	vsStageInfo.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vsStageInfo.stage  = VK_SHADER_STAGE_VERTEX_BIT;
	vsStageInfo.module = shaderProgram->VulkanVS();
	vsStageInfo.pName  = "main";
	//vsStageInfo.pSpecializationInfo = nullptr;

	fsStageInfo.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fsStageInfo.stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
	fsStageInfo.module = shaderProgram->VulkanFS();
	fsStageInfo.pName  = "main";
	//fsStageInfo.pSpecializationInfo = nullptr;

	VkPipelineShaderStageCreateInfo        shaderStages[] = { vsStageInfo, fsStageInfo };
	VkPipelineInputAssemblyStateCreateInfo inputAssembly  = {};
	VkPipelineVertexInputStateCreateInfo   vertexInput    = {};
	VkGraphicsPipelineCreateInfo           pipelineInfo   = {};
	//VkPipeline                             pipeline       = nullptr;

	vertexInput.sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInput.vertexAttributeDescriptionCount = NR_OF_ATTRIBS;
	vertexInput.pVertexAttributeDescriptions    = attribsDesc;
	vertexInput.vertexBindingDescriptionCount   = 1;
	vertexInput.pVertexBindingDescriptions      = &attribsBindingDesc;

	inputAssembly.sType    = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	//inputAssembly.primitiveRestartEnable = VK_FALSE;

	pipelineInfo.sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount          = 2;
	pipelineInfo.pStages             = shaderStages;
	pipelineInfo.pColorBlendState    = this->colorBlending;
	pipelineInfo.pDepthStencilState  = this->depthStencilBuffer;
	//pipelineInfo.pDynamicState       = nullptr;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pMultisampleState   = this->multisampling;
	pipelineInfo.pRasterizationState = this->rasterizer;
	pipelineInfo.pVertexInputState   = &vertexInput;
	pipelineInfo.pViewportState      = this->viewportState;
	//pipelineInfo.basePipelineHandle  = VK_NULL_HANDLE;
	pipelineInfo.basePipelineIndex   = -1;
	pipelineInfo.layout              = this->pipelineLayout;
	pipelineInfo.renderPass          = this->renderPass;
	//pipelineInfo.subpass             = 0;

	//switch((ShaderID)i) {
	//case SHADER_ID_HUD:
	//	break;
	//case SHADER_ID_SKYBOX:
	//	break;
	//case SHADER_ID_WATER:
	//	break;
	//case SHADER_ID_SOLID:
	//	break;
	//default:
	//	break;
	//}

	if (vkCreateGraphicsPipelines(this->deviceContext, nullptr, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS)
		return nullptr;

	return true;
}

VkPipelineLayout VulkanContext::initPipelineLayout()
{
	if (this->deviceContext == nullptr)
		return nullptr;

	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	VkPipelineLayout           pipelineLayout     = nullptr;

	pipelineLayoutInfo.sType          = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts    = &this->uniformLayout;
	//pipelineLayoutInfo.pushConstantRangeCount = 0;
	//pipelineLayoutInfo.pPushConstantRanges    = 0;

	if (vkCreatePipelineLayout(this->deviceContext, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
		return nullptr;

	return pipelineLayout;
}

VkPipelineRasterizationStateCreateInfo* VulkanContext::initRasterizer()
{
	VkPipelineRasterizationStateCreateInfo* rasterizer = new VkPipelineRasterizationStateCreateInfo();

	rasterizer->sType     = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	//rasterizer.depthClampEnable        = VK_FALSE;
	//rasterizer.rasterizerDiscardEnable = VK_FALSE;
	//rasterizer.polygonMode             = VK_POLYGON_MODE_FILL;
	//rasterizer.polygonMode             = VK_POLYGON_MODE_LINE;
	//rasterizer.polygonMode             = VK_POLYGON_MODE_POINT;
	rasterizer->lineWidth = 1.0f;
	rasterizer->cullMode  = VK_CULL_MODE_BACK_BIT;
	//rasterizer->frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizer->frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	//rasterizer.depthBiasEnable         = VK_FALSE;
	//rasterizer.depthBiasConstantFactor = 0.0f;
	//rasterizer.depthBiasClamp          = 0.0f;
	//rasterizer.depthBiasSlopeFactor    = 0.0f;

	return rasterizer;
}

VkRenderPass VulkanContext::initRenderPass()
{
	VkAttachmentDescription colorAttachment    = {};
	VkAttachmentReference   colorAttachmentRef = {};
	VkRenderPassCreateInfo  renderPassInfo     = {};
	VkSubpassDescription    subpass            = {};
	VkSubpassDependency     subPassDependency  = {};
	VkRenderPass            renderPass         = nullptr;

	colorAttachment.format         = this->swapChain->SurfaceFormat->format;
	colorAttachment.samples        = VK_SAMPLE_COUNT_1_BIT;
	//colorAttachment.loadOp         = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	//colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	//subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments    = &colorAttachmentRef;

	renderPassInfo.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments    = &colorAttachment;
	renderPassInfo.subpassCount    = 1;
	renderPassInfo.pSubpasses      = &subpass;

	subPassDependency.dstAccessMask = (VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT);
	subPassDependency.dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	//subPassDependency.dstSubpass    = 0;
	//subPassDependency.srcAccessMask = 0;
	subPassDependency.srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subPassDependency.srcSubpass    = VK_SUBPASS_EXTERNAL;

	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies   = &subPassDependency;

	if (vkCreateRenderPass(this->deviceContext, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
		return nullptr;

	return renderPass;
}

VkSurfaceKHR VulkanContext::initSurface()
{
	if (this->instance == nullptr)
		return nullptr;

	VkSurfaceKHR surface = nullptr;

	#if defined _WINDOWS
		VkWin32SurfaceCreateInfoKHR surfaceInfo = {};

		surfaceInfo.sType     = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		surfaceInfo.hwnd      = (HWND)RenderEngine::Canvas.Canvas->GetHWND();
		surfaceInfo.hinstance = GetModuleHandle(nullptr);

		if (vkCreateWin32SurfaceKHR(this->instance, &surfaceInfo, nullptr, &surface) != VK_SUCCESS)
			return nullptr;
	#elif defined _LINUX
		VkXcbSurfaceCreateInfoKHR surfaceInfo = {};

		surfaceInfo.sType     = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
		//surfaceInfo.connection = (xcb_connection_t*)RenderEngine::Canvas.Canvas->GetX();
		surfaceInfo.window    = (xcb_window_t)RenderEngine::Canvas.Canvas->GetX11();
		surfaceInfo.hinstance = GetModuleHandle(nullptr);

		if (vkCreateXcbSurfaceKHR(this->instance, &surfaceInfo, nullptr, &surface) != VK_SUCCESS)
			return nullptr;
	#endif

	return surface;
}

VulkanSwapchain* VulkanContext::initSwapChain(VulkanSwapChainSupport* swapChainSupport, VkSurfaceKHR surface)
{
	if ((swapChainSupport == nullptr) || (this->deviceContext == nullptr))
		return nullptr;

	VkPresentModeKHR presentMode   = this->getPresentationMode(swapChainSupport->PresentModes);
	uint32_t         maxImageCount = swapChainSupport->SurfaceCapabilities.maxImageCount;
	uint32_t         imageCount    = (swapChainSupport->SurfaceCapabilities.minImageCount + 1);
	VulkanSwapchain* swapChain     = new VulkanSwapchain(this->deviceContext);

	swapChain->SurfaceFormat = this->getSurfaceFormat(swapChainSupport->SurfaceFormats);
	swapChain->Size          = this->getSurfaceSize(swapChainSupport->SurfaceCapabilities);

	// Make sure we don't request more images than max allowed
	if ((maxImageCount > 0) && (imageCount > maxImageCount))
		imageCount = maxImageCount;

	VkSwapchainCreateInfoKHR swapChainInfo = {};

	swapChainInfo.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapChainInfo.clipped          = VK_TRUE;
	swapChainInfo.compositeAlpha   = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	//swapChainInfo.oldSwapchain     = VK_NULL_HANDLE;
	//swapChainInfo.oldSwapchain     = this->swapChain->SwapChain;
	swapChainInfo.preTransform     = swapChainSupport->SurfaceCapabilities.currentTransform;
	swapChainInfo.presentMode      = presentMode;
	swapChainInfo.surface          = surface;
	swapChainInfo.minImageCount    = imageCount;
	swapChainInfo.imageFormat      = swapChain->SurfaceFormat->format;
	swapChainInfo.imageColorSpace  = swapChain->SurfaceFormat->colorSpace;
	swapChainInfo.imageExtent      = *swapChain->Size;
	swapChainInfo.imageArrayLayers = 1;
	swapChainInfo.imageUsage       = (VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
	swapChainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;

	// Use concurrenct share mode if the graphics and presentation queues are different
	if (this->queues[VULKAN_QUEUE_PRESENTATION]->Queue != this->queues[VULKAN_QUEUE_GRAPHICS]->Queue)
	{
		uint32_t queueIndices[] = {
			(uint32_t)this->queues[VULKAN_QUEUE_PRESENTATION]->Index,
			(uint32_t)this->queues[VULKAN_QUEUE_GRAPHICS]->Index
		};

		swapChainInfo.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
		swapChainInfo.queueFamilyIndexCount = 2;
		swapChainInfo.pQueueFamilyIndices   = queueIndices;
	}

	// Create the swap chain
	if (vkCreateSwapchainKHR(this->deviceContext, &swapChainInfo, nullptr, &swapChain->SwapChain) != VK_SUCCESS)
		return nullptr;

	// Get the swap chain images
	vkGetSwapchainImagesKHR(this->deviceContext, swapChain->SwapChain, &imageCount, nullptr);
	swapChain->Images.resize(imageCount);
	vkGetSwapchainImagesKHR(this->deviceContext, swapChain->SwapChain, &imageCount, swapChain->Images.data());

	// Create the swap chain image views based on the images
	swapChain->ImageViews.resize(swapChain->Images.size());

	for (int i = 0; i < (int)swapChain->Images.size(); i++)
	{
		VkImageViewCreateInfo imageViewInfo = {};

		imageViewInfo.sType                       = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewInfo.format                      = swapChain->SurfaceFormat->format;
		imageViewInfo.image                       = swapChain->Images[i];
		imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageViewInfo.subresourceRange.levelCount = 1;
		imageViewInfo.subresourceRange.layerCount = 1;
		imageViewInfo.viewType                    = VK_IMAGE_VIEW_TYPE_2D;

		if (vkCreateImageView(this->deviceContext, &imageViewInfo, nullptr, &swapChain->ImageViews[i]) != VK_SUCCESS)
			swapChain->ImageViews[i] = nullptr;
	}

	return swapChain;
}

bool VulkanContext::initSync()
{
	VkFenceCreateInfo     fenceInfo     = {};
	VkSemaphoreCreateInfo semaphoreInfo = {};

	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	this->frameFences.resize(MAX_CONCURRENT_FRAMES);
	this->semDrawComplete.resize(MAX_CONCURRENT_FRAMES);
	this->semImageAvailable.resize(MAX_CONCURRENT_FRAMES);

	for (unsigned int i = 0; i < MAX_CONCURRENT_FRAMES; i++)
	{
		if (vkCreateFence(this->deviceContext, &fenceInfo, nullptr, &this->frameFences[i]) != VK_SUCCESS)
			return false;

		if (vkCreateSemaphore(this->deviceContext, &semaphoreInfo, nullptr, &this->semDrawComplete[i]) != VK_SUCCESS)
			return false;

		if (vkCreateSemaphore(this->deviceContext, &semaphoreInfo, nullptr, &this->semImageAvailable[i]) != VK_SUCCESS)
			return false;

		//vkCreateSemaphore(this->deviceContext, &semaphoreInfo, nullptr, &this->semDrawComplete2);
		//vkCreateSemaphore(this->deviceContext, &semaphoreInfo, nullptr, &this->semImageAvailable2);
	}

	return true;
}

VkPipelineViewportStateCreateInfo* VulkanContext::initViewportState()
{
	VkRect2D*                          scissorRect   = new VkRect2D();
	VkViewport*                        viewport      = new VkViewport();
	VkPipelineViewportStateCreateInfo* viewportState = new VkPipelineViewportStateCreateInfo();

	viewport->x        = 0.0f;
	viewport->y        = 0.0f;
	viewport->width    = (float)this->swapChain->Size->width;
	viewport->height   = (float)this->swapChain->Size->height;
	viewport->minDepth = 0.0f;
	viewport->maxDepth = 1.0f;

	scissorRect->offset = { 0, 0 };
	scissorRect->extent = *this->swapChain->Size;

	viewportState->sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState->viewportCount = 1;
	viewportState->pViewports    = viewport;
	viewportState->scissorCount  = 1;
	viewportState->pScissors     = scissorRect;

	return viewportState;
}

//VkDescriptorSetLayout VulkanContext::initUniformLayout(uint32_t binding, VkShaderStageFlags shaderFlags)
VkDescriptorSetLayout VulkanContext::initUniformLayout()
{
	if (this->deviceContext == nullptr)
		return nullptr;

	VkDescriptorSetLayoutBinding    uniformMatrixBinding  = {};
	VkDescriptorSetLayoutBinding    uniformDefaultBinding = {};
	VkDescriptorSetLayoutCreateInfo uniformLayoutInfo     = {};
	VkDescriptorSetLayout           uniformLayout         = nullptr;

	uniformMatrixBinding.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uniformMatrixBinding.descriptorCount = 1;
	uniformMatrixBinding.binding         = 0;
	uniformMatrixBinding.stageFlags      = VK_SHADER_STAGE_VERTEX_BIT;
	//uniformLayoutBinding.pImmutableSamplers = nullptr;

	uniformDefaultBinding.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uniformDefaultBinding.descriptorCount = 1;
	uniformDefaultBinding.binding         = 1;
	uniformDefaultBinding.stageFlags      = VK_SHADER_STAGE_FRAGMENT_BIT;

	VkDescriptorSetLayoutBinding uniformLayoutBindings[] = { uniformMatrixBinding, uniformDefaultBinding };

	uniformLayoutInfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	uniformLayoutInfo.bindingCount = 2;
	uniformLayoutInfo.pBindings    = uniformLayoutBindings;

	if (vkCreateDescriptorSetLayout(this->deviceContext, &uniformLayoutInfo, nullptr, &uniformLayout) != VK_SUCCESS)
		return nullptr;

	return uniformLayout;
}

VkDescriptorPool VulkanContext::initUniformPool()
{
	if (this->deviceContext == nullptr)
		return nullptr;

	VkDescriptorPoolCreateInfo uniformPoolInfo = {};
	VkDescriptorPoolSize       uniformPoolSize = {};
	VkDescriptorPool           uniformPool     = nullptr;

	// TODO: VULKAN
	uniformPoolSize.type            = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uniformPoolSize.descriptorCount = 2;

	uniformPoolInfo.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	uniformPoolInfo.poolSizeCount = 1;
	uniformPoolInfo.pPoolSizes    = &uniformPoolSize;
	uniformPoolInfo.maxSets       = uniformPoolSize.descriptorCount;

	if (vkCreateDescriptorPool(this->deviceContext, &uniformPoolInfo, nullptr, &uniformPool) != VK_SUCCESS)
		return nullptr;

	return uniformPool;
}

VkDescriptorSet VulkanContext::initUniformSet()
{
	if (this->deviceContext == nullptr)
		return nullptr;

	//VkDescriptorSetLayout       uniformSetLayouts[] = { this->uniformLayout };
	VkDescriptorSetAllocateInfo uniformSetAllocInfo = {};
	VkDescriptorSet             uniformSet          = nullptr;

	uniformSetAllocInfo.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	uniformSetAllocInfo.descriptorSetCount = 1;
	uniformSetAllocInfo.pSetLayouts        = &this->uniformLayout;
	uniformSetAllocInfo.descriptorPool     = this->uniformPool;

	if (vkAllocateDescriptorSets(this->deviceContext, &uniformSetAllocInfo, &uniformSet) != VK_SUCCESS)
		return nullptr;

	return uniformSet;
}

bool VulkanContext::init(bool vsync)
{
	this->frameIndex = 0;

	this->instance = this->initInstance();

	if (this->instance == nullptr)
		return false;

	this->surface = this->initSurface();

	if (this->surface == nullptr)
		return false;

	this->deviceContext = this->initDeviceContext();

	if (this->deviceContext == nullptr)
		return false;

	this->swapChain = this->initSwapChain(this->swapChainSupport, this->surface);

	if (this->swapChain == nullptr)
		return false;

	this->viewportState = this->initViewportState();

	this->renderPass = this->initRenderPass();

	if (this->renderPass == nullptr)
		return false;

	this->colorBlending = this->initColorBlending();

	if (this->colorBlending == nullptr)
		return false;

	this->depthStencilBuffer = this->initDepthStencilBuffer();
	this->multisampling      = this->initMultisampling();
	this->rasterizer         = this->initRasterizer();

	// TODO: VULKAN
	//this->uniformLayouts[0] = this->initUniformLayout(0, VK_SHADER_STAGE_VERTEX_BIT);
	//this->uniformLayouts[1] = this->initUniformLayout(1, VK_SHADER_STAGE_FRAGMENT_BIT);

	//if ((this->uniformLayouts[0] == nullptr) || (this->uniformLayouts[1] == nullptr))
	//	return false;

	this->uniformLayout = this->initUniformLayout();

	if (this->uniformLayout == nullptr)
		return false;

	this->uniformPool = this->initUniformPool();

	if (this->uniformPool == nullptr)
		return false;

	this->uniformSet = this->initUniformSet();

	if (this->uniformSet == nullptr)
		return false;

	this->pipelineLayout = this->initPipelineLayout();

	if (this->pipelineLayout == nullptr)
		return false;

	this->frameBuffers = this->initFramebuffers();

	if (this->frameBuffers.empty())
		return false;

	this->commandPool = this->initCommandPool();

	if (this->commandPool == nullptr)
		return false;

	this->commandBuffers = this->initCommandBuffers(this->frameBuffers.size());

	if (this->commandBuffers.empty())
		return false;

	if (!this->initSync())
		return false;

	/*
	Dynamic state
	A limited amount of the state that we've specified in the previous structs can actually be changed
	without recreating the pipeline. Examples are the size of the viewport, line width and blend constants.
	If you want to do that, then you'll have to fill in a VkPipelineDynamicStateCreateInfo structure like this:

	VkDynamicState                   dynamicStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_LINE_WIDTH };
	VkPipelineDynamicStateCreateInfo dynamicState    = {};

	dynamicState.sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = 2;
	dynamicState.pDynamicStates    = dynamicStates;
	*/

	this->SetVSync(vsync);

	RenderEngine::GPU.Vendor   = wxT("");
	RenderEngine::GPU.Renderer = this->getDeviceName(this->device);
	RenderEngine::GPU.Version  = this->getApiVersion(this->device);

	return true;
}

bool VulkanContext::IsOK()
{
	return this->isOK;
}

void VulkanContext::release()
{
	if (this->deviceContext != nullptr)
		vkDeviceWaitIdle(this->deviceContext);

	for (int i = 0; i < NR_OF_SHADERS; i++)
		_DELETEP(ShaderManager::Programs[i]);

	for (auto fence : this->frameFences) {
		if (fence != nullptr)
			vkDestroyFence(this->deviceContext, fence, nullptr);
	}

	this->frameFences.clear();

	for (auto semaphore : this->semDrawComplete) {
		if (semaphore != nullptr)
			vkDestroySemaphore(this->deviceContext, semaphore, nullptr);
	}

	this->semDrawComplete.clear();

	for (auto semaphore : this->semImageAvailable) {
		if (semaphore != nullptr)
			vkDestroySemaphore(this->deviceContext, semaphore, nullptr);
	}

	this->semImageAvailable.clear();

	if (!this->commandBuffers.empty()) {
		vkFreeCommandBuffers(this->deviceContext, this->commandPool, (uint32_t)this->commandBuffers.size(), this->commandBuffers.data());
		this->commandBuffers.clear();
	}

	if (this->commandPool != nullptr) {
		vkDestroyCommandPool(this->deviceContext, this->commandPool, nullptr);
		this->commandPool = nullptr;
	}

	for (auto frameBuffer : this->frameBuffers) {
		if (frameBuffer != nullptr)
			vkDestroyFramebuffer(this->deviceContext, frameBuffer, nullptr);
	}

	this->frameBuffers.clear();

	if (this->pipelineLayout != nullptr) {
		vkDestroyPipelineLayout(this->deviceContext, this->pipelineLayout, nullptr);
		this->pipelineLayout = nullptr;
	}

	if (this->uniformPool != nullptr) {
		vkDestroyDescriptorPool(this->deviceContext, this->uniformPool, nullptr);
		this->uniformPool = nullptr;
	}

	if (this->uniformLayout != nullptr) {
		vkDestroyDescriptorSetLayout(this->deviceContext, this->uniformLayout, nullptr);
		this->uniformLayout = nullptr;
	}

	_DELETEP(this->rasterizer);
	_DELETEP(this->multisampling);
	_DELETEP(this->depthStencilBuffer);

	if (this->colorBlending != nullptr)
		_DELETEP(this->colorBlending->pAttachments);

	_DELETEP(this->colorBlending);

	if (this->renderPass != nullptr) {
		vkDestroyRenderPass(this->deviceContext, this->renderPass, nullptr);
		this->renderPass = nullptr;
	}

	if (this->viewportState != nullptr) {
		_DELETEP(this->viewportState->pScissors);
		_DELETEP(this->viewportState->pViewports);
		_DELETEP(this->viewportState);
	}

	_DELETEP(this->swapChain);

	if (this->deviceContext != nullptr) {
		vkDestroyDevice(this->deviceContext, nullptr);
		this->deviceContext = nullptr;
	}

	for (auto queue : this->queues)
		_DELETEP(queue.second);

	this->queues.clear();

	_DELETEP(this->swapChainSupport);

	if (this->surface != nullptr) {
		vkDestroySurfaceKHR(this->instance, this->surface, nullptr);
		this->surface = nullptr;
	}

	#if defined _DEBUG
		// Try to load the external method
		PFN_vkDestroyDebugReportCallbackEXT destroyDebugReportCallbackEXT = VK_NULL_HANDLE;
		destroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");

		if ((destroyDebugReportCallbackEXT != VK_NULL_HANDLE) && (this->debugCallback != nullptr)) {
			destroyDebugReportCallbackEXT(this->instance, this->debugCallback, nullptr);
			this->debugCallback = nullptr;
		}
	#endif

	if (this->instance != nullptr) {
		vkDestroyInstance(this->instance, nullptr);
		this->instance = nullptr;
	}
}

void VulkanContext::Present()
{
	VkPresentInfoKHR     presentInfo        = {};
	VkSemaphore          signalSemaphores[] = { this->semDrawComplete[this->frameIndex] };
	VkSubmitInfo         submitInfo         = {};
	VkSwapchainKHR       swapChains[]       = { this->swapChain->SwapChain };
	VkSemaphore          waitSemaphores[]   = { this->semImageAvailable[this->frameIndex] };
	VkPipelineStageFlags waitStages[]       = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

	// STOP RENDER PASS
	vkCmdEndRenderPass(this->commandBuffers[this->imageIndex]);

	// STOP RECORDING COMMAND BUFFER
	vkEndCommandBuffer(this->commandBuffers[this->imageIndex]);

	// SUBMIT DRAW COMMAND TO QUEUE
	submitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pCommandBuffers      = &this->commandBuffers[this->imageIndex];
	submitInfo.commandBufferCount   = 1;
	submitInfo.pSignalSemaphores    = signalSemaphores;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pWaitSemaphores      = waitSemaphores;
	submitInfo.waitSemaphoreCount   = 1;
	submitInfo.pWaitDstStageMask    = waitStages;

	vkQueueSubmit(this->queues[VULKAN_QUEUE_GRAPHICS]->Queue, 1, &submitInfo, this->frameFences[this->frameIndex]);

	// PRESENT THE DRAWN BUFFER TO SCREEN
	presentInfo.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pWaitSemaphores    = signalSemaphores;
	presentInfo.waitSemaphoreCount = 1;

	presentInfo.pImageIndices  = &this->imageIndex;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains    = swapChains;

	VkResult result = vkQueuePresentKHR(this->queues[VULKAN_QUEUE_PRESENTATION]->Queue, &presentInfo);

	if ((result == VK_ERROR_OUT_OF_DATE_KHR) || (result == VK_SUBOPTIMAL_KHR))
		this->UpdateSwapChain();

	this->frameIndex = ((this->frameIndex + 1) % MAX_CONCURRENT_FRAMES);

	// WAIT FOR PRESENTATION TO FINISH
	vkQueueWaitIdle(this->queues[VULKAN_QUEUE_PRESENTATION]->Queue);
}

void VulkanContext::SetVSync(bool enable)
{
	this->vSync = enable;
}

bool VulkanContext::UpdateSwapChain()
{
	this->release();

	if (!this->init(this->vSync))
		return false;

	if (ShaderManager::Init() != 0)
		return false;

	return true;
}
