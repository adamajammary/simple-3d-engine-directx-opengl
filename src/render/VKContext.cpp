#include "VKContext.h"

VKContext::VKContext(bool vsync)
{
	this->isOK = this->init(vsync);

	if (!this->isOK)
		this->release();
}

VKContext::~VKContext()
{
	this->release();
}

void VKContext::blitImage(VkCommandBuffer cmdBuffer, VkImage image, int mipWidth, int mipHeight, int index)
{
	VkImageBlit imageBlit = {};

	imageBlit.srcOffsets[1] = { mipWidth, mipHeight, 1 };

	imageBlit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageBlit.srcSubresource.mipLevel   = (index - 1);
	imageBlit.srcSubresource.layerCount = 1;

	imageBlit.dstOffsets[1] = { (mipWidth > 1 ? (mipWidth / 2) : 1), (mipHeight > 1 ? (mipHeight / 2) : 1), 1 };

	imageBlit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageBlit.dstSubresource.mipLevel   = index;
	imageBlit.dstSubresource.layerCount = 1;

	vkCmdBlitImage(
		cmdBuffer,
		image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1, &imageBlit, VK_FILTER_LINEAR
	);
}

void VKContext::Clear(const glm::vec4 &colorRGBA, const DrawProperties &properties)
{
	VkCommandBuffer commandBuffer = properties.VKCommandBuffer;
	VkRect2D        scissorRect   = {};
	VkViewport      viewport      = {};

	if ((properties.FBO != nullptr) && (commandBuffer != nullptr))
	{
		Texture* texture = properties.FBO->GetTexture();

		if (texture != nullptr)
		{
			VkClearValue          clearValues[1] = {};
			VkRenderPassBeginInfo renderPassInfo = {};
			wxSize                textureSize    = texture->Size();

			// START RENDER PASS
			renderPassInfo.sType             = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderArea.extent = { (uint32_t)textureSize.GetWidth(), (uint32_t)textureSize.GetHeight() };

			switch (properties.FBO->Type()) {
			case FBO_COLOR:
				clearValues[0].color      = { colorRGBA.r, colorRGBA.g, colorRGBA.b, colorRGBA.a };
				renderPassInfo.renderPass = this->renderPasses[RENDER_PASS_FBO_COLOR];
				break;
			case FBO_DEPTH:
				clearValues[0].depthStencil = { 1.0f, 0 };
				renderPassInfo.renderPass   = this->renderPasses[RENDER_PASS_FBO_DEPTH];
				break;
			default:
				throw;
			}

			renderPassInfo.clearValueCount = 1;
			renderPassInfo.pClearValues    = clearValues;

			if (properties.FBO->Type() == FBO_DEPTH)
				renderPassInfo.framebuffer = texture->DepthBuffers[properties.DepthLayer];
			else
				renderPassInfo.framebuffer = texture->BufferVK;

			vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

			viewport = texture->BufferViewPort();
		}
	}
	else
	{
		// WAIT FOR LAST FRAME TO FINISH
		vkWaitForFences(this->deviceContext, 1, &this->frameFences[this->frameIndex], VK_TRUE, UINT64_MAX);
		vkResetFences(this->deviceContext,   1, &this->frameFences[this->frameIndex]);

		VkResult result = vkAcquireNextImageKHR(
			this->deviceContext, this->swapChain->SwapChain, UINT64_MAX,
			this->semImageAvailable[this->frameIndex], nullptr, &this->imageIndex
		);

		if (result == VK_ERROR_OUT_OF_DATE_KHR)
			this->ResetSwapChain();

		commandBuffer = this->commandBuffers[this->imageIndex];

		// START RECORDING COMMAND BUFFER
		VkCommandBufferBeginInfo beginCommandInfo = {};

		beginCommandInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginCommandInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

		vkBeginCommandBuffer(commandBuffer, &beginCommandInfo);

		VkClearValue clearValues[2] = {};

		clearValues[0].color        = { colorRGBA.r, colorRGBA.g, colorRGBA.b, colorRGBA.a };
		clearValues[1].depthStencil = { 1.0f, 0 };

		// START RENDER PASS
		VkRenderPassBeginInfo renderPassInfo = {};

		renderPassInfo.sType             = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.clearValueCount   = 2;
		renderPassInfo.pClearValues      = clearValues;
		renderPassInfo.framebuffer       = this->frameBuffers[this->imageIndex];
		renderPassInfo.renderPass        = this->renderPasses[RENDER_PASS_DEFAULT];
		renderPassInfo.renderArea.extent = this->swapChain->Size;

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		viewport.x        = 0.0f;
		viewport.y        = 0.0f;
		viewport.width    = (float)this->swapChain->Size.width;
		viewport.height   = (float)this->swapChain->Size.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
	}

	scissorRect.extent = { (uint32_t)viewport.width, (uint32_t)viewport.height };

	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
	vkCmdSetScissor(commandBuffer,  0, 1, &scissorRect);
}

VkCommandBuffer VKContext::CommandBufferBegin()
{
	VkCommandBufferBeginInfo     commandInfo    = {};
	std::vector<VkCommandBuffer> commandBuffers = this->initCommandBuffers(1);

	if (commandBuffers.empty() || (commandBuffers[0] == nullptr))
		return nullptr;

	commandInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	commandInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffers[0], &commandInfo);

    return commandBuffers[0];
}

