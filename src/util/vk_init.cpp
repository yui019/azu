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

VkFramebufferCreateInfo
vk_init::framebufferCreateInfo(VkRenderPass renderPass, VkExtent2D extent) {
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

VkRenderPassBeginInfo vk_init::renderPassBeginInfo(
    VkRenderPass renderPass, VkExtent2D windowExtent, VkFramebuffer framebuffer
) {
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