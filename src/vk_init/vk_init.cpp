#include "vk_init.h"

VkCommandPoolCreateInfo vk_init::commandPoolCreateInfo(
    uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags /*= 0*/
) {
	VkCommandPoolCreateInfo info = {};
	info.sType                   = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	info.pNext                   = nullptr;

	info.flags = flags;
	return info;
}

VkCommandBufferAllocateInfo vk_init::commandBufferAllocateInfo(
    VkCommandPool pool, uint32_t count /*= 1*/,
    VkCommandBufferLevel level /*= VK_COMMAND_BUFFER_LEVEL_PRIMARY*/
) {
	VkCommandBufferAllocateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	info.pNext = nullptr;

	info.commandPool        = pool;
	info.commandBufferCount = count;
	info.level              = level;
	return info;
}

VkCommandBufferBeginInfo
vk_init::commandBufferBeginInfo(VkCommandBufferUsageFlags flags /*= 0*/) {
	VkCommandBufferBeginInfo info = {};
	info.sType                    = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	info.pNext                    = nullptr;

	info.pInheritanceInfo = nullptr;
	info.flags            = flags;
	return info;
}

VkFramebufferCreateInfo vk_init::framebufferCreateInfo(VkRenderPass renderPass,
                                                       VkExtent2D extent) {
	VkFramebufferCreateInfo info = {};
	info.sType                   = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	info.pNext                   = nullptr;

	info.renderPass      = renderPass;
	info.attachmentCount = 1;
	info.width           = extent.width;
	info.height          = extent.height;
	info.layers          = 1;

	return info;
}

VkFenceCreateInfo vk_init::fenceCreateInfo(VkFenceCreateFlags flags /*= 0*/) {
	VkFenceCreateInfo info = {};
	info.sType             = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	info.pNext             = nullptr;

	info.flags = flags;

	return info;
}

VkSemaphoreCreateInfo
vk_init::semaphoreCreateInfo(VkSemaphoreCreateFlags flags /*= 0*/) {
	VkSemaphoreCreateInfo info = {};
	info.sType                 = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	info.pNext                 = nullptr;
	info.flags                 = flags;
	return info;
}

VkSubmitInfo vk_init::submitInfo(VkCommandBuffer *cmd) {
	VkSubmitInfo info = {};
	info.sType        = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	info.pNext        = nullptr;

	info.waitSemaphoreCount   = 0;
	info.pWaitSemaphores      = nullptr;
	info.pWaitDstStageMask    = nullptr;
	info.commandBufferCount   = 1;
	info.pCommandBuffers      = cmd;
	info.signalSemaphoreCount = 0;
	info.pSignalSemaphores    = nullptr;

	return info;
}

VkPresentInfoKHR vk_init::presentInfo() {
	VkPresentInfoKHR info = {};
	info.sType            = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	info.pNext            = nullptr;

	info.swapchainCount     = 0;
	info.pSwapchains        = nullptr;
	info.pWaitSemaphores    = nullptr;
	info.waitSemaphoreCount = 0;
	info.pImageIndices      = nullptr;

	return info;
}

VkRenderPassBeginInfo vk_init::renderPassBeginInfo(VkRenderPass renderPass,
                                                   VkExtent2D windowExtent,
                                                   VkFramebuffer framebuffer) {
	VkRenderPassBeginInfo info = {};
	info.sType                 = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	info.pNext                 = nullptr;

	info.renderPass          = renderPass;
	info.renderArea.offset.x = 0;
	info.renderArea.offset.y = 0;
	info.renderArea.extent   = windowExtent;
	info.clearValueCount     = 1;
	info.pClearValues        = nullptr;
	info.framebuffer         = framebuffer;

	return info;
}

VkPipelineShaderStageCreateInfo
vk_init::pipelineShaderStageCreateInfo(VkShaderStageFlagBits stage,
                                       VkShaderModule shaderModule) {

	VkPipelineShaderStageCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	info.pNext = nullptr;

	// shader stage
	info.stage = stage;
	// module containing the code for this shader stage
	info.module = shaderModule;
	// the entry point of the shader
	info.pName = "main";
	return info;
}

VkPipelineVertexInputStateCreateInfo
vk_init::pipelineVertexInputStateCreateInfo() {
	VkPipelineVertexInputStateCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	info.pNext = nullptr;

	// no vertex bindings or attributes
	info.vertexBindingDescriptionCount   = 0;
	info.vertexAttributeDescriptionCount = 0;
	return info;
}

