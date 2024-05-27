#ifndef UTIL_VK_INIT_H
#define UTIL_VK_INIT_H

#include <span>
#include <cstdint>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

namespace azu::vk_init {

VkCommandPoolCreateInfo
commandPoolCreateInfo(uint32_t queueFamilyIndex,
                      VkCommandPoolCreateFlags flags = 0);

VkCommandBufferAllocateInfo commandBufferAllocateInfo(
    VkCommandPool pool, uint32_t count = 1,
    VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);

VkCommandBufferBeginInfo
commandBufferBeginInfo(VkCommandBufferUsageFlags flags = 0);

VkFramebufferCreateInfo framebufferCreateInfo(VkRenderPass renderPass,
                                              VkExtent2D extent);

VkFenceCreateInfo fenceCreateInfo(VkFenceCreateFlags flags = 0);

VkSemaphoreCreateInfo semaphoreCreateInfo(VkSemaphoreCreateFlags flags = 0);

VkSubmitInfo submitInfo(VkCommandBuffer *cmd);

VkPresentInfoKHR presentInfo();

VkRenderPassBeginInfo renderPassBeginInfo(VkRenderPass renderPass,
                                          VkExtent2D windowExtent,
                                          VkFramebuffer framebuffer);

VkPipelineShaderStageCreateInfo
pipelineShaderStageCreateInfo(VkShaderStageFlagBits stage,
                              VkShaderModule shaderModule);

VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo();

VkPipelineInputAssemblyStateCreateInfo
pipelineInputAssemblyStateCreateInfo(VkPrimitiveTopology topology);

VkPipelineRasterizationStateCreateInfo
pipelineRasterizationStateCreateInfo(VkPolygonMode polygonMode);

VkPipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo();

VkPipelineColorBlendAttachmentState pipelineColorBlendAttachmentState();

VkPipelineLayoutCreateInfo
pipelineLayoutCreateInfo(std::span<VkPushConstantRange> pushConstantRanges,
                         std::span<VkDescriptorSetLayout> descriptorSetLayouts);

VkBufferCreateInfo bufferCreateInfo(uint32_t size, VkBufferUsageFlags usage);

VkImageCreateInfo imageCreateInfo(VkFormat format, VkImageUsageFlags usageFlags,
                                  VkExtent3D extent);

VkSamplerCreateInfo samplerCreateInfo(VkFilter filters,
                                      VkSamplerAddressMode samplerAddressMode);
} // namespace azu::vk_init

#endif // UTIL_VK_INIT_H