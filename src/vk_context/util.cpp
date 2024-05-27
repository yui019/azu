#include "vk_context.h"
#include "../util/util.h"
#include "../vk_init/vk_init.h"
#include "VkBootstrap.h"

using namespace azu;

void VkContext::FillQuadsBuffer(std::span<QuadData> quadData) {
	memset(QuadsBuffer.Data, 0, INITIAL_QUADS_BUFFER_SIZE);
	memcpy(QuadsBuffer.Data, quadData.data(), quadData.size_bytes());
}

void VkContext::ImmediateSubmit(
    std::function<void(VkCommandBuffer cmd)> &&function) {
	VkCommandBuffer cmd = _immediateSubmitContext.commandBuffer;

	// begin command buffer recording
	// buffer used only once before being reset
	VkCommandBufferBeginInfo cmdBeginInfo = vk_init::commandBufferBeginInfo(
	    VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));

	// execute the function
	function(cmd);

	VK_CHECK(vkEndCommandBuffer(cmd));

	VkSubmitInfo submit = vk_init::submitInfo(&cmd);

	// submit command buffer to the queue and execute it.
	//  _immediateSubmitContext.fence will now block until the commands finish
	//  execution
	VK_CHECK(vkQueueSubmit(GraphicsQueue, 1, &submit,
	                       _immediateSubmitContext.fence));

	vkWaitForFences(Device, 1, &_immediateSubmitContext.fence, true,
	                9999999999);
	vkResetFences(Device, 1, &_immediateSubmitContext.fence);

	// reset the command buffer
	vkResetCommandPool(Device, _immediateSubmitContext.commandPool, 0);
}

void VkContext::HandleWindowResize(VkExtent2D newWindowExtent) {
	WindowExtent = newWindowExtent;

	// DESTROY CURRENT FRAMEBUFFERS AND SWAPCHAIN
	// ==========================================

	for (uint32_t i = 0; i < Framebuffers.size(); i++) {
		vkDestroyFramebuffer(Device, Framebuffers[i], nullptr);
		vkDestroyImageView(Device, SwapchainImageViews[i], nullptr);
	}

	vkDestroySwapchainKHR(Device, Swapchain, nullptr);

	// CREATE NEW SWAPCHAIN
	// ====================

	vkb::SwapchainBuilder vkbSwapchainBuilder{chosenGPU, Device, Surface};

	vkb::Swapchain vkbSwapchain =
	    vkbSwapchainBuilder
	        .use_default_format_selection()
	        // use vsync present mode
	        .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
	        .set_desired_extent(WindowExtent.width, WindowExtent.height)
	        .build()
	        .value();

	Swapchain            = vkbSwapchain.swapchain;
	SwapchainImages      = vkbSwapchain.get_images().value();
	SwapchainImageViews  = vkbSwapchain.get_image_views().value();
	SwapchainImageFormat = vkbSwapchain.image_format;

	// CREATE NEW FRAMEBUFFERS
	// =======================

	VkFramebufferCreateInfo framebufferInfo =
	    vk_init::framebufferCreateInfo(RenderPass, WindowExtent);

	const uint32_t swapchainImageCount = (uint32_t)SwapchainImages.size();
	Framebuffers = std::vector<VkFramebuffer>(swapchainImageCount);

	for (uint32_t i = 0; i < swapchainImageCount; i++) {
		framebufferInfo.pAttachments = &SwapchainImageViews[i];
		VK_CHECK(vkCreateFramebuffer(Device, &framebufferInfo, nullptr,
		                             &Framebuffers[i]));
	}
}