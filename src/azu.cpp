#include "azu.h"

#include "consts.h"
#include "util/util.h"
#include "SDL_error.h"
#include "glm/ext/matrix_clip_space.hpp"
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

	// inverted top and bottom because... uhhh idk
	projectionMatrix = glm::ortho(0.0, (double)width, 0.0, (double)height);

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

	// request image from the swapchain (1 sec timeout) and signal
	// _presentSemaphore
	uint32_t swapchainImageIndex;
	VK_CHECK(vkAcquireNextImageKHR(context.vk._device, context.vk._swapchain,
	                               1000000000, context.vk._presentSemaphore,
	                               nullptr, &swapchainImageIndex));

	VK_CHECK(vkResetCommandBuffer(context.vk._mainCommandBuffer, 0));

	// BEGIN COMMAND BUFFER
	// --------------------

	// naming it cmd for shorter writing
	VkCommandBuffer cmd = context.vk._mainCommandBuffer;

	// this command buffer will be used
	// exactly once, so the usage_one_time flag is used
	VkCommandBufferBeginInfo cmdBeginInfo = {};
	cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmdBeginInfo.pNext = nullptr;

	cmdBeginInfo.pInheritanceInfo = nullptr;
	cmdBeginInfo.flags            = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));

	// BEGIN RENDER PASS
	// -----------------

	// clear screen to black each frame
	VkClearValue clearValue;
	clearValue.color = {
	    {0.0f, 0.0f, 0.0, 1.0f}
    };

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

	// RENDERING COMMANDS
	// ------------------

	vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
	                  context.vk._pipeline);

	vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
	                        context.vk._pipelineLayout, 0, 1,
	                        &context.vk._globalDescriptorSet, 0, nullptr);

	vkCmdPushConstants(cmd, context.vk._pipelineLayout,
	                   VK_SHADER_STAGE_VERTEX_BIT, 0, 4 * 4 * 4,
	                   &context.projectionMatrix);

	vkCmdDraw(cmd, 6 * QUAD_COUNT, 1, 0, 0);

	vkCmdEndRenderPass(cmd);
	VK_CHECK(vkEndCommandBuffer(cmd));

	// SUBMIT TO QUEUE
	// ---------------

	// wait on _presentSemaphore, as that semaphore is signaled when the
	// swapchain is ready
	// signal _renderSemaphore, to say that rendering has finished

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

	// PRESENT TO SWAPCHAIN
	// --------------------

	// this will put the rendered image into the visible window.
	// I'm waiting on _renderSemaphore for that, which is signaled when the
	// drawing commands submitted to the queue have finished executing

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

	while (!bQuit) {
		while (SDL_PollEvent(&e) != 0) {
			if (e.type == SDL_QUIT)
				bQuit = true;
		}

		draw(context);
	}
}
