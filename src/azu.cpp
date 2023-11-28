#include "azu.h"

#include "util/quad_data.h"
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
	_projectionMatrix = glm::ortho(0.0, (double)width, 0.0, (double)height);

	_vk = VkContext(_window, VkExtent2D{width, height}, true);
}

Context::~Context() {
	SDL_DestroyWindow(_window);
}

void Context::beginDraw() {
	// reset this frame's quad data
	_quadData.clear();

	// wait until the GPU has finished rendering the last frame. Timeout of 1
	// second
	VK_CHECK(
	    vkWaitForFences(_vk._device, 1, &_vk._renderFence, true, 1000000000));
	VK_CHECK(vkResetFences(_vk._device, 1, &_vk._renderFence));

	// request image from the swapchain (1 sec timeout) and signal
	// _presentSemaphore
	VK_CHECK(vkAcquireNextImageKHR(_vk._device, _vk._swapchain, 1000000000,
	                               _vk._presentSemaphore, nullptr,
	                               &_swapchainImageIndex));

	VK_CHECK(vkResetCommandBuffer(_vk._mainCommandBuffer, 0));

	// BEGIN COMMAND BUFFER
	// --------------------

	// naming it cmd for shorter writing
	VkCommandBuffer cmd = _vk._mainCommandBuffer;

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

	rpInfo.renderPass          = _vk._renderPass;
	rpInfo.renderArea.offset.x = 0;
	rpInfo.renderArea.offset.y = 0;
	rpInfo.renderArea.extent   = _vk._windowExtent;
	rpInfo.framebuffer         = _vk._framebuffers[_swapchainImageIndex];

	// connect clear values
	rpInfo.clearValueCount = 1;
	rpInfo.pClearValues    = &clearValue;

	vkCmdBeginRenderPass(cmd, &rpInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void Context::endDraw() {
	// fill quads buffer with the quads that were rendered by the user in
	// drawQuads
	_vk.fillQuadsBuffer(_quadData);

	// RENDERING COMMANDS
	// ------------------

	// naming it cmd for shorter writing
	VkCommandBuffer cmd = _vk._mainCommandBuffer;

	vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _vk._pipeline);

	vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
	                        _vk._pipelineLayout, 0, 1,
	                        &_vk._globalDescriptorSet, 0, nullptr);

	vkCmdPushConstants(cmd, _vk._pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0,
	                   4 * 4 * 4, &_projectionMatrix);

	vkCmdDraw(cmd, 6 * _quadData.size(), 1, 0, 0);

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
	submit.pWaitSemaphores    = &_vk._presentSemaphore;

	submit.signalSemaphoreCount = 1;
	submit.pSignalSemaphores    = &_vk._renderSemaphore;

	submit.commandBufferCount = 1;
	submit.pCommandBuffers    = &cmd;

	// submit command buffer to the queue and execute it.
	//  _renderFence will now block until the graphic commands finish execution
	VK_CHECK(vkQueueSubmit(_vk._graphicsQueue, 1, &submit, _vk._renderFence));

	// PRESENT TO SWAPCHAIN
	// --------------------

	// this will put the rendered image into the visible window.
	// I'm waiting on _renderSemaphore for that, which is signaled when the
	// drawing commands submitted to the queue have finished executing

	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType            = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pNext            = nullptr;

	presentInfo.pSwapchains    = &_vk._swapchain;
	presentInfo.swapchainCount = 1;

	presentInfo.pWaitSemaphores    = &_vk._renderSemaphore;
	presentInfo.waitSemaphoreCount = 1;

	presentInfo.pImageIndices = &_swapchainImageIndex;

	VK_CHECK(vkQueuePresentKHR(_vk._graphicsQueue, &presentInfo));

	frameNumber++;
}

void Context::drawQuad(Quad quad, Color fill) {
	_quadData.push_back({quad, fill});
}