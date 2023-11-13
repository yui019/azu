#include <SDL.h>
#include <optional>
#include <vulkan/vulkan.h>
#include <vector>

class VkContext {
	void initVulkan(SDL_Window *window, bool useValidationLayers);
	void initSwapchain();
	void initDefaultRenderpass();
	void initFramebuffers();
	void initCommands();
	void initSyncStructures();
	void initPipelines();

	std::optional<VkShaderModule>
	loadShaderModuleFromFile(const char *path) const;

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

	VkPipelineLayout _trianglePipelineLayout;
	VkPipeline _trianglePipeline;

	VkExtent2D _windowExtent;

	VkContext() {}

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
		std::swap(_trianglePipelineLayout, other._trianglePipelineLayout);
		std::swap(_trianglePipeline, other._trianglePipeline);
		std::swap(_windowExtent, other._windowExtent);

		return *this;
	}

	~VkContext();
};