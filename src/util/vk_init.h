#pragma once

#include <vulkan/vulkan.h>

namespace vk_init {

VkCommandPoolCreateInfo commandPoolCreateInfo(
    uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags = 0
);

VkCommandBufferAllocateInfo commandBufferAllocateInfo(
    VkCommandPool pool, uint32_t count = 1,
    VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY
);

VkCommandBufferBeginInfo
commandBufferBeginInfo(VkCommandBufferUsageFlags flags = 0);

VkFramebufferCreateInfo
framebufferCreateInfo(VkRenderPass renderPass, VkExtent2D extent);

VkFenceCreateInfo fenceCreateInfo(VkFenceCreateFlags flags = 0);

VkSemaphoreCreateInfo semaphoreCreateInfo(VkSemaphoreCreateFlags flags = 0);

VkSubmitInfo submitInfo(VkCommandBuffer *cmd);

VkPresentInfoKHR presentInfo();

VkRenderPassBeginInfo renderPassBeginInfo(
    VkRenderPass renderPass, VkExtent2D windowExtent, VkFramebuffer framebuffer
);

} // namespace vk_init