VkPipelineInputAssemblyStateCreateInfo
vk_init::pipelineInputAssemblyStateCreateInfo(VkPrimitiveTopology topology) {
	VkPipelineInputAssemblyStateCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	info.pNext = nullptr;

	info.topology = topology;
	// we are not going to use primitive restart on the entire tutorial so leave
	// it on false
	info.primitiveRestartEnable = VK_FALSE;
	return info;
}

VkPipelineRasterizationStateCreateInfo
vk_init::pipelineRasterizationStateCreateInfo(VkPolygonMode polygonMode) {
	VkPipelineRasterizationStateCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	info.pNext = nullptr;

	info.depthClampEnable = VK_FALSE;
	// discards all primitives before the rasterization stage if enabled which
	// we don't want
	info.rasterizerDiscardEnable = VK_FALSE;

	info.polygonMode = polygonMode;
	info.lineWidth   = 1.0f;
	// no backface cull
	info.cullMode  = VK_CULL_MODE_NONE;
	info.frontFace = VK_FRONT_FACE_CLOCKWISE;
	// no depth bias
	info.depthBiasEnable         = VK_FALSE;
	info.depthBiasConstantFactor = 0.0f;
	info.depthBiasClamp          = 0.0f;
	info.depthBiasSlopeFactor    = 0.0f;

	return info;
}

VkPipelineMultisampleStateCreateInfo
vk_init::pipelineMultisampleStateCreateInfo() {
	VkPipelineMultisampleStateCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	info.pNext = nullptr;

	info.sampleShadingEnable = VK_FALSE;
	// multisampling defaulted to no multisampling (1 sample per pixel)
	info.rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT;
	info.minSampleShading      = 1.0f;
	info.pSampleMask           = nullptr;
	info.alphaToCoverageEnable = VK_FALSE;
	info.alphaToOneEnable      = VK_FALSE;
	return info;
}

VkPipelineColorBlendAttachmentState
vk_init::pipelineColorBlendAttachmentState() {
	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.colorWriteMask =
	    VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
	    VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_TRUE;

	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	colorBlendAttachment.dstColorBlendFactor =
	    VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;

	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	colorBlendAttachment.dstAlphaBlendFactor =
	    VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

	return colorBlendAttachment;
}

VkPipelineLayoutCreateInfo vk_init::pipelineLayoutCreateInfo(
    tcb::span<VkPushConstantRange> pushConstantRanges,
    tcb::span<VkDescriptorSetLayout> descriptorSetLayouts) {
	VkPipelineLayoutCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	info.pNext = nullptr;

	// empty defaults
	info.flags                  = 0;
	info.setLayoutCount         = descriptorSetLayouts.size();
	info.pSetLayouts            = descriptorSetLayouts.data();
	info.pushConstantRangeCount = pushConstantRanges.size();
	info.pPushConstantRanges    = pushConstantRanges.data();
	return info;
}

VkBufferCreateInfo vk_init::bufferCreateInfo(uint32_t size,
                                             VkBufferUsageFlags usage) {
	VkBufferCreateInfo info = {};
	info.sType              = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	info.pNext              = nullptr;
	info.size               = size;
	info.usage              = usage;

	return info;
}

VkImageCreateInfo vk_init::imageCreateInfo(VkFormat format,
                                           VkImageUsageFlags usageFlags,
                                           VkExtent3D extent) {
	VkImageCreateInfo info = {};
	info.sType             = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	info.pNext             = nullptr;

	info.imageType = VK_IMAGE_TYPE_2D;

	info.format = format;
	info.extent = extent;

	info.mipLevels   = 1;
	info.arrayLayers = 1;
	info.samples     = VK_SAMPLE_COUNT_1_BIT;
	info.tiling      = VK_IMAGE_TILING_OPTIMAL;
	info.usage       = usageFlags;

	return info;
}

VkSamplerCreateInfo
vk_init::samplerCreateInfo(VkFilter filters,
                           VkSamplerAddressMode samplerAddressMode) {
	VkSamplerCreateInfo info = {};
	info.sType               = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	info.pNext               = nullptr;

	info.magFilter    = filters;
	info.minFilter    = filters;
	info.addressModeU = samplerAddressMode;
	info.addressModeV = samplerAddressMode;
	info.addressModeW = samplerAddressMode;

	return info;
}