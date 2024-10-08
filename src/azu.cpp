#include "azu.h"

#include "src/util/color.h"
#include "src/vk_context/vk_context.h"
#include "util/texture.h"
#include "util/buffer.h"
#include "util/quad_data.h"
#include "util/util.h"
#include "vk_init/vk_init.h"
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <vector>
#include <vulkan/vulkan_core.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "SDL_error.h"
#include <vulkan/vulkan.h>

using namespace azu;

Context::Context(std::string_view title, uint32_t width, uint32_t height) {
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		throw SDL_GetError();
	}

	Window = SDL_CreateWindow(title.data(), SDL_WINDOWPOS_UNDEFINED,
	                          SDL_WINDOWPOS_UNDEFINED, (int32_t)width,
	                          (int32_t)height,
	                          SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
	if (Window == NULL)
		throw SDL_GetError();

	_calculateProjectionMatrix((float)width, (float)height);

	Vk = VkContext(Window, VkExtent2D{width, height}, true);
}

Context::~Context() {
	SDL_DestroyWindow(Window);
}

void Context::_calculateProjectionMatrix(float windowWidth,
                                         float windowHeight) {
	float left   = 0.0;
	float right  = windowWidth;
	float bottom = windowHeight;
	float top    = 0.0;
	float near   = -1.0;
	float far    = 1.0;

	_projectionMatrix[0][0] = 2 / (right - left);
	_projectionMatrix[1][0] = 0.0;
	_projectionMatrix[2][0] = 0.0;
	_projectionMatrix[3][0] = -(right + left) / (right - left);

	_projectionMatrix[0][1] = 0.0;
	_projectionMatrix[1][1] = 2 / (bottom - top);
	_projectionMatrix[2][1] = 0.0;
	_projectionMatrix[3][1] = -(bottom + top) / (bottom - top);

	_projectionMatrix[0][2] = 0.0;
	_projectionMatrix[1][2] = 0.0;
	_projectionMatrix[2][2] = 1 / (far - near);
	_projectionMatrix[3][2] = -near / (far - near);

	_projectionMatrix[0][3] = 0.0;
	_projectionMatrix[1][3] = 0.0;
	_projectionMatrix[2][3] = 0.0;
	_projectionMatrix[3][3] = 1.0;
}

void Context::_handleResize() {
	vkDeviceWaitIdle(Vk.Device);

	int w, h;
	SDL_GetWindowSize(Window, &w, &h);

	Vk.HandleWindowResize(VkExtent2D{(uint32_t)w, (uint32_t)h});
}

void Context::BeginDraw() {
	// reset this frame's quad data
	_quadData.clear();

	// wait until the GPU has finished rendering the last frame. Timeout of 1
	// second
	VK_CHECK(vkWaitForFences(Vk.Device, 1, &Vk.RenderFence, true, 1000000000));
	VK_CHECK(vkResetFences(Vk.Device, 1, &Vk.RenderFence));

	// If vkAcquireNextImageKHR returns VK_ERROR_OUT_OF_DATE_KHR, that means the
	// swapchain needs to be recreated due to a window resize. But that's
	// apparently not guaranteed to fix it on the first try so it's recreated
	// again and again until it returns successfully. Usually what happens is
	// that it returns VK_ERROR_OUT_OF_DATE_KHR, the swapchain is recreated and
	// it returns success the second time (this second call is necessary btw, so
	// even if it were guaranteed to go this smoothly every time, the loop would
	// still be necessary just so that second call happens)
	// This is all you need to handle window resizing btw, which is sort of
	// surprising.
	while (true) {
		// Request image from the swapchain (1 sec timeout) and signal
		// _presentSemaphore.
		VkResult result = vkAcquireNextImageKHR(Vk.Device, Vk.Swapchain,
		                                        1000000000, Vk.PresentSemaphore,
		                                        nullptr, &_swapchainImageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			_handleResize();
		} else {
			break;
		}
	}

	VK_CHECK(vkResetCommandBuffer(Vk.MainCommandBuffer, 0));

	// BEGIN COMMAND BUFFER
	// --------------------

	// naming it cmd for shorter writing
	VkCommandBuffer cmd = Vk.MainCommandBuffer;

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

	rpInfo.renderPass          = Vk.RenderPass;
	rpInfo.renderArea.offset.x = 0;
	rpInfo.renderArea.offset.y = 0;
	rpInfo.renderArea.extent   = Vk.WindowExtent;
	rpInfo.framebuffer         = Vk.Framebuffers[_swapchainImageIndex];

	// connect clear values
	rpInfo.clearValueCount = 1;
	rpInfo.pClearValues    = &clearValue;

	vkCmdBeginRenderPass(cmd, &rpInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void Context::EndDraw() {
	// fill quads buffer with the quads that were rendered by the user in
	// drawQuads
	Vk.FillQuadsBuffer(_quadData);

	// RENDERING COMMANDS
	// ------------------

	// naming it cmd for shorter writing
	VkCommandBuffer cmd = Vk.MainCommandBuffer;

	vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, Vk.Pipeline);

	vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
	                        Vk.PipelineLayout, 0, 1, &Vk.GlobalDescriptorSet, 0,
	                        nullptr);

	vkCmdPushConstants(cmd, Vk.PipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0,
	                   4 * 4 * 4, &_projectionMatrix);

	vkCmdDraw(cmd, (uint32_t)(6 * _quadData.size()), 1, 0, 0);

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
	submit.pWaitSemaphores    = &Vk.PresentSemaphore;

	submit.signalSemaphoreCount = 1;
	submit.pSignalSemaphores    = &Vk.RenderSemaphore;

	submit.commandBufferCount = 1;
	submit.pCommandBuffers    = &cmd;

	// submit command buffer to the queue and execute it.
	//  _renderFence will now block until the graphic commands finish execution
	VK_CHECK(vkQueueSubmit(Vk.GraphicsQueue, 1, &submit, Vk.RenderFence));

	// PRESENT TO SWAPCHAIN
	// --------------------

	// this will put the rendered image into the visible window.
	// I'm waiting on _renderSemaphore for that, which is signaled when the
	// drawing commands submitted to the queue have finished executing

	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType            = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pNext            = nullptr;

	presentInfo.pSwapchains    = &Vk.Swapchain;
	presentInfo.swapchainCount = 1;

	presentInfo.pWaitSemaphores    = &Vk.RenderSemaphore;
	presentInfo.waitSemaphoreCount = 1;

	presentInfo.pImageIndices = &_swapchainImageIndex;

	vkQueuePresentKHR(Vk.GraphicsQueue, &presentInfo);

	FrameNumber++;
}

