#include "azu.h"

#include "util/util.h"
#include "SDL_error.h"
#include <vulkan/vulkan.h>

using namespace azu;

Context::Context(std::string_view title, uint32_t width, uint32_t height) {
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		throw SDL_GetError();
	}

	_window = SDL_CreateWindow(title.data(), SDL_WINDOWPOS_UNDEFINED,
	                           SDL_WINDOWPOS_UNDEFINED, width, height,
	                           SDL_WINDOW_VULKAN);
	if (_window == NULL)
		throw SDL_GetError();

	vk = VkContext(_window, VkExtent2D{width, height}, true);
}

Context::~Context() {
	SDL_DestroyWindow(_window);
}

void draw(Context &context) {
	// wait until the GPU has finished rendering the last frame. Timeout of 1
	// second
	VK_CHECK(vkWaitForFences(context.vk._device, 1, &context.vk._renderFence,
	                         true, 1000000000));
	VK_CHECK(vkResetFences(context.vk._device, 1, &context.vk._renderFence));

	// request image from the swapchain, one second timeout
	uint32_t swapchainImageIndex;
	VK_CHECK(vkAcquireNextImageKHR(context.vk._device, context.vk._swapchain,
	                               1000000000, context.vk._presentSemaphore,
	                               nullptr, &swapchainImageIndex));

	// now that we are sure that the commands finished executing, we can safely
	// reset the command buffer to begin recording again.
	VK_CHECK(vkResetCommandBuffer(context.vk._mainCommandBuffer, 0));

	// naming it cmd for shorter writing
	VkCommandBuffer cmd = context.vk._mainCommandBuffer;

	// begin the command buffer recording. We will use this command buffer
	// exactly once, so we want to let Vulkan know that
	VkCommandBufferBeginInfo cmdBeginInfo = {};
	cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmdBeginInfo.pNext = nullptr;

	cmdBeginInfo.pInheritanceInfo = nullptr;
	cmdBeginInfo.flags            = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));

	// make a clear-color from frame number. This will flash with a 120*pi frame
	// period.
	VkClearValue clearValue;
	float flash      = abs(sin(context.frameNumber / 120.f));
	clearValue.color = {
	    {0.0f, 0.0f, flash, 1.0f}
    };

	// start the main renderpass.
	// We will use the clear color from above, and the framebuffer of the index
	// the swapchain gave us
	VkRenderPassBeginInfo rpInfo = {};
	rpInfo.sType                 = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	rpInfo.pNext                 = nullptr;

	rpInfo.renderPass          = context.vk._renderPass;
	rpInfo.renderArea.offset.x = 0;
	rpInfo.renderArea.offset.y = 0;
	rpInfo.renderArea.extent   = context.vk._windowExtent;
	rpInfo.framebuffer         = context.vk._framebuffers[swapchainImageIndex];

	// connect clear values
	rpInfo.clearValueCount = 1;
	rpInfo.pClearValues    = &clearValue;

	vkCmdBeginRenderPass(cmd, &rpInfo, VK_SUBPASS_CONTENTS_INLINE);

	// rendering commands
	vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
	                  context.vk._trianglePipeline);
	vkCmdDraw(cmd, 6, 1, 0, 0);

	// finalize the render pass
	vkCmdEndRenderPass(cmd);
	// finalize the command buffer (we can no longer add commands, but it can
	// now be executed)
	VK_CHECK(vkEndCommandBuffer(cmd));

	// prepare the submission to the queue.
	// we want to wait on the _presentSemaphore, as that semaphore is signaled
	// when the swapchain is ready we will signal the _renderSemaphore, to
	// signal that rendering has finished

	VkSubmitInfo submit = {};
	submit.sType        = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit.pNext        = nullptr;

	VkPipelineStageFlags waitStage =
	    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	submit.pWaitDstStageMask = &waitStage;

	submit.waitSemaphoreCount = 1;
	submit.pWaitSemaphores    = &context.vk._presentSemaphore;

	submit.signalSemaphoreCount = 1;
	submit.pSignalSemaphores    = &context.vk._renderSemaphore;

	submit.commandBufferCount = 1;
	submit.pCommandBuffers    = &cmd;

	// submit command buffer to the queue and execute it.
	//  _renderFence will now block until the graphic commands finish execution
	VK_CHECK(vkQueueSubmit(context.vk._graphicsQueue, 1, &submit,
	                       context.vk._renderFence));

	// this will put the image we just rendered into the visible window.
	// we want to wait on the _renderSemaphore for that,
	// as it's necessary that drawing commands have finished before the image is
	// displayed to the user
	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType            = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pNext            = nullptr;

	presentInfo.pSwapchains    = &context.vk._swapchain;
	presentInfo.swapchainCount = 1;

	presentInfo.pWaitSemaphores    = &context.vk._renderSemaphore;
	presentInfo.waitSemaphoreCount = 1;

	presentInfo.pImageIndices = &swapchainImageIndex;

	VK_CHECK(vkQueuePresentKHR(context.vk._graphicsQueue, &presentInfo));

	context.frameNumber++;
}

void azu::run(Context &context) {
	SDL_Event e;
	bool bQuit = false;

	// main loop
	while (!bQuit) {
		// Handle events on queue
		while (SDL_PollEvent(&e) != 0) {
			// close the window when user alt-f4s or clicks the X button
			if (e.type == SDL_QUIT)
				bQuit = true;
		}

		draw(context);
	}
}
