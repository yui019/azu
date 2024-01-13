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
	VkInstance Instance                     = nullptr;
	VkDebugUtilsMessengerEXT DebugMessenger = nullptr;
	VkPhysicalDevice chosenGPU              = nullptr;
	VkDevice Device                         = nullptr;

	VkSemaphore PresentSemaphore, RenderSemaphore = nullptr;
	VkFence RenderFence = nullptr;

	VkQueue GraphicsQueue = nullptr;
	uint32_t GraphicsQueueFamily;

	VkCommandPool CommandPool         = nullptr;
	VkCommandBuffer MainCommandBuffer = nullptr;

	VkRenderPass RenderPass = nullptr;

	VkSurfaceKHR Surface     = nullptr;
	VkSwapchainKHR Swapchain = nullptr;
	VkFormat SwapchainImageFormat;

	std::vector<VkFramebuffer> Framebuffers;
	std::vector<VkImage> SwapchainImages;
	std::vector<VkImageView> SwapchainImageViews;

	VkPipelineLayout PipelineLayout;
	VkPipeline Pipeline;

	VkDescriptorPool GlobalDescriptorPool;
	VkDescriptorSetLayout GlobalDescriptorSetLayout;
	VkDescriptorSet GlobalDescriptorSet;

	const uint32_t INITIAL_QUADS_BUFFER_SIZE =
	    sizeof(QuadData) * 10000; // Unit: bytes
	Buffer QuadsBuffer;

	const uint32_t INITIAL_ARRAY_OF_TEXTURES_LENGTH = 1000; // Unit: elements

	VkSampler GlobalSampler;

	VkExtent2D WindowExtent;

	VmaAllocator Allocator;

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
		std::swap(Instance, other.Instance);
		std::swap(DebugMessenger, other.DebugMessenger);
		std::swap(chosenGPU, other.chosenGPU);
		std::swap(Device, other.Device);
		std::swap(PresentSemaphore, other.PresentSemaphore);
		std::swap(RenderSemaphore, other.RenderSemaphore);
		std::swap(RenderFence, other.RenderFence);
		std::swap(GraphicsQueue, other.GraphicsQueue);
		std::swap(GraphicsQueueFamily, other.GraphicsQueueFamily);
		std::swap(CommandPool, other.CommandPool);
		std::swap(MainCommandBuffer, other.MainCommandBuffer);
		std::swap(RenderPass, other.RenderPass);
		std::swap(Surface, other.Surface);
		std::swap(Swapchain, other.Swapchain);
		std::swap(SwapchainImageFormat, other.SwapchainImageFormat);
		std::swap(Framebuffers, other.Framebuffers);
		std::swap(SwapchainImages, other.SwapchainImages);
		std::swap(SwapchainImageViews, other.SwapchainImageViews);
		std::swap(PipelineLayout, other.PipelineLayout);
		std::swap(Pipeline, other.Pipeline);
		std::swap(WindowExtent, other.WindowExtent);
		std::swap(DeletionQueue, other.DeletionQueue);
		std::swap(Allocator, other.Allocator);
		std::swap(GlobalDescriptorPool, other.GlobalDescriptorPool);
		std::swap(GlobalDescriptorSetLayout, other.GlobalDescriptorSetLayout);
		std::swap(GlobalDescriptorSet, other.GlobalDescriptorSet);
		std::swap(QuadsBuffer, other.QuadsBuffer);
		std::swap(_immediateSubmitContext, other._immediateSubmitContext);
		std::swap(GlobalSampler, other.GlobalSampler);

		return *this;
	}

	~VkContext();

	struct DeletionQueue {
		std::deque<std::function<void(const VkContext &context)>> deletors;

		void pushFunction(std::function<void(const VkContext &)> &&function) {
			deletors.push_back(function);
		}

		void flush(const VkContext &context) {
			for (auto it = deletors.rbegin(); it != deletors.rend(); it++) {
				(*it)(context);
			}

			deletors.clear();
		}
	};

	DeletionQueue DeletionQueue;

	void ImmediateSubmit(std::function<void(VkCommandBuffer cmd)> &&function);

	void FillQuadsBuffer(std::span<QuadData> quadData);
};

} // namespace azu

#endif // VK_CONTEXT_H