void Context::DrawQuad(Quad quad, Color color,
                       std::optional<DrawQuadOptions> options) {
	DrawQuadOptions opt =
	    options.has_value() ? options.value() : DrawQuadOptions();

	_quadData.push_back(QuadData(quad, color, opt));
}

void Context::DrawQuad(Quad quad, const char *textureName,
                       std::optional<DrawQuadOptions> options) {
	if (_textures.count(textureName) == 0) {
		throw std::runtime_error("There is no texture with that name");
	}

	DrawQuadOptions opt =
	    options.has_value() ? options.value() : DrawQuadOptions();

	_quadData.push_back(QuadData(quad, _textures[textureName].vkId, opt));
}

Vec2 Context::GetTextureDimensions(const char *name) {
	if (_textures.count(name) == 0) {
		throw std::runtime_error("There is no texture with that name");
	}

	return {(float)_textures[name].width, (float)_textures[name].height};
}

bool Context::CreateTextureFromFile(const char *name, const char *path) {
	// early return if a texture with the given name
	// already exists
	if (_textures.count(name)) {
		return false;
	}

	int width, height, channels;

	stbi_uc *pixels =
	    stbi_load(path, &width, &height, &channels, STBI_rgb_alpha);

	if (!pixels) {
		return false;
	}

	void *pixel_ptr        = pixels;
	VkDeviceSize imageSize = (uint64_t)(width * height * 4);

	// temporary CPU buffer that will be used to upload
	// to a real GPU buffer later on
	Buffer stagingBuffer =
	    Buffer(Vk.Allocator, (uint32_t)imageSize,
	           VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);

	// copy data to stagingBuffer and unmap its memory
	memcpy(stagingBuffer.Data, pixel_ptr, static_cast<size_t>(imageSize));
	vmaUnmapMemory(Vk.Allocator, stagingBuffer.Allocation);

	// image data is now in stagingBuffer
	stbi_image_free(pixels);

	VkExtent3D imageExtent;
	imageExtent.width  = static_cast<uint32_t>(width);
	imageExtent.height = static_cast<uint32_t>(height);
	imageExtent.depth  = 1;

	VkFormat imageFormat = VK_FORMAT_R8G8B8A8_SRGB;

	VkImageCreateInfo imageCreateInfo = vk_init::imageCreateInfo(
	    imageFormat,
	    VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
	    imageExtent);

	Texture texture;
	texture.vkId   = (uint32_t)_textures.size();
	texture.width  = (uint32_t)width;
	texture.height = (uint32_t)height;

	VmaAllocationCreateInfo imageAllocateInfo = {};
	imageAllocateInfo.usage                   = VMA_MEMORY_USAGE_GPU_ONLY;

	// allocate and create the image
	VK_CHECK(vmaCreateImage(Vk.Allocator, &imageCreateInfo, &imageAllocateInfo,
	                        &texture.image, &texture.allocation, nullptr));

	Vk.ImmediateSubmit([&](VkCommandBuffer cmd) {
		// TRANSFER IMAGE TO
		// VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
		// ------------------------------------------------------

		VkImageSubresourceRange range;
		range.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
		range.baseMipLevel   = 0;
		range.levelCount     = 1;
		range.baseArrayLayer = 0;
		range.layerCount     = 1;

		VkImageMemoryBarrier imageBarrier_toTransfer = {};
		imageBarrier_toTransfer.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;

		imageBarrier_toTransfer.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageBarrier_toTransfer.newLayout =
		    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		imageBarrier_toTransfer.image            = texture.image;
		imageBarrier_toTransfer.subresourceRange = range;

		imageBarrier_toTransfer.srcAccessMask = 0;
		imageBarrier_toTransfer.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		                     VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0,
		                     nullptr, 1, &imageBarrier_toTransfer);

		// COPY BUFFER TO IMAGE
		// ------------------------------------------------------

		VkBufferImageCopy copyRegion = {};
		copyRegion.bufferOffset      = 0;
		copyRegion.bufferRowLength   = 0;
		copyRegion.bufferImageHeight = 0;

		copyRegion.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
		copyRegion.imageSubresource.mipLevel       = 0;
		copyRegion.imageSubresource.baseArrayLayer = 0;
		copyRegion.imageSubresource.layerCount     = 1;
		copyRegion.imageExtent                     = imageExtent;

		vkCmdCopyBufferToImage(cmd, stagingBuffer.VulkanBuffer, texture.image,
		                       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
		                       &copyRegion);

		// TRANSFER IMAGE TO
		// VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
		// ------------------------------------------------------

		VkImageMemoryBarrier imageBarrier_toReadable = imageBarrier_toTransfer;

		imageBarrier_toReadable.oldLayout =
		    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		imageBarrier_toReadable.newLayout =
		    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		imageBarrier_toReadable.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		imageBarrier_toReadable.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT,
		                     VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0,
		                     nullptr, 0, nullptr, 1, &imageBarrier_toReadable);
	});

	Vk.DeletionQueue.pushFunction([texture](const VkContext &ctx) {
		vmaDestroyImage(ctx.Allocator, texture.image, texture.allocation);
	});

	vmaDestroyBuffer(Vk.Allocator, stagingBuffer.VulkanBuffer,
	                 stagingBuffer.Allocation);

	VkImageViewCreateInfo imageViewInfo{};
	imageViewInfo.sType    = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageViewInfo.image    = texture.image;
	imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageViewInfo.format   = imageFormat;
	imageViewInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
	imageViewInfo.subresourceRange.baseMipLevel   = 0;
	imageViewInfo.subresourceRange.levelCount     = 1;
	imageViewInfo.subresourceRange.baseArrayLayer = 0;
	imageViewInfo.subresourceRange.layerCount     = 1;

	VK_CHECK(vkCreateImageView(Vk.Device, &imageViewInfo, nullptr,
	                           &texture.imageView));

	std::vector<VkDescriptorImageInfo> descriptorImageInfos;
	descriptorImageInfos.resize(Vk.INITIAL_ARRAY_OF_TEXTURES_LENGTH);

	// Fill in all the descriptors with this same
	// texture Only doing the minimum number of
	// descriptors (_textures.size()+1), everything else
	// can stay uninitialized
	for (uint32_t i = 0; i < _textures.size() + 1; i++) {
		descriptorImageInfos[i].sampler   = Vk.GlobalSampler;
		descriptorImageInfos[i].imageView = texture.imageView;
		descriptorImageInfos[i].imageLayout =
		    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	}

	// Change all the previously created descriptors
	// into their own image views
	for (auto [name, t] : _textures) {
		descriptorImageInfos[t.vkId].sampler   = Vk.GlobalSampler;
		descriptorImageInfos[t.vkId].imageView = t.imageView;
		descriptorImageInfos[t.vkId].imageLayout =
		    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	}

	VkWriteDescriptorSet setWriteImage = {};
	setWriteImage.sType                = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	setWriteImage.pNext                = nullptr;
	setWriteImage.dstBinding           = 1;
	setWriteImage.dstSet               = Vk.GlobalDescriptorSet;
	setWriteImage.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	setWriteImage.descriptorCount = (uint32_t)_textures.size() + 1;
	setWriteImage.pImageInfo      = descriptorImageInfos.data();

	vkUpdateDescriptorSets(Vk.Device, 1, &setWriteImage, 0, nullptr);

	Vk.DeletionQueue.pushFunction([texture](const VkContext &ctx) {
		vkDestroyImageView(ctx.Device, texture.imageView, nullptr);
	});

	_textures[name] = texture;

	return true;
}