#include "vk_context.h"

#include "VkBootstrap.h"
#include "vk_initializers.h"
#include <SDL_vulkan.h>
#include <vulkan/vulkan_core.h>

using namespace azu;

#define VK_CHECK(x)                                                            \
	do {                                                                       \
		VkResult err = x;                                                      \
		if (err) {                                                             \
			throw err;                                                         \
		}                                                                      \
	} while (0)

VkContext::VkContext(
    SDL_Window *window, VkExtent2D windowExtent, bool useValidationLayers
) {
	_windowExtent = windowExtent;

	initVulkan(window, useValidationLayers);
	initSwapchain();
	initDefaultRenderpass();
	initFramebuffers();
	initCommands();
	initSyncStructures();
}

VkContext::~VkContext() {
	// if the VkInstance is set to nullptr, it's probably a VkContext left in an
	// invalid state after being moved, so the destructor shouldn't run
	if (_instance) {
		vkDeviceWaitIdle(_device);

		vkDestroyCommandPool(_device, _commandPool, nullptr);

		vkDestroyFence(_device, _renderFence, nullptr);
		vkDestroySemaphore(_device, _renderSemaphore, nullptr);
		vkDestroySemaphore(_device, _presentSemaphore, nullptr);

		vkDestroySwapchainKHR(_device, _swapchain, nullptr);

		vkDestroyRenderPass(_device, _renderPass, nullptr);

		for (uint32_t i = 0; i < _framebuffers.size(); i++) {
			vkDestroyFramebuffer(_device, _framebuffers[i], nullptr);

			vkDestroyImageView(_device, _swapchainImageViews[i], nullptr);
		}

		vkDestroySurfaceKHR(_instance, _surface, nullptr);

		vkDestroyDevice(_device, nullptr);
		vkb::destroy_debug_utils_messenger(_instance, _debug_messenger);
		vkDestroyInstance(_instance, nullptr);
	}
}

void VkContext::initVulkan(SDL_Window *window, bool useValidationLayers) {
	vkb::InstanceBuilder builder;

	// make the vulkan instance, with basic debug features
	auto vkbInstanceResult = builder.set_app_name("Azu Application")
	                             .request_validation_layers(useValidationLayers)
	                             .use_default_debug_messenger()
	                             .require_api_version(1, 3, 0)
	                             .build();
	if (!vkbInstanceResult) {
		throw vkbInstanceResult.error();
	}

	vkb::Instance vkbInstance = vkbInstanceResult.value();

	// grab the instance
	_instance        = vkbInstance.instance;
	_debug_messenger = vkbInstance.debug_messenger;

	SDL_Vulkan_CreateSurface(window, _instance, &_surface);

	// use vkbootstrap to select a gpu.
	// We want a gpu that can write to the SDL surface and supports vulkan 1.2
	vkb::PhysicalDeviceSelector selector{vkbInstance};
	auto physicalDeviceResult =
	    selector.set_minimum_version(1, 1).set_surface(_surface).select();
	if (!physicalDeviceResult) {
		throw physicalDeviceResult.error();
	}

	vkb::PhysicalDevice physicalDevice = physicalDeviceResult.value();

	// create the final vulkan device

	vkb::DeviceBuilder deviceBuilder{physicalDevice};

	auto vkbDeviceResult = deviceBuilder.build();
	if (!vkbDeviceResult) {
		throw vkbDeviceResult.error();
	}

	vkb::Device vkbDevice = vkbDeviceResult.value();

	// Get the VkDevice handle used in the rest of a vulkan application
	_device    = vkbDevice.device;
	_chosenGPU = physicalDevice.physical_device;

	// use vkbootstrap to get a Graphics queue and family
	auto graphicsQueueResult = vkbDevice.get_queue(vkb::QueueType::graphics);
	if (!graphicsQueueResult) {
		throw graphicsQueueResult.error();
	}

	auto graphicsQueueFamilyResult =
	    vkbDevice.get_queue_index(vkb::QueueType::graphics);
	if (!graphicsQueueFamilyResult) {
		throw graphicsQueueFamilyResult.error();
	}

	_graphicsQueue       = graphicsQueueResult.value();
	_graphicsQueueFamily = graphicsQueueFamilyResult.value();
}

