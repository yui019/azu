#include "vk_context.h"
#include "../util/util.h"
#include "../vk_init/vk_init.h"

void VkContext::fillQuadsBuffer(tcb::span<QuadData> quadData) {
	memset(_quadsBuffer.data, 0, INITIAL_QUADS_BUFFER_SIZE);
	memcpy(_quadsBuffer.data, quadData.data(), quadData.size_bytes());
}

void VkContext::immediateSubmit(
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
	VK_CHECK(vkQueueSubmit(_graphicsQueue, 1, &submit,
	                       _immediateSubmitContext.fence));

	vkWaitForFences(_device, 1, &_immediateSubmitContext.fence, true,
	                9999999999);
	vkResetFences(_device, 1, &_immediateSubmitContext.fence);

	// reset the command buffer
	vkResetCommandPool(_device, _immediateSubmitContext.commandPool, 0);
}