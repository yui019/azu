#include "vk_context.h"
#include "../util/util.h"
#include "../vk_init/vk_init.h"

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