void VKContext::CommandBufferEnd(VkCommandBuffer cmdBuffer)
{
	VkSubmitInfo queueSubmitInfo = {};

	vkEndCommandBuffer(cmdBuffer);

	queueSubmitInfo.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	queueSubmitInfo.commandBufferCount = 1;
	queueSubmitInfo.pCommandBuffers    = &cmdBuffer;

	vkQueueSubmit(this->queues[VK_QUEUE_GRAPHICS]->Queue, 1, &queueSubmitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(this->queues[VK_QUEUE_GRAPHICS]->Queue);

	vkFreeCommandBuffers(this->deviceContext, this->commandPool, 1, &cmdBuffer);
}

int VKContext::copyBuffer(VkBuffer sourceBuffer, VkBuffer destinationBuffer, VkDeviceSize bufferSize)
{
	VkCommandBuffer cmdBuffer = this->CommandBufferBegin();

	if (cmdBuffer != nullptr)
	{
		VkBufferCopy copyRegion = { 0, 0, bufferSize };

		vkCmdCopyBuffer(cmdBuffer, sourceBuffer, destinationBuffer, 1, &copyRegion);
		this->CommandBufferEnd(cmdBuffer);
	}

	return 0;
}

int VKContext::copyBufferToImage(VkBuffer buffer, VkImage image, int colorComponents, uint32_t width, uint32_t height, uint32_t index)
{
	VkCommandBuffer cmdBuffer = this->CommandBufferBegin();

	if (cmdBuffer == nullptr)
		return -1;

	VkBufferImageCopy copyRegion = {};

	copyRegion.bufferOffset                    = (index * width * height * colorComponents);
	copyRegion.imageExtent                     = { width, height, 1 };
	copyRegion.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
	copyRegion.imageSubresource.baseArrayLayer = index;
	copyRegion.imageSubresource.layerCount     = 1;

	vkCmdCopyBufferToImage(cmdBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

	this->CommandBufferEnd(cmdBuffer);

	return 0;
}

int VKContext::copyImage(VkImage image, VkFormat imageFormat, uint32_t mipLevels, TextureType textureType, VkImageLayout oldLayout, VkImageLayout newLayout)
{
	VkCommandBuffer cmdBuffer = this->CommandBufferBegin();

	if (cmdBuffer == nullptr)
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
	imageMemBarrier.subresourceRange.levelCount = mipLevels;

	switch (textureType) {
		case TEXTURE_2D:            imageMemBarrier.subresourceRange.layerCount = 1; break;
		case TEXTURE_CUBEMAP:       imageMemBarrier.subresourceRange.layerCount = MAX_TEXTURES; break;
		case TEXTURE_2D_ARRAY:      imageMemBarrier.subresourceRange.layerCount = MAX_LIGHT_SOURCES; break;
		case TEXTURE_CUBEMAP_ARRAY: imageMemBarrier.subresourceRange.layerCount = (MAX_LIGHT_SOURCES * MAX_TEXTURES); break;
		default: throw;
	}

	// DEPTH BUFFER
	if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
		imageMemBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	else
		imageMemBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

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
	// TO DEPTH BUFFER
	else if ((oldLayout == VK_IMAGE_LAYOUT_UNDEFINED) && (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL))
	{
		imageMemBarrier.srcAccessMask = 0;
		imageMemBarrier.dstAccessMask = (VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT);

		pipelineStageSrcFlags  = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		pipelineStageDestFlags = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	}
	// TO COLOR BUFFER
	else if ((oldLayout == VK_IMAGE_LAYOUT_UNDEFINED) && (newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL))
	{
		imageMemBarrier.srcAccessMask = 0;
		imageMemBarrier.dstAccessMask = (VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT);

		pipelineStageSrcFlags  = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		pipelineStageDestFlags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    }
	// UNKNOWN TRANSITION
	else
	{
		throw;
	}

	vkCmdPipelineBarrier(cmdBuffer, pipelineStageSrcFlags, pipelineStageDestFlags, 0, 0, nullptr, 0, nullptr, 1, &imageMemBarrier);

	this->CommandBufferEnd(cmdBuffer);

	return 0;
}

int VKContext::createBuffer(
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

int VKContext::createImage(
	uint32_t              width,
	uint32_t              height,
	uint32_t              mipLevels,
	uint32_t              sampleCount,
	VkFormat              format,
	VkImageTiling         tiling,
	VkImageUsageFlags     useFlags,
	VkMemoryPropertyFlags memoryFlags,
	TextureType           textureType,
	VkImage*              image,
	VkDeviceMemory*       imageMemory
)
{
	VkImageCreateInfo imageInfo = {};

	imageInfo.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.extent        = { width, height, 1 };
	imageInfo.flags         = 0;
	imageInfo.format        = format;
	imageInfo.imageType     = VK_IMAGE_TYPE_2D;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.mipLevels     = 1;
	imageInfo.samples       = (VkSampleCountFlagBits)sampleCount;
	imageInfo.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.tiling        = tiling;
	imageInfo.usage         = useFlags;

	switch (textureType) {
	case TEXTURE_2D:
		imageInfo.arrayLayers = 1;
		imageInfo.mipLevels   = mipLevels;
		break;
	case TEXTURE_CUBEMAP:
		imageInfo.arrayLayers = MAX_TEXTURES;
		imageInfo.flags       = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
		break;
	case TEXTURE_2D_ARRAY:
		imageInfo.arrayLayers = MAX_LIGHT_SOURCES;
		break;
	case TEXTURE_CUBEMAP_ARRAY:
		imageInfo.arrayLayers = (MAX_LIGHT_SOURCES * MAX_TEXTURES);
		imageInfo.flags       = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
		break;
	default:
		throw;
	}

	if (vkCreateImage(this->deviceContext, &imageInfo, nullptr, image) != VK_SUCCESS)
		return -2;

	// IMAGE MEMORY
	VkMemoryAllocateInfo imageAllocInfo = {};
	VkMemoryRequirements imageMemReq    = {};

	vkGetImageMemoryRequirements(this->deviceContext, *image, &imageMemReq);

	imageAllocInfo.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	imageAllocInfo.allocationSize  = imageMemReq.size;
	imageAllocInfo.memoryTypeIndex = this->getMemoryType(imageMemReq.memoryTypeBits, memoryFlags);

	if (imageAllocInfo.memoryTypeIndex < 0)
		return -3;

	if (vkAllocateMemory(this->deviceContext, &imageAllocInfo, nullptr, imageMemory) != VK_SUCCESS)
		return -4;

	vkBindImageMemory(this->deviceContext, *image, *imageMemory, 0);

	return 0;
}

VkSampler VKContext::createImageSampler(float mipLevels, float sampleCount, VkSamplerCreateInfo &samplerInfo)
{
	samplerInfo.sType            = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy    = sampleCount;
	samplerInfo.maxLod           = mipLevels;
	samplerInfo.mipmapMode       = VK_SAMPLER_MIPMAP_MODE_LINEAR;

	if (samplerInfo.borderColor == 0)
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;

	VkSampler imageSampler = nullptr;

	if (vkCreateSampler(this->deviceContext, &samplerInfo, nullptr, &imageSampler) != VK_SUCCESS)
		return nullptr;

	return imageSampler;
}

VkImageView VKContext::createImageView(VkImage image, VkFormat imageFormat, VkImageAspectFlags aspectFlags, uint32_t mipLevels, VkImageViewType viewType, uint32_t layerCount, uint32_t layer)
{
	VkImageViewCreateInfo imageViewInfo = {};

	imageViewInfo.sType      = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageViewInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
	imageViewInfo.format     = imageFormat;
	imageViewInfo.image      = image;
	imageViewInfo.viewType   = viewType;

	imageViewInfo.subresourceRange.aspectMask     = aspectFlags;
	imageViewInfo.subresourceRange.baseArrayLayer = layer;
	imageViewInfo.subresourceRange.layerCount     = layerCount;
	imageViewInfo.subresourceRange.levelCount     = mipLevels;

	VkImageView imageView = nullptr;

	if (vkCreateImageView(this->deviceContext, &imageViewInfo, nullptr, &imageView) != VK_SUCCESS)
		return nullptr;

	return imageView;
}

int VKContext::CreateIndexBuffer(const std::vector<uint32_t> &indices, Buffer* buffer)
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
	if (this->createBuffer(indexBufferSize, indexBufferUseFlags, indexBufferMemFlags, &buffer->IndexBuffer, &buffer->IndexBufferMemory) < 0)
		return -2;

	// COPY DATA FROM STAGING TO VERTEX BUFFER (DEVICE LOCAL)
	this->copyBuffer(stagingBuffer, buffer->IndexBuffer, indexBufferSize);

	vkFreeMemory(this->deviceContext,    stagingBufferMemory, nullptr);
	vkDestroyBuffer(this->deviceContext, stagingBuffer,       nullptr);

	return 0;
}

int VKContext::createMipMaps(VkImage image, VkFormat imageFormat, int width, int height, uint32_t mipLevels)
{
	VkFormatProperties formatProperties = {};

	vkGetPhysicalDeviceFormatProperties(this->device, imageFormat, &formatProperties);

	if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
		return -1;

	VkCommandBuffer      cmdBuffer       = this->CommandBufferBegin();
	VkImageMemoryBarrier imageMemBarrier = {};
	int                  mipWidth        = width;
	int                  mipHeight       = height;

	// BASE MIP LEVEL
	imageMemBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imageMemBarrier.image = image;

	imageMemBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageMemBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

	imageMemBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageMemBarrier.subresourceRange.layerCount = 1;
	imageMemBarrier.subresourceRange.levelCount = 1;

	// MIP LEVELS
	for (uint32_t i = 1; i < mipLevels; i++)
	{
		// TRANSITION TO SOURCE LAYOUT
		imageMemBarrier.subresourceRange.baseMipLevel = (i - 1);

		imageMemBarrier.oldLayout     = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		imageMemBarrier.newLayout     = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		imageMemBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		imageMemBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

		this->transitionImageLayout(cmdBuffer, imageMemBarrier, VK_PIPELINE_STAGE_TRANSFER_BIT);

		// BLIT IMAGE
		this->blitImage(cmdBuffer, image, mipWidth, mipHeight, i);

		// TRANSITION TO FRAGMENT SHADER LAYOUT
		imageMemBarrier.oldLayout     = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		imageMemBarrier.newLayout     = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageMemBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		imageMemBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		this->transitionImageLayout(cmdBuffer, imageMemBarrier, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

		if (mipWidth > 1)
			mipWidth /= 2;

		if (mipHeight > 1)
			mipHeight /= 2;
	}

	// TRANSITION LAST MIP LEVEL TO FRAGMENT SHADER LAYOUT
	imageMemBarrier.subresourceRange.baseMipLevel = (mipLevels - 1);

	imageMemBarrier.oldLayout     = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	imageMemBarrier.newLayout     = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageMemBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	imageMemBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	this->transitionImageLayout(cmdBuffer, imageMemBarrier, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

	this->CommandBufferEnd(cmdBuffer);

	return 0;
}

int VKContext::createPipeline(
	ShaderProgram*   shaderProgram,
	VkPipeline*      pipeline,
	VkPipelineLayout pipelineLayout,
	FBOType          fboType,
	const std::vector<VkVertexInputAttributeDescription> &attribsDescs,
	const VkVertexInputBindingDescription                &attribsBindingDesc
)
{
	if ((this->deviceContext == nullptr) || (shaderProgram == nullptr))
		return -1;

	VkPipelineShaderStageCreateInfo fsStageInfo = {};
	VkPipelineShaderStageCreateInfo gsStageInfo = {};
	VkPipelineShaderStageCreateInfo vsStageInfo = {};

	vsStageInfo.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vsStageInfo.stage  = VK_SHADER_STAGE_VERTEX_BIT;
	vsStageInfo.module = shaderProgram->VulkanVS();
	vsStageInfo.pName  = "main";

	fsStageInfo.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fsStageInfo.stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
	fsStageInfo.module = shaderProgram->VulkanFS();
	fsStageInfo.pName  = "main";

	VkPipelineDynamicStateCreateInfo       dynamicState    = {};
	VkDynamicState                         dynamicStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
	VkPipelineInputAssemblyStateCreateInfo inputAssembly   = {};
	VkPipelineVertexInputStateCreateInfo   vertexInput     = {};

	dynamicState.sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = 2;
	dynamicState.pDynamicStates    = dynamicStates;

	inputAssembly.sType    = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

	vertexInput.sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInput.vertexAttributeDescriptionCount = attribsDescs.size();
	vertexInput.pVertexAttributeDescriptions    = attribsDescs.data();
	vertexInput.vertexBindingDescriptionCount   = 1;
	vertexInput.pVertexBindingDescriptions      = &attribsBindingDesc;

	VkPipelineMultisampleStateCreateInfo multisampleInfo = {};
	VkGraphicsPipelineCreateInfo         pipelineInfo    = {};

	switch (fboType) {
	case FBO_COLOR:
	case FBO_DEPTH:
		multisampleInfo         = this->initMultisampling(1);
		pipelineInfo.renderPass = this->renderPasses[RENDER_PASS_FBO_COLOR];
		break;
	default:
		multisampleInfo         = this->initMultisampling(this->multiSampleCount);
		pipelineInfo.renderPass = this->renderPasses[RENDER_PASS_DEFAULT];
		break;
	}

	ShaderID                                     shaderID     = shaderProgram->ID();
	std::vector<VkPipelineShaderStageCreateInfo> shaderStages = { vsStageInfo, fsStageInfo };

	switch (shaderID) {
	case SHADER_ID_DEPTH:
		pipelineInfo.renderPass = this->renderPasses[RENDER_PASS_FBO_DEPTH];
		break;
	case SHADER_ID_DEPTH_OMNI:
		pipelineInfo.renderPass = this->renderPasses[RENDER_PASS_FBO_DEPTH];

		gsStageInfo.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		gsStageInfo.stage  = VK_SHADER_STAGE_GEOMETRY_BIT;
		gsStageInfo.module = shaderProgram->VulkanGS();
		gsStageInfo.pName  = "main";

		shaderStages.push_back(gsStageInfo);

		break;
	default:
		break;
	}

	VkPipelineViewportStateCreateInfo viewportInfo = this->initViewport();

	pipelineInfo.sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount          = shaderStages.size();
	pipelineInfo.pStages             = shaderStages.data();
	pipelineInfo.pDynamicState       = &dynamicState;
	pipelineInfo.pMultisampleState   = &multisampleInfo;
	pipelineInfo.pVertexInputState   = &vertexInput;
	pipelineInfo.pViewportState      = &viewportInfo;
	pipelineInfo.basePipelineIndex   = -1;
	pipelineInfo.layout              = pipelineLayout;

	VkPipelineColorBlendAttachmentState    colorBlendAttachment = {};
	VkPipelineColorBlendStateCreateInfo    colorBlendInfo       = {};
	VkPipelineDepthStencilStateCreateInfo  depthStencilInfo     = {};
	VkPipelineRasterizationStateCreateInfo rasterizationInfo    = {};

	switch(shaderID) {
	case SHADER_ID_HUD:
		colorBlendInfo    = this->initColorBlending(colorBlendAttachment, VK_TRUE);
		depthStencilInfo  = this->initDepthStencilBuffer(VK_FALSE);
		rasterizationInfo = this->initRasterizer(VK_CULL_MODE_NONE);
		break;
	case SHADER_ID_SKYBOX:
		colorBlendInfo    = this->initColorBlending(colorBlendAttachment, VK_FALSE);
		depthStencilInfo  = this->initDepthStencilBuffer(VK_TRUE, VK_COMPARE_OP_LESS_OR_EQUAL);
		rasterizationInfo = this->initRasterizer(VK_CULL_MODE_NONE);
		break;
	default:
		colorBlendInfo    = this->initColorBlending(colorBlendAttachment, VK_FALSE);
		depthStencilInfo  = this->initDepthStencilBuffer(VK_TRUE);
		rasterizationInfo = this->initRasterizer();
		break;
	}

	switch (shaderID) {
	case SHADER_ID_WIREFRAME:
		inputAssembly.topology        = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
		rasterizationInfo.polygonMode = VK_POLYGON_MODE_LINE;
		break;
	case SHADER_ID_DEPTH:
	case SHADER_ID_DEPTH_OMNI:
		colorBlendInfo.attachmentCount     = 0;
		rasterizationInfo.depthClampEnable = VK_TRUE;
		rasterizationInfo.cullMode         = VK_CULL_MODE_FRONT_BIT;
		break;
	default:
		break;
	}

	pipelineInfo.pColorBlendState    = &colorBlendInfo;
	pipelineInfo.pDepthStencilState  = &depthStencilInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pRasterizationState = &rasterizationInfo;

	if (vkCreateGraphicsPipelines(this->deviceContext, nullptr, 1, &pipelineInfo, nullptr, pipeline) != VK_SUCCESS)
		return -2;

	return 0;
}

int VKContext::createPipelineLayout(Buffer* buffer)
{
	if (this->deviceContext == nullptr)
		return -1;

	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};

	pipelineLayoutInfo.sType          = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts    = &buffer->Uniform.Layout;

	if (vkCreatePipelineLayout(this->deviceContext, &pipelineLayoutInfo, nullptr, &buffer->Pipeline.Layout) != VK_SUCCESS)
		return -2;

	return 0;
}

int VKContext::CreateShaderModule(const wxString &shaderFile, const wxString &stage, VkShaderModule* shaderModule)
{
	#if defined _DEBUG
		wxArrayString output;
		wxString      glsValidator = wxString(std::getenv("VK_SDK_PATH")).append("/Bin/glslangValidator -V ");
		wxString      command      = wxString(glsValidator + shaderFile + " -S " + stage + " -o " + shaderFile + ".spv");
		long          result       = wxExecute(command, output, wxEXEC_SYNC);

		for (auto line : output)
			wxLogDebug("%s\n", line.c_str().AsChar());

		if (result < 0)
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

int VKContext::CreateTexture(const std::vector<uint8_t*> &imagePixels, Texture* texture, VkFormat imageFormat)
{
	if ((texture == nullptr) || imagePixels.empty())
		return -1;

	VkBufferUsageFlags    bufferUseFlags      = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	VkMemoryPropertyFlags bufferMemFlags      = (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	wxSize                textureSize         = texture->Size();
	int                   colorComponents     = ((imageFormat == VK_FORMAT_R8G8B8A8_UNORM || imageFormat == VK_FORMAT_R8G8B8A8_SRGB) ? 4 : 3);
	VkDeviceSize          imageSize           = (textureSize.GetWidth() * textureSize.GetHeight() * colorComponents);
	VkBuffer              stagingBuffer       = nullptr;
	VkDeviceMemory        stagingBufferMemory = nullptr;

	// STAGING BUFFER
	int result = this->createBuffer(
		(imagePixels.size() * imageSize), bufferUseFlags, bufferMemFlags,
		&stagingBuffer, &stagingBufferMemory
	);

	if (result < 0)
		return -2;

	// COPY IMAGE DATA TO STAGE BUFFER
	void* imageMemData = nullptr;

	for (size_t i = 0; i < imagePixels.size(); i++)
	{
		vkMapMemory(this->deviceContext, stagingBufferMemory, (i * imageSize), imageSize, 0, &imageMemData);
		memcpy(imageMemData, imagePixels[i], (size_t)imageSize);
		vkUnmapMemory(this->deviceContext, stagingBufferMemory);
	}

	// CREATE IMAGE
	VkMemoryPropertyFlags imageUseFlags = (VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
	VkMemoryPropertyFlags imageMemFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	TextureType           textureType   = (imagePixels.size() > 1 ? TEXTURE_CUBEMAP : TEXTURE_2D);
	uint32_t              mipLevels     = (textureType == TEXTURE_CUBEMAP ? 1 : texture->MipLevels());

	result = this->createImage(
		(uint32_t)textureSize.GetWidth(), (uint32_t)textureSize.GetHeight(),
		mipLevels, VK_SAMPLE_COUNT_1_BIT, imageFormat, VK_IMAGE_TILING_OPTIMAL,
		imageUseFlags, imageMemFlags, textureType, &texture->Image, &texture->ImageMemory
	);

	if (result < 0)
		return result;

	for (size_t i = 0; i < imagePixels.size(); i++)
	{
		// TRANSITION IMAGE LAYOUT TO DESTINATION
		if (this->copyImage(texture->Image, imageFormat, mipLevels, textureType, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) < 0)
			return -4;

		// COPY DATA FROM STAGING BUFFER TO IMAGE (DEVICE LOCAL)
		if (this->copyBufferToImage(stagingBuffer, texture->Image, colorComponents, (uint32_t)textureSize.GetWidth(), (uint32_t)textureSize.GetHeight(), i) < 0)
			return -5;

		// TRANSITION IMAGE LAYOUT TO SHADER
		if (textureType == TEXTURE_CUBEMAP)
			result = this->copyImage(texture->Image, imageFormat, mipLevels, textureType, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		else
			result = this->createMipMaps(texture->Image, imageFormat, textureSize.GetWidth(), textureSize.GetHeight(), mipLevels);

		if (result < 0)
			return -6;
	}

	vkFreeMemory(this->deviceContext,    stagingBufferMemory, nullptr);
	vkDestroyBuffer(this->deviceContext, stagingBuffer,       nullptr);

	// SAMPLER
	texture->Sampler = this->createImageSampler(
		(float)mipLevels, (float)this->multiSampleCount, texture->SamplerInfo
	);

	if (texture->Sampler == nullptr)
		return -7;

	// IMAGE VIEW
	VkImageViewType viewType = Utils::ToVkImageViewType(textureType);

	texture->ImageView = this->createImageView(
		texture->Image, imageFormat, VK_IMAGE_ASPECT_COLOR_BIT, mipLevels, viewType
	);

	if (texture->ImageView == nullptr)
		return -8;

	return 0;
}

int VKContext::CreateTextureBuffer(FBOType fboType, VkFormat imageFormat, Texture* texture)
{
	if (texture == nullptr)
		return -1;

	VkImageAspectFlags      imageAspectFlags;
	VkImageLayout           imageLayout;
	VkMemoryPropertyFlags   imageUseFlags   = VK_IMAGE_USAGE_SAMPLED_BIT;
	VkMemoryPropertyFlags   imageMemFlags   = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	VkFramebufferCreateInfo framebufferInfo = {};
	wxSize                  textureSize     = texture->Size();
	TextureType             textureType     = texture->Type();

	switch (fboType) {
	case FBO_COLOR:
		imageAspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
		imageLayout      = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		imageUseFlags   |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		framebufferInfo.renderPass = this->renderPasses[RENDER_PASS_FBO_COLOR];

		break;
	case FBO_DEPTH:
		imageAspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT;
		imageLayout      = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		imageUseFlags   |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

		//imageFormat = this->getDepthBufferFormat();

		framebufferInfo.renderPass = this->renderPasses[RENDER_PASS_FBO_DEPTH];

		break;
	default:
		throw;
	}

	// IMAGE (TEXTURE)
	int result = this->createImage(
		textureSize.GetWidth(), textureSize.GetHeight(), 1, 1, imageFormat, VK_IMAGE_TILING_OPTIMAL,
		imageUseFlags, imageMemFlags, textureType, &texture->Image, &texture->ImageMemory
	);

	if (result < 0)
		return -2;

	// IMAGE (TEXTURE) SAMPLER
	texture->Sampler = this->createImageSampler(1.0f, 1.0f, texture->SamplerInfo);

	if (texture->Sampler == nullptr)
		return -3;

	// IMAGE (TEXTURE) VIEW
	uint32_t        layerCount;
	VkImageViewType viewType = Utils::ToVkImageViewType(textureType);

	switch (textureType) {
		case TEXTURE_2D:            layerCount = 1; break;
		case TEXTURE_CUBEMAP:       layerCount = MAX_TEXTURES; break;
		case TEXTURE_2D_ARRAY:      layerCount = MAX_LIGHT_SOURCES; break;
		case TEXTURE_CUBEMAP_ARRAY: layerCount = (MAX_LIGHT_SOURCES * MAX_TEXTURES); break;
		default: throw;
	}

	texture->ImageView = this->createImageView(
		texture->Image, imageFormat, imageAspectFlags, 1, viewType, layerCount
	);

	if (texture->ImageView == nullptr)
		return -4;

	// DEPTH BUFFER LAYERED VIEWS
	if (fboType == FBO_DEPTH)
	{
		for (uint32_t i = 0; i < MAX_LIGHT_SOURCES; i++)
		{
			uint32_t layer  = (i * (textureType == TEXTURE_CUBEMAP_ARRAY ? MAX_TEXTURES : 1));
			uint32_t layers = (layerCount / MAX_LIGHT_SOURCES);

			texture->DepthViews[i] = this->createImageView(
				texture->Image, imageFormat, imageAspectFlags, 1, viewType, layers, layer
			);

			if (texture->DepthViews[i] == nullptr)
				return -5;
		}
	}

	result = this->copyImage(
		texture->Image, imageFormat, 1, textureType, VK_IMAGE_LAYOUT_UNDEFINED, imageLayout
	);

	if (result < 0)
		return -6;

	// FRAME BUFFER
	framebufferInfo.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferInfo.attachmentCount = 1;
	framebufferInfo.layers          = 1;
	framebufferInfo.pAttachments    = &texture->ImageView;
	framebufferInfo.width           = textureSize.GetWidth();
	framebufferInfo.height          = textureSize.GetHeight();

	VkResult resultVK = vkCreateFramebuffer(
		this->deviceContext, &framebufferInfo, nullptr, &texture->BufferVK
	);

	if (resultVK != VK_SUCCESS)
		return -7;

	// DEPTH BUFFER LAYERED FRAMEBUFFERS
	if (fboType == FBO_DEPTH)
	{
		for (uint32_t i = 0; i < MAX_LIGHT_SOURCES; i++)
		{
			framebufferInfo.pAttachments = &texture->DepthViews[i];

			resultVK = vkCreateFramebuffer(
				this->deviceContext, &framebufferInfo, nullptr, &texture->DepthBuffers[i]
			);

			if (resultVK != VK_SUCCESS)
				return -8;
		}
	}

	return 0;
}

int VKContext::createUniformBuffers(Buffer* buffer)
{
	VkBufferUsageFlags    bufferUseFlags = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	VkMemoryPropertyFlags bufferMemFlags = (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	if (this->createBuffer(sizeof(CBMatrix), bufferUseFlags, bufferMemFlags, &buffer->Uniform.Buffers[UBO_VK_MATRIX], &buffer->Uniform.BufferMemories[UBO_VK_MATRIX]) < 0)
		return -1;

	if (this->createBuffer(sizeof(CBColor), bufferUseFlags, bufferMemFlags, &buffer->Uniform.Buffers[UBO_VK_COLOR], &buffer->Uniform.BufferMemories[UBO_VK_COLOR]) < 0)
		return -2;

	if (this->createBuffer(sizeof(CBDefault), bufferUseFlags, bufferMemFlags, &buffer->Uniform.Buffers[UBO_VK_DEFAULT], &buffer->Uniform.BufferMemories[UBO_VK_DEFAULT]) < 0)
		return -3;

	if (this->createBuffer(sizeof(CBDepth), bufferUseFlags, bufferMemFlags, &buffer->Uniform.Buffers[UBO_VK_DEPTH], &buffer->Uniform.BufferMemories[UBO_VK_DEPTH]) < 0)
		return -4;

	if (this->createBuffer(sizeof(CBHUD), bufferUseFlags, bufferMemFlags, &buffer->Uniform.Buffers[UBO_VK_HUD], &buffer->Uniform.BufferMemories[UBO_VK_HUD]) < 0)
		return -5;

	if (this->createBuffer(sizeof(CBSkybox), bufferUseFlags, bufferMemFlags, &buffer->Uniform.Buffers[UBO_VK_SKYBOX], &buffer->Uniform.BufferMemories[UBO_VK_SKYBOX]) < 0)
		return -6;

	return 0;
}

int VKContext::createUniformLayout(VkDescriptorSetLayout* uniformLayout)
{
	if (this->deviceContext == nullptr)
		return -1;

	VkDescriptorSetLayoutBinding uniformLayoutBindings[NR_OF_UBO_BINDINGS] = {};

	// MATRIX BUFFER
	uniformLayoutBindings[UBO_BINDING_MATRIX].descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uniformLayoutBindings[UBO_BINDING_MATRIX].descriptorCount = 1;
	uniformLayoutBindings[UBO_BINDING_MATRIX].binding         = UBO_BINDING_MATRIX;
	uniformLayoutBindings[UBO_BINDING_MATRIX].stageFlags      = (VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT);

	// DEFAULT BUFFER
	uniformLayoutBindings[UBO_BINDING_DEFAULT].descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uniformLayoutBindings[UBO_BINDING_DEFAULT].descriptorCount = 1;
	uniformLayoutBindings[UBO_BINDING_DEFAULT].binding         = UBO_BINDING_DEFAULT;
	uniformLayoutBindings[UBO_BINDING_DEFAULT].stageFlags      = (VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_GEOMETRY_BIT);

	// TEXTURE SAMPLER
	uniformLayoutBindings[UBO_BINDING_TEXTURES].descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	uniformLayoutBindings[UBO_BINDING_TEXTURES].descriptorCount = MAX_TEXTURES;
	uniformLayoutBindings[UBO_BINDING_TEXTURES].binding         = UBO_BINDING_TEXTURES;
	uniformLayoutBindings[UBO_BINDING_TEXTURES].stageFlags      = VK_SHADER_STAGE_FRAGMENT_BIT;

	// DEPTH SAMPLER - 2D
	uniformLayoutBindings[UBO_BINDING_DEPTH_2D].descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	uniformLayoutBindings[UBO_BINDING_DEPTH_2D].descriptorCount = 1;
	uniformLayoutBindings[UBO_BINDING_DEPTH_2D].binding         = UBO_BINDING_DEPTH_2D;
	uniformLayoutBindings[UBO_BINDING_DEPTH_2D].stageFlags      = VK_SHADER_STAGE_FRAGMENT_BIT;

	// DEPTH SAMPLER - CUBE MAP
	uniformLayoutBindings[UBO_BINDING_DEPTH_CUBEMAPS].descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	uniformLayoutBindings[UBO_BINDING_DEPTH_CUBEMAPS].descriptorCount = 1;
	uniformLayoutBindings[UBO_BINDING_DEPTH_CUBEMAPS].binding         = UBO_BINDING_DEPTH_CUBEMAPS;
	uniformLayoutBindings[UBO_BINDING_DEPTH_CUBEMAPS].stageFlags      = VK_SHADER_STAGE_FRAGMENT_BIT;

	VkDescriptorSetLayoutCreateInfo uniformLayoutInfo = {};

	uniformLayoutInfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	uniformLayoutInfo.bindingCount = NR_OF_UBO_BINDINGS;
	uniformLayoutInfo.pBindings    = uniformLayoutBindings;

	if (vkCreateDescriptorSetLayout(this->deviceContext, &uniformLayoutInfo, nullptr, uniformLayout) != VK_SUCCESS)
		return -2;

	return 0;
}

int VKContext::createUniformPool(VkDescriptorPool* uniformPool)
{
	if (this->deviceContext == nullptr)
		return -1;

	VkDescriptorPoolCreateInfo uniformPoolInfo = {};
	VkDescriptorPoolSize       uniformPoolSizes[NR_OF_UBO_BINDINGS - 1] = {};

	uniformPoolSizes[UBO_BINDING_DEFAULT - 1].type            = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uniformPoolSizes[UBO_BINDING_DEFAULT - 1].descriptorCount = 2; // matrix + default

	uniformPoolSizes[UBO_BINDING_TEXTURES - 1].type            = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	uniformPoolSizes[UBO_BINDING_TEXTURES - 1].descriptorCount = MAX_TEXTURES;

	uniformPoolSizes[UBO_BINDING_DEPTH_2D - 1].type            = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	uniformPoolSizes[UBO_BINDING_DEPTH_2D - 1].descriptorCount = 1;

	uniformPoolSizes[UBO_BINDING_DEPTH_CUBEMAPS - 1].type            = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	uniformPoolSizes[UBO_BINDING_DEPTH_CUBEMAPS - 1].descriptorCount = 1;

	uniformPoolInfo.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	uniformPoolInfo.poolSizeCount = NR_OF_UBO_BINDINGS - 1;
	uniformPoolInfo.pPoolSizes    = uniformPoolSizes;
	uniformPoolInfo.maxSets       = 1;	// maximum number of descriptor sets that can be allocated from the pool

	if (vkCreateDescriptorPool(this->deviceContext, &uniformPoolInfo, nullptr, uniformPool) != VK_SUCCESS)
		return -2;

	return 0;
}

int VKContext::createUniformSet(Buffer* buffer)
{
	if (this->deviceContext == nullptr)
		return -1;

	if (this->createUniformLayout(&buffer->Uniform.Layout) < 0)
		return false;

	if (this->createUniformPool(&buffer->Uniform.Pool) < 0)
		return false;

	VkDescriptorSetAllocateInfo uniformSetAllocInfo = {};

	uniformSetAllocInfo.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	uniformSetAllocInfo.descriptorSetCount = 1;
	uniformSetAllocInfo.pSetLayouts        = &buffer->Uniform.Layout;
	uniformSetAllocInfo.descriptorPool     = buffer->Uniform.Pool;

	if (vkAllocateDescriptorSets(this->deviceContext, &uniformSetAllocInfo, &buffer->Uniform.Set) != VK_SUCCESS)
		return -2;

	return 0;
}

int VKContext::CreateVertexBuffer(const std::vector<float> &vertices, const std::vector<float> &normals, const std::vector<float> &texCoords, Buffer* buffer)
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
	if (this->createBuffer(vertexBufferSize, vertexBufferUseFlags, vertexBufferMemFlags, &buffer->VertexBuffer, &buffer->VertexBufferMemory) < 0)
		return -2;

	// COPY DATA FROM STAGING TO VERTEX BUFFER (DEVICE LOCAL)
	if (this->copyBuffer(stagingBuffer, buffer->VertexBuffer, vertexBufferSize) < 0)
		return -3;

	vkFreeMemory(this->deviceContext,    stagingBufferMemory, nullptr);
	vkDestroyBuffer(this->deviceContext, stagingBuffer,       nullptr);

	return 0;
}

#if defined _DEBUG
VKAPI_ATTR VkBool32 VKAPI_CALL VKContext::debugLog(VkDebugReportFlagsEXT f, VkDebugReportObjectTypeEXT ot, uint64_t o, size_t l, int32_t c, const char* lp, const char* m, void* ud)
{
	wxLogDebug("%s\n", m);
	return VK_FALSE;
}
#endif

void VKContext::DestroyBuffer(VkBuffer* buffer, VkDeviceMemory* bufferMemory)
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

void VKContext::DestroyFramebuffer(VkFramebuffer* frameBuffer)
{
	if (*frameBuffer != nullptr) {
		vkDestroyFramebuffer(this->deviceContext, *frameBuffer, nullptr);
		*frameBuffer = nullptr;
	}
}

void VKContext::DestroyPipeline(VkPipeline* pipeline)
{
	if (*pipeline != nullptr) {
		vkDestroyPipeline(this->deviceContext, *pipeline, nullptr);
		*pipeline = nullptr;
	}
}

void VKContext::DestroyPipelineLayout(VkPipelineLayout* pipelineLayout)
{
	if (*pipelineLayout != nullptr) {
		vkDestroyPipelineLayout(this->deviceContext, *pipelineLayout, nullptr);
		*pipelineLayout = nullptr;
	}
}

void VKContext::DestroyShaderModule(VkShaderModule* shaderModule)
{
	if (*shaderModule != nullptr) {
		vkDestroyShaderModule(this->deviceContext, *shaderModule, nullptr);
		*shaderModule = nullptr;
	}
}

void VKContext::DestroyTexture(VkImage* image, VkDeviceMemory* imageMemory, VkImageView* textureImageView, VkSampler* sampler)
{
	if ((sampler != nullptr) && (*sampler != nullptr)) {
		vkDestroySampler(this->deviceContext, *sampler, nullptr);
		*sampler = nullptr;
	}

	if ((textureImageView != nullptr) && (*textureImageView != nullptr)) {
		vkDestroyImageView(this->deviceContext, *textureImageView, nullptr);
		*textureImageView = nullptr;
	}

	if ((imageMemory != nullptr) && (*imageMemory != nullptr)) {
		vkFreeMemory(this->deviceContext, *imageMemory, nullptr);
		*imageMemory = nullptr;
	}

	if ((image != nullptr) && (*image != nullptr)) {
		vkDestroyImage(this->deviceContext, *image, nullptr);
		*image = nullptr;
	}
}

void VKContext::DestroyUniformSet(VkDescriptorPool* uniformPool, VkDescriptorSetLayout* uniformLayout)
{
	if ((uniformPool != nullptr) && (*uniformPool != nullptr)) {
		vkDestroyDescriptorPool(this->deviceContext, *uniformPool, nullptr);
		*uniformPool = nullptr;
	}

	if ((uniformLayout != nullptr) && (*uniformLayout != nullptr)) {
		vkDestroyDescriptorSetLayout(this->deviceContext, *uniformLayout, nullptr);
		*uniformLayout = nullptr;
	}
}

bool VKContext::deviceSupportsExtensions(VkPhysicalDevice device, const std::vector<const char*> &extensions)
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

bool VKContext::deviceSupportsFeatures(VkPhysicalDevice device, const VkPhysicalDeviceFeatures &features)
{
	VkPhysicalDeviceFeatures deviceFeatures = {};
	bool                     supported      = true;

	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

	if (features.samplerAnisotropy)
		supported = deviceFeatures.samplerAnisotropy;

	return supported;
}

int VKContext::Draw(Component* mesh, ShaderProgram* shaderProgram, const DrawProperties &properties)
{
	if ((RenderEngine::CameraMain == nullptr) || (mesh == nullptr) || (shaderProgram == nullptr))
		return -1;

	Buffer*  indexBuffer     = dynamic_cast<Mesh*>(mesh)->IndexBuffer();
	Buffer*  vertexBuffer    = dynamic_cast<Mesh*>(mesh)->VertexBuffer();
	ShaderID shaderID        = shaderProgram->ID();
	VkBuffer vertexBuffers[] = { vertexBuffer->VertexBuffer };

	if ((vertexBuffer == nullptr) || (shaderID == SHADER_ID_UNKNOWN))
		return -2;

	// UPDATE UNIFORM VALUES
	if (shaderProgram->UpdateUniformsVK(this->deviceContext, mesh, vertexBuffer->Uniform, properties) < 0)
		return -3;

	VkPipeline      pipeline;
	VkCommandBuffer cmdBuffer = (properties.VKCommandBuffer != nullptr ? properties.VKCommandBuffer : this->commandBuffers[this->imageIndex]);

	// BIND SHADER TO PIPELINE
	if (properties.FBO != nullptr)
		pipeline = vertexBuffer->Pipeline.PipelinesFBO[shaderID];
	else
		pipeline = vertexBuffer->Pipeline.Pipelines[shaderID];

	vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

	// BIND INDEX AND VERTEX BUFFERS
	if (indexBuffer != nullptr)
		vkCmdBindIndexBuffer(cmdBuffer, indexBuffer->IndexBuffer, 0, VK_INDEX_TYPE_UINT32);

	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(cmdBuffer, 0, 1, vertexBuffers, offsets);
	
	// TODO: SLOW
	// BIND UNIFORMS
	vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vertexBuffer->Pipeline.Layout, 0, 1, &vertexBuffer->Uniform.Set, 0, nullptr);

	// DRAW
	if (indexBuffer != nullptr)
		vkCmdDrawIndexed(cmdBuffer, dynamic_cast<Mesh*>(mesh)->NrOfIndices(), 1, 0, 0, 0);
	else
		vkCmdDraw(cmdBuffer, dynamic_cast<Mesh*>(mesh)->NrOfVertices(), 1, 0, 0);

	return 0;
}

wxString VKContext::getApiVersion(VkPhysicalDevice device)
{
	if (device == nullptr)
		return "";

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

/*VkFormat VKContext::getDepthBufferFormat()
{
	std::vector<VkFormat> formats     = { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT };
	VkImageTiling         imageTiling = VK_IMAGE_TILING_OPTIMAL;
	VkFormatFeatureFlags  features    = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;

	return this->getImageFormat(formats, imageTiling, features);
}*/

VkPhysicalDevice VKContext::getDevice(const std::vector<const char*> &extensions, const VkPhysicalDeviceFeatures &features)
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

		// SWAP CHAIN SUPPORT
		this->swapChainSupport = this->getDeviceSwapChainSupport(device);

		if (this->swapChainSupport->PresentModes.empty() || this->swapChainSupport->SurfaceFormats.empty())
			continue;

		// QUEUE SUPPORT
		this->queues = this->getDeviceQueueSupport(device);

		if ((this->queues[VK_QUEUE_GRAPHICS]->Index < 0) || (this->queues[VK_QUEUE_PRESENTATION]->Index < 0))
			continue;

		if (!this->deviceSupportsExtensions(device, extensions) || !this->deviceSupportsFeatures(device, features))
			continue;

		physicalDevice = device;

		// VSYNC SUPPORT
		if (std::find(this->swapChainSupport->PresentModes.begin(), this->swapChainSupport->PresentModes.end(), VK_PRESENT_MODE_IMMEDIATE_KHR) == this->swapChainSupport->PresentModes.end())
			continue;

		break;
	}

	return physicalDevice;
}

wxString VKContext::getDeviceName(VkPhysicalDevice device)
{
	if (device == nullptr)
		return "";

	VkPhysicalDeviceProperties properties = {};
	vkGetPhysicalDeviceProperties(device, &properties);

	wxString vendor;

	switch (properties.vendorID) {
		case 0x1002: vendor = "AMD";    break;
		case 0x10DE: vendor = "NVIDIA"; break;
		case 0x8086: vendor = "Intel";  break;
		default:     vendor = "";       break;
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

std::vector<VKQueue*> VKContext::getDeviceQueueSupport(VkPhysicalDevice device)
{
	std::vector<VKQueue*> queueSupport(NR_OF_VK_QUEUES);

	uint32_t queueCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueCount, nullptr);

	std::vector<VkQueueFamilyProperties> queues(queueCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueCount, queues.data());

	for (int32_t i = 0; i < (int32_t)queueCount; i++)
	{
		if (queues[i].queueCount == 0)
			continue;

		if ((queues[0].queueFlags & VK_QUEUE_GRAPHICS_BIT) && (queueSupport[VK_QUEUE_GRAPHICS] == nullptr))
			queueSupport[VK_QUEUE_GRAPHICS] = new VKQueue(i);

		VkBool32 supportsPresentation = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, this->surface, &supportsPresentation);

		if (supportsPresentation && (queueSupport[VK_QUEUE_PRESENTATION] == nullptr))
			queueSupport[VK_QUEUE_PRESENTATION] = new VKQueue(i);

		if ((queueSupport[VK_QUEUE_PRESENTATION] != nullptr) && (queueSupport[VK_QUEUE_GRAPHICS] != nullptr))
			break;
	}

	return queueSupport;
}

VKSwapChainSupport* VKContext::getDeviceSwapChainSupport(VkPhysicalDevice device)
{
	VKSwapChainSupport* swapChainSupport = new VKSwapChainSupport();

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

VkFormat VKContext::getImageFormat(const std::vector<VkFormat> &formats, VkImageTiling imageTiling, VkFormatFeatureFlags features)
{
	VkFormatProperties properties = {};

	for (VkFormat format : formats)
	{
		vkGetPhysicalDeviceFormatProperties(this->device, format, &properties);

		if (((imageTiling == VK_IMAGE_TILING_LINEAR)  && ((properties.linearTilingFeatures  & features) == features)) ||
			((imageTiling == VK_IMAGE_TILING_OPTIMAL) && ((properties.optimalTilingFeatures & features) == features)))
		{
			return format;
		}
	}

	return VK_FORMAT_UNDEFINED;
}

uint32_t VKContext::getMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memoryProperties;
	vkGetPhysicalDeviceMemoryProperties(this->device, &memoryProperties);

	for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
		if ((typeFilter & (1 << i)) && ((memoryProperties.memoryTypes[i].propertyFlags & properties) == properties))
			return i;
	}

	return -1;
}

uint32_t VKContext::getMultiSampleCount()
{
	uint32_t sampleCount = VK_SAMPLE_COUNT_1_BIT;

	if (this->device == nullptr)
		return sampleCount;

	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(this->device, &deviceProperties);

	VkSampleCountFlags sampleCountFlags = std::min(
		deviceProperties.limits.framebufferColorSampleCounts, deviceProperties.limits.framebufferDepthSampleCounts
	);

	if (sampleCountFlags & VK_SAMPLE_COUNT_64_BIT)
		sampleCount = VK_SAMPLE_COUNT_64_BIT;
	else if (sampleCountFlags & VK_SAMPLE_COUNT_32_BIT)
		sampleCount = VK_SAMPLE_COUNT_32_BIT;
	else if (sampleCountFlags & VK_SAMPLE_COUNT_16_BIT)
		sampleCount = VK_SAMPLE_COUNT_16_BIT;
	else if (sampleCountFlags & VK_SAMPLE_COUNT_8_BIT)
		sampleCount = VK_SAMPLE_COUNT_8_BIT;
	else if (sampleCountFlags & VK_SAMPLE_COUNT_4_BIT)
		sampleCount = VK_SAMPLE_COUNT_4_BIT;
	else if (sampleCountFlags & VK_SAMPLE_COUNT_2_BIT)
		sampleCount = VK_SAMPLE_COUNT_2_BIT;

	return sampleCount;
}

VkPresentModeKHR VKContext::getPresentationMode(const std::vector<VkPresentModeKHR> &presentationModes)
{
	VkPresentModeKHR presentMode = (this->vSync ? VK_PRESENT_MODE_MAILBOX_KHR : VK_PRESENT_MODE_IMMEDIATE_KHR);

	if (std::find(presentationModes.begin(), presentationModes.end(), presentMode) != presentationModes.end())
		return presentMode;

	return VK_PRESENT_MODE_FIFO_KHR;
}

VkSurfaceFormatKHR VKContext::getSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &surfaceFormats)
{
	VkSurfaceFormatKHR surfaceFormat = {};

	surfaceFormat.format     = VK_FORMAT_B8G8R8A8_UNORM;
	surfaceFormat.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

	for (auto format : surfaceFormats) {
		if ((format.format == VK_FORMAT_UNDEFINED) || ((format.format == surfaceFormat.format) && (format.colorSpace == surfaceFormat.colorSpace)))
			return surfaceFormat;
	}

	if (!surfaceFormats.empty()) {
		surfaceFormat.format     = surfaceFormats[0].format;
		surfaceFormat.colorSpace = surfaceFormats[0].colorSpace;
	}

	return surfaceFormat;
}

VkExtent2D VKContext::getSurfaceSize(const VkSurfaceCapabilitiesKHR &capabilities)
{
	// Use the currently selected size if already detected
	if (capabilities.currentExtent.width != UINT32_MAX)
		return VkExtent2D(capabilities.currentExtent);

	// Otherwise try to match the size as much as possible to the canvas size
	VkExtent2D size = {};

	uint32_t width     = (uint32_t)RenderEngine::Canvas.Size.GetWidth();
	uint32_t height    = (uint32_t)RenderEngine::Canvas.Size.GetHeight();
	uint32_t minWidth  = capabilities.minImageExtent.width;
	uint32_t minHeight = capabilities.minImageExtent.height;
	uint32_t maxWidth  = capabilities.maxImageExtent.width;
	uint32_t maxHeight = capabilities.maxImageExtent.height;

	size.width  = std::max(minWidth,  std::min(maxWidth,  width));
	size.height = std::max(minHeight, std::min(maxHeight, height));

	return size;
}

VkPipelineColorBlendStateCreateInfo VKContext::initColorBlending(VkPipelineColorBlendAttachmentState &attachment, VkBool32 enableBlending)
{
	VkPipelineColorBlendStateCreateInfo colorBlendInfo = {};

	attachment.blendEnable         = enableBlending;
	attachment.colorWriteMask      = (VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT);
	attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;

	colorBlendInfo.sType           = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlendInfo.logicOp         = VK_LOGIC_OP_COPY;
	colorBlendInfo.attachmentCount = 1;
	colorBlendInfo.pAttachments    = &attachment;

	return colorBlendInfo;
}

int VKContext::initColorImages()
{
	if (this->swapChain == nullptr)
		return -1;

	VkFormat colorFormat = this->swapChain->SurfaceFormat.format;

	const size_t NR_OF_SWAP_CHAINS = this->swapChain->Images.size();

	this->colorImages.resize(NR_OF_SWAP_CHAINS);
	this->colorImageMemories.resize(NR_OF_SWAP_CHAINS);
	this->colorImageViews.resize(NR_OF_SWAP_CHAINS);

	for (size_t i = 0; i < NR_OF_SWAP_CHAINS; i++)
	{
		int result = this->createImage(
			this->swapChain->Size.width, this->swapChain->Size.height,
			1, this->multiSampleCount,
			colorFormat, VK_IMAGE_TILING_OPTIMAL,
			(VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT),
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			TEXTURE_2D,
			&this->colorImages[i], &this->colorImageMemories[i]
		);

		if (result < 0)
			return -2;

		this->colorImageViews[i] = this->createImageView(
			this->colorImages[i], colorFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1, VK_IMAGE_VIEW_TYPE_2D
		);

		result = this->copyImage(
			this->colorImages[i], colorFormat, 1, TEXTURE_2D,
			VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
		);

		if (result < 0)
			return -3;
	}

	return 0;
}

std::vector<VkCommandBuffer> VKContext::initCommandBuffers(uint32_t bufferCount)
{
	// Command buffer allocation
	VkCommandBufferAllocateInfo  allocateInfo   = {};
	std::vector<VkCommandBuffer> commandBuffers = std::vector<VkCommandBuffer>(bufferCount);

	allocateInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocateInfo.commandBufferCount = (uint32_t)commandBuffers.size();
	allocateInfo.commandPool        = this->commandPool;

	if (vkAllocateCommandBuffers(this->deviceContext, &allocateInfo, commandBuffers.data()) != VK_SUCCESS)
		commandBuffers.clear();

	return commandBuffers;
}

VkCommandPool VKContext::initCommandPool()
{
	VkCommandPoolCreateInfo commandPoolInfo = {};
	VkCommandPool           commandPool     = nullptr;

	commandPoolInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolInfo.queueFamilyIndex = this->queues[VK_QUEUE_GRAPHICS]->Index;
	commandPoolInfo.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	if (vkCreateCommandPool(this->deviceContext, &commandPoolInfo, nullptr, &commandPool) != VK_SUCCESS)
		return nullptr;

	return commandPool;
}

VkPipelineDepthStencilStateCreateInfo VKContext::initDepthStencilBuffer(VkBool32 enableDepth, VkCompareOp compareOperation)
{
	VkPipelineDepthStencilStateCreateInfo depthStencilInfo = {};

	depthStencilInfo.sType            = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencilInfo.depthTestEnable  = enableDepth;
	depthStencilInfo.depthWriteEnable = enableDepth;
	depthStencilInfo.depthCompareOp   = compareOperation;

	return depthStencilInfo;
}

int VKContext::initDepthStencilImages()
{
	//VkFormat depthFormat = this->getDepthBufferFormat();
	VkFormat depthFormat = VK_FORMAT_D16_UNORM;

	//if ((depthFormat == VK_FORMAT_UNDEFINED) || (this->swapChain == nullptr) || this->swapChain->Images.empty())
	if ((this->swapChain == nullptr) || this->swapChain->Images.empty())
		return -1;

	const size_t NR_OF_SWAP_CHAINS = this->swapChain->Images.size();

	this->depthBufferImages.resize(NR_OF_SWAP_CHAINS);
	this->depthBufferImageMemories.resize(NR_OF_SWAP_CHAINS);
	this->depthBufferImageViews.resize(NR_OF_SWAP_CHAINS);

	for (size_t i = 0; i < NR_OF_SWAP_CHAINS; i++)
	{
		int result = this->createImage(
			this->swapChain->Size.width, this->swapChain->Size.height,
			1, this->multiSampleCount,
			depthFormat, VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			TEXTURE_2D,
			&this->depthBufferImages[i], &this->depthBufferImageMemories[i]
		);

		if (result < 0)
			return -2;

		this->depthBufferImageViews[i] = this->createImageView(
			this->depthBufferImages[i], depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1, VK_IMAGE_VIEW_TYPE_2D
		);

		result = this->copyImage(
			this->depthBufferImages[i], depthFormat, 1, TEXTURE_2D,
			VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
		);

		if (result < 0)
			return -3;
	}

	return 0;
}

VkDevice VKContext::initDeviceContext()
{
	if ((this->instance == nullptr) || (this->surface == nullptr))
		return nullptr;

	std::vector<const char*> extensions = { "VK_KHR_swapchain" };
	VkPhysicalDeviceFeatures features   = {};

	features.depthClamp        = VK_TRUE;
	features.fillModeNonSolid  = VK_TRUE;
	features.samplerAnisotropy = VK_TRUE;
	features.imageCubeArray    = VK_TRUE;
	features.geometryShader    = VK_TRUE;

	this->device = this->getDevice(extensions, features);

	if (this->device == nullptr)
		return nullptr;

	this->multiSampleCount = this->getMultiSampleCount();	// MSAA

	std::vector<VkDeviceQueueCreateInfo> queueInfos;
	float                                queuePriority[] = { 1.0f };

	// Get the unique queue indices for graphics and presentation (they may be the same)
	std::set<uint32_t> uniqueQueueIndices = {
		(uint32_t)this->queues[VK_QUEUE_PRESENTATION]->Index, (uint32_t)this->queues[VK_QUEUE_GRAPHICS]->Index
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

	deviceInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceInfo.queueCreateInfoCount    = queueInfos.size();
	deviceInfo.pQueueCreateInfos       = queueInfos.data();
	deviceInfo.pEnabledFeatures        = &features;
	deviceInfo.ppEnabledExtensionNames = extensions.data();
	deviceInfo.enabledExtensionCount   = extensions.size();

	if (vkCreateDevice(this->device, &deviceInfo, nullptr, &deviceContext) != VK_SUCCESS)
		return nullptr;

	// Create the graphics and presentation queues
	vkGetDeviceQueue(deviceContext, this->queues[VK_QUEUE_GRAPHICS]->Index,     0, &this->queues[VK_QUEUE_GRAPHICS]->Queue);
	vkGetDeviceQueue(deviceContext, this->queues[VK_QUEUE_PRESENTATION]->Index, 0, &this->queues[VK_QUEUE_PRESENTATION]->Queue);

	return deviceContext;
}

std::vector<VkFramebuffer> VKContext::initFramebuffers()
{
	const int                  NR_OF_ATTACHMENTS = 3;
	const int                  NR_OF_SWAP_CHAINS = this->swapChain->ImageViews.size();
	std::vector<VkFramebuffer> frameBuffers      = std::vector<VkFramebuffer>(NR_OF_SWAP_CHAINS);

	for (int i = 0; i < NR_OF_SWAP_CHAINS; i++)
	{
		VkImageView attachments[NR_OF_ATTACHMENTS] = {
			this->colorImageViews[i], this->depthBufferImageViews[i], this->swapChain->ImageViews[i]
		};

		VkFramebufferCreateInfo framebufferInfo = {};

		framebufferInfo.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.attachmentCount = NR_OF_ATTACHMENTS;
		framebufferInfo.pAttachments    = attachments;
		framebufferInfo.width           = this->swapChain->Size.width;
		framebufferInfo.height          = this->swapChain->Size.height;
		framebufferInfo.layers          = 1;
		framebufferInfo.renderPass      = this->renderPasses[RENDER_PASS_DEFAULT];

		if (vkCreateFramebuffer(this->deviceContext, &framebufferInfo, nullptr, &frameBuffers[i]) != VK_SUCCESS)
			continue;

	}

	return frameBuffers;
}
VkInstance VKContext::initInstance()
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
		debugInfo.pfnCallback = VKContext::debugLog;

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

VkPipelineMultisampleStateCreateInfo VKContext::initMultisampling(uint32_t sampleCount)
{
	VkPipelineMultisampleStateCreateInfo multisampleInfo = {};

	multisampleInfo.sType                = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampleInfo.rasterizationSamples = (VkSampleCountFlagBits)sampleCount;		// MSAA

	return multisampleInfo;
}

int VKContext::InitPipelines(Buffer* buffer)
{
	// SHADER ATTRIBS
	VkVertexInputBindingDescription                attribsBindingDesc = {};
	std::vector<VkVertexInputAttributeDescription> attribsDescs       = {};

	uint32_t offset = 0;

	// NORMALS
	if (buffer->Normals() > 0)
	{
		VkVertexInputAttributeDescription attribsDesc = {};
		
		attribsDesc.location = ATTRIB_NORMAL;
		attribsDesc.format   = VK_FORMAT_R32G32B32_SFLOAT;
		attribsDesc.offset   = offset;

		attribsDescs.push_back(attribsDesc);

		offset += (3 * sizeof(float));
	}

	// POSITIONS
	if (buffer->Vertices() > 0)
	{
		VkVertexInputAttributeDescription attribsDesc = {};

		attribsDesc.location = ATTRIB_POSITION;
		attribsDesc.format   = VK_FORMAT_R32G32B32_SFLOAT;
		attribsDesc.offset   = offset;

		attribsDescs.push_back(attribsDesc);

		offset += (3 * sizeof(float));
	}

	// TEXTURE COORDINATES
	if (buffer->TexCoords() > 0)
	{
		VkVertexInputAttributeDescription attribsDesc = {};

		attribsDesc.location = ATTRIB_TEXCOORDS;
		attribsDesc.format   = VK_FORMAT_R32G32_SFLOAT;
		attribsDesc.offset   = offset;

		attribsDescs.push_back(attribsDesc);

		offset += (2 * sizeof(float));
	}

	attribsBindingDesc.stride    = offset;
	attribsBindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	this->createUniformSet(buffer);
	this->createUniformBuffers(buffer);

	if (this->createPipelineLayout(buffer) < 0)
		return -2;

	// RENDER PIPELINES
	for (int i = 0; i < NR_OF_SHADERS; i++)
	{
		if (this->createPipeline(ShaderManager::Programs[i], &buffer->Pipeline.Pipelines[i], buffer->Pipeline.Layout, FBO_UNKNOWN, attribsDescs, attribsBindingDesc) < 0)
			return -3;

		if (this->createPipeline(ShaderManager::Programs[i], &buffer->Pipeline.PipelinesFBO[i], buffer->Pipeline.Layout, FBO_COLOR, attribsDescs, attribsBindingDesc) < 0)
			return -4;
	}

	return 0;
}

VkPipelineRasterizationStateCreateInfo VKContext::initRasterizer(VkCullModeFlags cullMode, VkPolygonMode polyMode)
{
	VkPipelineRasterizationStateCreateInfo rasterizationInfo = {};

	rasterizationInfo.sType       = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizationInfo.lineWidth   = 1.0f;
	rasterizationInfo.cullMode    = cullMode;
	rasterizationInfo.frontFace   = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizationInfo.polygonMode = polyMode;

	return rasterizationInfo;
}

VkRenderPass VKContext::initRenderPass(VkFormat format, uint32_t sampleCount, VKAttachmentDesc attachmentDesc)
{
	VkSubpassDescription                 subpass           = {};
	VkSubpassDependency                  subPassDependency = {};
	const size_t                         ATTACHMENT_COUNT  = (attachmentDesc == NR_OF_VK_ATTACHMENTS ? NR_OF_VK_ATTACHMENTS : 1);
	const VkSampleCountFlagBits          SAMPLE_COUNT      = (attachmentDesc == NR_OF_VK_ATTACHMENTS ? (VkSampleCountFlagBits)sampleCount : VK_SAMPLE_COUNT_1_BIT);
	int                                  attachmentIndex;
	std::vector<VkAttachmentDescription> attachments(ATTACHMENT_COUNT);
	std::vector<VkAttachmentReference>   attachmentRefs(ATTACHMENT_COUNT);

	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

	// COLOR BUFFER
	if ((attachmentDesc == VK_COLOR_ATTACHMENT) || (attachmentDesc == NR_OF_VK_ATTACHMENTS))
	{
		attachmentIndex = (attachmentDesc == NR_OF_VK_ATTACHMENTS ? VK_COLOR_ATTACHMENT : 0);

		attachments[attachmentIndex]                = {};
		attachments[attachmentIndex].format         = format;
		attachments[attachmentIndex].samples        = SAMPLE_COUNT;
		attachments[attachmentIndex].loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachments[attachmentIndex].storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[attachmentIndex].stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[attachmentIndex].stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[attachmentIndex].initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;

		if (attachmentDesc == VK_COLOR_ATTACHMENT)
			attachments[attachmentIndex].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		else
			attachments[attachmentIndex].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		attachmentRefs[attachmentIndex].attachment = attachmentIndex;
		attachmentRefs[attachmentIndex].layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments    = &attachmentRefs[attachmentIndex];

		subPassDependency.srcSubpass    = VK_SUBPASS_EXTERNAL;
		subPassDependency.srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		subPassDependency.dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		subPassDependency.dstAccessMask = (VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT);
	}

	// DEPTH BUFFER
	if ((attachmentDesc == VK_DEPTH_STENCIL_ATTACHMENT) || (attachmentDesc == NR_OF_VK_ATTACHMENTS))
	{
		attachmentIndex = (attachmentDesc == NR_OF_VK_ATTACHMENTS ? VK_DEPTH_STENCIL_ATTACHMENT : 0);

		attachments[attachmentIndex]                = {};
		//attachments[attachmentIndex].format         = this->getDepthBufferFormat();
		attachments[attachmentIndex].format         = VK_FORMAT_D16_UNORM;
		attachments[attachmentIndex].samples        = SAMPLE_COUNT;
		attachments[attachmentIndex].loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachments[attachmentIndex].storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[attachmentIndex].stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[attachmentIndex].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[attachmentIndex].initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
		attachments[attachmentIndex].finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		attachmentRefs[attachmentIndex].attachment = attachmentIndex;
		attachmentRefs[attachmentIndex].layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		subpass.pDepthStencilAttachment = &attachmentRefs[attachmentIndex];
	}

	// MSAA
	if ((attachmentDesc == VK_RESOLVE_ATTACHMENT) || (attachmentDesc == NR_OF_VK_ATTACHMENTS))
	{
		attachmentIndex = VK_RESOLVE_ATTACHMENT;

		attachments[attachmentIndex]                = {};
		attachments[attachmentIndex].format         = attachments[VK_COLOR_ATTACHMENT].format;
		attachments[attachmentIndex].samples        = VK_SAMPLE_COUNT_1_BIT;
		attachments[attachmentIndex].loadOp         = attachments[VK_COLOR_ATTACHMENT].stencilLoadOp;
		attachments[attachmentIndex].storeOp        = attachments[VK_COLOR_ATTACHMENT].storeOp;
		attachments[attachmentIndex].stencilLoadOp  = attachments[VK_COLOR_ATTACHMENT].stencilLoadOp;
		attachments[attachmentIndex].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[attachmentIndex].initialLayout  = attachments[VK_COLOR_ATTACHMENT].initialLayout;
		attachments[attachmentIndex].finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		attachmentRefs[attachmentIndex].attachment = attachmentIndex;
		attachmentRefs[attachmentIndex].layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		subpass.pResolveAttachments = &attachmentRefs[attachmentIndex];
	}

	// RENDER PASS
	VkRenderPassCreateInfo renderPassInfo    = {};
	VkRenderPass           renderPass        = nullptr;

	renderPassInfo.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = ATTACHMENT_COUNT;
	renderPassInfo.pAttachments    = attachments.data();
	renderPassInfo.subpassCount    = 1;
	renderPassInfo.pSubpasses      = &subpass;
	renderPassInfo.dependencyCount = ((attachmentDesc == VK_COLOR_ATTACHMENT) || (attachmentDesc == NR_OF_VK_ATTACHMENTS) ? 1 : 0);
	renderPassInfo.pDependencies   = &subPassDependency;

	if (vkCreateRenderPass(this->deviceContext, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
		return nullptr;

	return renderPass;
}

VkSurfaceKHR VKContext::initSurface()
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

VKSwapchain* VKContext::initSwapChain()
{
	if ((this->swapChainSupport == nullptr) || (this->deviceContext == nullptr))
		return nullptr;

	VkPresentModeKHR presentMode   = this->getPresentationMode(this->swapChainSupport->PresentModes);
	uint32_t         maxImageCount = this->swapChainSupport->SurfaceCapabilities.maxImageCount;
	uint32_t         imageCount    = (this->swapChainSupport->SurfaceCapabilities.minImageCount + 1);
	VKSwapchain*     swapChain     = new VKSwapchain(this->deviceContext);

	swapChain->SurfaceFormat = this->getSurfaceFormat(this->swapChainSupport->SurfaceFormats);
	swapChain->Size          = this->getSurfaceSize(this->swapChainSupport->SurfaceCapabilities);

	// Make sure we don't request more images than max allowed
	if ((maxImageCount > 0) && (imageCount > maxImageCount))
		imageCount = maxImageCount;

	VkSwapchainCreateInfoKHR swapChainInfo = {};

	swapChainInfo.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapChainInfo.clipped          = VK_TRUE;
	swapChainInfo.compositeAlpha   = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapChainInfo.preTransform     = this->swapChainSupport->SurfaceCapabilities.currentTransform;
	swapChainInfo.presentMode      = presentMode;
	swapChainInfo.surface          = this->surface;
	swapChainInfo.minImageCount    = imageCount;
	swapChainInfo.imageFormat      = swapChain->SurfaceFormat.format;
	swapChainInfo.imageColorSpace  = swapChain->SurfaceFormat.colorSpace;
	swapChainInfo.imageExtent      = swapChain->Size;
	swapChainInfo.imageArrayLayers = 1;
	swapChainInfo.imageUsage       = (VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
	swapChainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;

	// Use concurrenct share mode if the graphics and presentation queues are different
	if (this->queues[VK_QUEUE_PRESENTATION]->Queue != this->queues[VK_QUEUE_GRAPHICS]->Queue)
	{
		uint32_t queueIndices[] = {
			(uint32_t)this->queues[VK_QUEUE_PRESENTATION]->Index,
			(uint32_t)this->queues[VK_QUEUE_GRAPHICS]->Index
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
		swapChain->ImageViews[i] = this->createImageView(swapChain->Images[i], swapChain->SurfaceFormat.format, VK_IMAGE_ASPECT_COLOR_BIT, 1, VK_IMAGE_VIEW_TYPE_2D);

	return swapChain;
}

bool VKContext::initSync()
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
	}

	return true;
}

VkPipelineViewportStateCreateInfo VKContext::initViewport()
{
	VkPipelineViewportStateCreateInfo viewportState = {};

	viewportState.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.scissorCount  = 1;

	return viewportState;
}

bool VKContext::init(bool vsync)
{
	this->frameIndex = 0;
	this->vSync      = vsync;

	this->instance = this->initInstance();

	if (this->instance == nullptr)
		return false;

	this->surface = this->initSurface();

	if (this->surface == nullptr)
		return false;

	this->deviceContext = this->initDeviceContext();

	if (this->deviceContext == nullptr)
		return false;

	this->commandPool = this->initCommandPool();

	if (this->commandPool == nullptr)
		return false;

	if (!this->updateSwapChain(false))
		return false;

	if (!this->initSync())
		return false;

	RenderEngine::GPU.Vendor   = "";
	RenderEngine::GPU.Renderer = this->getDeviceName(this->device);
	RenderEngine::GPU.Version  = this->getApiVersion(this->device);

	return true;
}

bool VKContext::IsOK()
{
	return this->isOK;
}

void VKContext::Present(VkCommandBuffer cmdBuffer)
{
	VkCommandBuffer      commandBuffer      = (cmdBuffer != nullptr ? cmdBuffer : this->commandBuffers[this->imageIndex]);
	VkPresentInfoKHR     presentInfo        = {};
	VkSemaphore          signalSemaphores[] = { this->semDrawComplete[this->frameIndex] };
	VkSubmitInfo         submitInfo         = {};
	VkSwapchainKHR       swapChains[]       = { this->swapChain->SwapChain };
	VkSemaphore          waitSemaphores[]   = { this->semImageAvailable[this->frameIndex] };
	VkPipelineStageFlags waitStages[]       = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

	// STOP RENDER PASS
	vkCmdEndRenderPass(commandBuffer);

	if (cmdBuffer != nullptr) {
		RenderEngine::Canvas.VK->CommandBufferEnd(cmdBuffer);
		return;
	}

	// STOP RECORDING COMMAND BUFFER
	vkEndCommandBuffer(commandBuffer);

	// SUBMIT DRAW COMMAND TO QUEUE
	submitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pCommandBuffers      = &commandBuffer;
	submitInfo.commandBufferCount   = 1;
	submitInfo.pSignalSemaphores    = signalSemaphores;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pWaitSemaphores      = waitSemaphores;
	submitInfo.waitSemaphoreCount   = 1;
	submitInfo.pWaitDstStageMask    = waitStages;

	vkQueueSubmit(this->queues[VK_QUEUE_GRAPHICS]->Queue, 1, &submitInfo, this->frameFences[this->frameIndex]);

	// PRESENT THE DRAWN BUFFER TO SCREEN
	presentInfo.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pWaitSemaphores    = signalSemaphores;
	presentInfo.waitSemaphoreCount = 1;

	presentInfo.pImageIndices  = &this->imageIndex;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains    = swapChains;

	VkResult result = vkQueuePresentKHR(this->queues[VK_QUEUE_PRESENTATION]->Queue, &presentInfo);

	if ((result == VK_ERROR_OUT_OF_DATE_KHR) || (result == VK_SUBOPTIMAL_KHR))
		this->ResetSwapChain();

	this->frameIndex = ((this->frameIndex + 1) % MAX_CONCURRENT_FRAMES);

	// WAIT FOR PRESENTATION TO FINISH
	vkQueueWaitIdle(this->queues[VK_QUEUE_PRESENTATION]->Queue);
}

void VKContext::release()
{
	if (this->deviceContext != nullptr)
		vkDeviceWaitIdle(this->deviceContext);

	this->ResetPipelines();

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

	this->releaseSwapChain(false);

	if (this->commandPool != nullptr) {
		vkDestroyCommandPool(this->deviceContext, this->commandPool, nullptr);
		this->commandPool = nullptr;
	}

	_DELETEP(this->swapChainSupport);

	if (this->surface != nullptr) {
		vkDestroySurfaceKHR(this->instance, this->surface, nullptr);
		this->surface = nullptr;
	}

	for (auto queue : this->queues)
		_DELETEP(queue);

	this->queues.clear();

	if (this->deviceContext != nullptr) {
		vkDestroyDevice(this->deviceContext, nullptr);
		this->deviceContext = nullptr;
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

void VKContext::releaseSwapChain(bool releaseSupport)
{
	if (!this->commandBuffers.empty()) {
		vkFreeCommandBuffers(this->deviceContext, this->commandPool, (uint32_t)this->commandBuffers.size(), this->commandBuffers.data());
		this->commandBuffers.clear();
	}

	for (auto frameBuffer : this->frameBuffers)
		this->DestroyFramebuffer(&frameBuffer);

	this->frameBuffers.clear();

	if (this->swapChain != nullptr)
	{
		for (size_t i = 0; i < this->swapChain->Images.size(); i++) {
			this->DestroyTexture(&this->depthBufferImages[i], &this->depthBufferImageMemories[i], &this->depthBufferImageViews[i], nullptr);
			this->DestroyTexture(&this->colorImages[i],       &this->colorImageMemories[i],       &this->colorImageViews[i],       nullptr);
		}
	}

	for (int i = 0; i < NR_OF_RENDER_PASSES; i++)
	{
		if (this->renderPasses[i] != nullptr) {
			vkDestroyRenderPass(this->deviceContext, this->renderPasses[i], nullptr);
			this->renderPasses[i] = nullptr;
		}
	}

	_DELETEP(this->swapChain);

	if (releaseSupport)
		_DELETEP(this->swapChainSupport);
}

void VKContext::ResetPipelines()
{
	RenderEngine::Ready = false;

	for (auto mesh : RenderEngine::Renderables)
	{
		Buffer* vertexBuffer = dynamic_cast<Mesh*>(mesh)->VertexBuffer();

		if (vertexBuffer != nullptr)
			vertexBuffer->ResetPipelines();
	}

	RenderEngine::Ready = true;
}

bool VKContext::ResetSwapChain()
{
	RenderEngine::Ready = false;

	if (this->deviceContext != nullptr)
		vkDeviceWaitIdle(this->deviceContext);

	this->releaseSwapChain(true);

	if (!this->updateSwapChain(true))
		return false;

	RenderEngine::Ready = true;

	return true;
}

void VKContext::SetVSync(bool enable)
{
	this->vSync = enable;

	this->ResetSwapChain();
}

void VKContext::transitionImageLayout(VkCommandBuffer cmdBuffer, VkImageMemoryBarrier &imageMemBarrier, VkPipelineStageFlagBits destStage)
{
	vkCmdPipelineBarrier(
		cmdBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, destStage, 0, 0, nullptr, 0, nullptr, 1, &imageMemBarrier
	);
}

bool VKContext::updateSwapChain(bool updateSupport)
{
	if (updateSupport)
		this->swapChainSupport = this->getDeviceSwapChainSupport(this->device);

	if (this->swapChainSupport == nullptr)
		return false;

	this->swapChain = this->initSwapChain();

	if (this->swapChain == nullptr)
		return false;

	this->renderPasses[RENDER_PASS_DEFAULT]   = this->initRenderPass(this->swapChain->SurfaceFormat.format, this->multiSampleCount, NR_OF_VK_ATTACHMENTS);
	this->renderPasses[RENDER_PASS_FBO_COLOR] = this->initRenderPass(VK_FORMAT_R8G8B8A8_SRGB, 1, VK_COLOR_ATTACHMENT);
	//this->renderPasses[RENDER_PASS_FBO_DEPTH] = this->initRenderPass(this->getDepthBufferFormat(), 1, VK_DEPTH_STENCIL_ATTACHMENT);
	this->renderPasses[RENDER_PASS_FBO_DEPTH] = this->initRenderPass(VK_FORMAT_D16_UNORM,     1, VK_DEPTH_STENCIL_ATTACHMENT);
	
	for (int i = 0; i < NR_OF_RENDER_PASSES; i++) {
		if (this->renderPasses[i] == nullptr)
			return false;
	}

	if (this->initColorImages() < 0)
		return false;

	if (this->initDepthStencilImages() < 0)
		return false;

	this->frameBuffers = this->initFramebuffers();

	if (this->frameBuffers.empty())
		return false;

	this->commandBuffers = this->initCommandBuffers(this->frameBuffers.size());

	if (this->commandBuffers.empty())
		return false;

	return true;
}
