#ifndef VK_CONTEXT_H
#define VK_CONTEXT_H

#include "../util/quad_data.h"
#include "../util/buffer.h"
#include "vk_mem_alloc.h"
#include <SDL.h>
#include <vulkan/vulkan.h>
#include <optional>
#include <span>
#include <vector>
#include <deque>
#include <functional>

namespace azu {

class VkContext {
	void _initVulkan(SDL_Window *window, bool useValidationLayers);
	void _initSwapchain();
	void _initDefaultRenderpass();
	void _initFramebuffers();
	void _initCommands();
	void _initSyncStructures();
	void _initDescriptors();
	void _initSampler();
	void _initPipelines();

	std::optional<VkShaderModule>
	_loadShaderModuleFromFile(const char *path) const;

	struct ImmediateSubmitContext {
		VkFence fence;
		VkCommandPool commandPool;
		VkCommandBuffer commandBuffer;
	};

	ImmediateSubmitContext _immediateSubmitContext;

  public:
	VkInstance _instance                      = nullptr;
	VkDebugUtilsMessengerEXT _debug_messenger = nullptr;
	VkPhysicalDevice _chosenGPU               = nullptr;
	VkDevice _device                          = nullptr;

	VkSemaphore _presentSemaphore, _renderSemaphore = nullptr;
	VkFence _renderFence = nullptr;

	VkQueue _graphicsQueue = nullptr;
	uint32_t _graphicsQueueFamily;

	VkCommandPool _commandPool         = nullptr;
	VkCommandBuffer _mainCommandBuffer = nullptr;

	VkRenderPass _renderPass = nullptr;

	VkSurfaceKHR _surface     = nullptr;
	VkSwapchainKHR _swapchain = nullptr;
	VkFormat _swapchainImageFormat;

	std::vector<VkFramebuffer> _framebuffers;
	std::vector<VkImage> _swapchainImages;
	std::vector<VkImageView> _swapchainImageViews;

	VkPipelineLayout _pipelineLayout;
	VkPipeline _pipeline;

	VkDescriptorPool _globalDescriptorPool;
	VkDescriptorSetLayout _globalDescriptorSetLayout;
	VkDescriptorSet _globalDescriptorSet;

	const uint32_t INITIAL_QUADS_BUFFER_SIZE =
	    sizeof(QuadData) * 10000; // Unit: bytes
	Buffer _quadsBuffer;

	const uint32_t INITIAL_ARRAY_OF_TEXTURES_LENGTH = 1000; // Unit: elements

	VkSampler _globalSampler;

	VkExtent2D _windowExtent;

	VmaAllocator _allocator;

	VkContext() = default;

	VkContext(SDL_Window *window, VkExtent2D windowExtent,
	          bool useValidationLayers);

	VkContext(const VkContext &other)            = delete;
	VkContext &operator=(const VkContext &other) = delete;

	VkContext(VkContext &&other) {
		*this = std::move(other);
	}

	VkContext &operator=(VkContext &&other) {
		// I'm sort of new to this move semantics thing, but I really don't see
		// a better way of doing this...
		std::swap(_instance, other._instance);
		std::swap(_debug_messenger, other._debug_messenger);
		std::swap(_chosenGPU, other._chosenGPU);
		std::swap(_device, other._device);
		std::swap(_presentSemaphore, other._presentSemaphore);
		std::swap(_renderSemaphore, other._renderSemaphore);
		std::swap(_renderFence, other._renderFence);
		std::swap(_graphicsQueue, other._graphicsQueue);
		std::swap(_graphicsQueueFamily, other._graphicsQueueFamily);
		std::swap(_commandPool, other._commandPool);
		std::swap(_mainCommandBuffer, other._mainCommandBuffer);
		std::swap(_renderPass, other._renderPass);
		std::swap(_surface, other._surface);
		std::swap(_swapchain, other._swapchain);
		std::swap(_swapchainImageFormat, other._swapchainImageFormat);
		std::swap(_framebuffers, other._framebuffers);
		std::swap(_swapchainImages, other._swapchainImages);
		std::swap(_swapchainImageViews, other._swapchainImageViews);
		std::swap(_pipelineLayout, other._pipelineLayout);
		std::swap(_pipeline, other._pipeline);
		std::swap(_windowExtent, other._windowExtent);
		std::swap(_deletionQueue, other._deletionQueue);
		std::swap(_allocator, other._allocator);
		std::swap(_globalDescriptorPool, other._globalDescriptorPool);
		std::swap(_globalDescriptorSetLayout, other._globalDescriptorSetLayout);
		std::swap(_globalDescriptorSet, other._globalDescriptorSet);
		std::swap(_quadsBuffer, other._quadsBuffer);
		std::swap(_immediateSubmitContext, other._immediateSubmitContext);
		std::swap(_globalSampler, other._globalSampler);

		return *this;
	}

	~VkContext();

	struct DeletionQueue {
		std::deque<std::function<void(const VkContext &context)>> deletors;

		void push_function(std::function<void(const VkContext &)> &&function) {
			deletors.push_back(function);
		}

		void flush(const VkContext &context) {
			for (auto it = deletors.rbegin(); it != deletors.rend(); it++) {
				(*it)(context);
			}

			deletors.clear();
		}
	};

	DeletionQueue _deletionQueue;

	void immediateSubmit(std::function<void(VkCommandBuffer cmd)> &&function);

	void fillQuadsBuffer(std::span<QuadData> quadData);
};

} // namespace azu

#endif // VK_CONTEXT_H