void VkContext::initSwapchain() {
	vkb::SwapchainBuilder vkbSwapchainBuilder{_chosenGPU, _device, _surface};

	vkb::Swapchain vkbSwapchain =
	    vkbSwapchainBuilder
	        .use_default_format_selection()
	        // use vsync present mode
	        .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
	        .set_desired_extent(_windowExtent.width, _windowExtent.height)
	        .build()
	        .value();

	// store swapchain and its related images
	_swapchain           = vkbSwapchain.swapchain;
	_swapchainImages     = vkbSwapchain.get_images().value();
	_swapchainImageViews = vkbSwapchain.get_image_views().value();

	_swapchainImageFormat = vkbSwapchain.image_format;
	throw VK_SUCCESS;
}

void VkContext::initDefaultRenderpass() {
	// we define an attachment description for our main color image
	// the attachment is loaded as "clear" when renderpass start
	// the attachment is stored when renderpass ends
	// the attachment layout starts as "undefined", and transitions to "Present"
	// so its possible to display it we dont care about stencil, and dont use
	// multisampling

	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format                  = _swapchainImageFormat;
	colorAttachment.samples                 = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp                  = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp                 = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp           = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp          = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout           = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout             = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef = {};
	colorAttachmentRef.attachment            = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	// we are going to create 1 subpass, which is the minimum you can do
	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments    = &colorAttachmentRef;

	// 1 dependency, which is from "outside" into the subpass. And we can read
	// or write color
	VkSubpassDependency dependency = {};
	dependency.srcSubpass          = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass          = 0;
	dependency.srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments    = &colorAttachment;
	renderPassInfo.subpassCount    = 1;
	renderPassInfo.pSubpasses      = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies   = &dependency;

	VK_CHECK(vkCreateRenderPass(_device, &renderPassInfo, nullptr, &_renderPass)
	);
}

void VkContext::initFramebuffers() {
	// create the framebuffers for the swapchain images. This will connect the
	// render-pass to the images for rendering
	VkFramebufferCreateInfo framebufferInfo =
	    vkinit::framebuffer_create_info(_renderPass, _windowExtent);

	const uint32_t swapchainImageCount = _swapchainImages.size();
	_framebuffers = std::vector<VkFramebuffer>(swapchainImageCount);

	for (uint32_t i = 0; i < swapchainImageCount; i++) {

		framebufferInfo.pAttachments = &_swapchainImageViews[i];
		VK_CHECK(vkCreateFramebuffer(
		    _device, &framebufferInfo, nullptr, &_framebuffers[i]
		));
	}
}

void VkContext::initCommands() {
	// create a command pool for commands submitted to the graphics queue.
	// we also want the pool to allow for resetting of individual command
	// buffers
	VkCommandPoolCreateInfo commandPoolInfo = vkinit::command_pool_create_info(
	    _graphicsQueueFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT
	);

	VK_CHECK(
	    vkCreateCommandPool(_device, &commandPoolInfo, nullptr, &_commandPool)
	);

	// allocate the default command buffer that we will use for rendering
	VkCommandBufferAllocateInfo cmdAllocInfo =
	    vkinit::command_buffer_allocate_info(_commandPool, 1);

	VK_CHECK(
	    vkAllocateCommandBuffers(_device, &cmdAllocInfo, &_mainCommandBuffer)
	);
}

void VkContext::initSyncStructures() {
	// create syncronization structures
	// one fence to control when the gpu has finished rendering the frame,
	// and 2 semaphores to syncronize rendering with swapchain
	// we want the fence to start signalled so we can wait on it on the first
	// frame
	VkFenceCreateInfo fenceCreateInfo =
	    vkinit::fence_create_info(VK_FENCE_CREATE_SIGNALED_BIT);

	VK_CHECK(vkCreateFence(_device, &fenceCreateInfo, nullptr, &_renderFence));

	VkSemaphoreCreateInfo semaphoreCreateInfo = vkinit::semaphore_create_info();

	VK_CHECK(vkCreateSemaphore(
	    _device, &semaphoreCreateInfo, nullptr, &_presentSemaphore
	));
	VK_CHECK(vkCreateSemaphore(
	    _device, &semaphoreCreateInfo, nullptr, &_renderSemaphore
	));
}