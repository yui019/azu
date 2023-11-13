#include "vk_context.h"

#include "../util/util.h"
#include "../util/vk_init.h"
#include "../vk_pipeline/vk_pipeline.h"
#include "VkBootstrap.h"
#include <SDL_vulkan.h>
#include <cstdio>

VkContext::VkContext(SDL_Window *window, VkExtent2D windowExtent,
                     bool useValidationLayers) {
	_windowExtent = windowExtent;

	initVulkan(window, useValidationLayers);
	initSwapchain();
	initDefaultRenderpass();
	initFramebuffers();
	initCommands();
	initSyncStructures();
	initPipelines();
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

	_deletionQueue.push_function([](const VkContext &ctx) {
		vkDestroySwapchainKHR(ctx._device, ctx._swapchain, nullptr);
	});
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

	VK_CHECK(
	    vkCreateRenderPass(_device, &renderPassInfo, nullptr, &_renderPass));

	_deletionQueue.push_function([](const VkContext &ctx) {
		vkDestroyRenderPass(ctx._device, ctx._renderPass, nullptr);
	});
}

void VkContext::initFramebuffers() {
	// create the framebuffers for the swapchain images. This will connect the
	// render-pass to the images for rendering
	VkFramebufferCreateInfo framebufferInfo =
	    vk_init::framebufferCreateInfo(_renderPass, _windowExtent);

	const uint32_t swapchainImageCount = _swapchainImages.size();
	_framebuffers = std::vector<VkFramebuffer>(swapchainImageCount);

	for (uint32_t i = 0; i < swapchainImageCount; i++) {
		framebufferInfo.pAttachments = &_swapchainImageViews[i];
		VK_CHECK(vkCreateFramebuffer(_device, &framebufferInfo, nullptr,
		                             &_framebuffers[i]));
	}

	_deletionQueue.push_function([](const VkContext &ctx) {
		for (uint32_t i = 0; i < ctx._framebuffers.size(); i++) {
			vkDestroyFramebuffer(ctx._device, ctx._framebuffers[i], nullptr);
			vkDestroyImageView(ctx._device, ctx._swapchainImageViews[i],
			                   nullptr);
		}
	});
}

void VkContext::initCommands() {
	// create a command pool for commands submitted to the graphics queue.
	// we also want the pool to allow for resetting of individual command
	// buffers
	VkCommandPoolCreateInfo commandPoolInfo = vk_init::commandPoolCreateInfo(
	    _graphicsQueueFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

	VK_CHECK(
	    vkCreateCommandPool(_device, &commandPoolInfo, nullptr, &_commandPool));

	// allocate the default command buffer that we will use for rendering
	VkCommandBufferAllocateInfo cmdAllocInfo =
	    vk_init::commandBufferAllocateInfo(_commandPool, 1);

	VK_CHECK(
	    vkAllocateCommandBuffers(_device, &cmdAllocInfo, &_mainCommandBuffer));

	_deletionQueue.push_function([](const VkContext &ctx) {
		vkDestroyCommandPool(ctx._device, ctx._commandPool, nullptr);
	});
}

void VkContext::initSyncStructures() {
	// create syncronization structures
	// one fence to control when the gpu has finished rendering the frame,
	// and 2 semaphores to syncronize rendering with swapchain
	// we want the fence to start signalled so we can wait on it on the first
	// frame
	VkFenceCreateInfo fenceCreateInfo =
	    vk_init::fenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);

	VK_CHECK(vkCreateFence(_device, &fenceCreateInfo, nullptr, &_renderFence));

	_deletionQueue.push_function([](const VkContext &ctx) {
		vkDestroyFence(ctx._device, ctx._renderFence, nullptr);
	});

	VkSemaphoreCreateInfo semaphoreCreateInfo = vk_init::semaphoreCreateInfo();

	VK_CHECK(vkCreateSemaphore(_device, &semaphoreCreateInfo, nullptr,
	                           &_presentSemaphore));
	VK_CHECK(vkCreateSemaphore(_device, &semaphoreCreateInfo, nullptr,
	                           &_renderSemaphore));

	_deletionQueue.push_function([](const VkContext &ctx) {
		vkDestroySemaphore(ctx._device, ctx._presentSemaphore, nullptr);
		vkDestroySemaphore(ctx._device, ctx._renderSemaphore, nullptr);
	});
}

void VkContext::initPipelines() {
	auto triangleFragShader =
	    loadShaderModuleFromFile("./shaders/triangle.frag.spv");

	if (!triangleFragShader) {
		printf("FAILED to build triangle fragment shader.\n");
		return;
	} else {
		printf("SUCCESSFULLY built triangle fragment shader.\n");
	}

	auto triangleVertShader =
	    loadShaderModuleFromFile("./shaders/triangle.vert.spv");

	if (!triangleVertShader) {
		printf("FAILED to build triangle vertex shader.\n");
		return;
	} else {
		printf("SUCCESSFULLY built triangle vertex shader.\n");
	}

	// build the pipeline layout that controls the inputs/outputs of the shader
	// we are not using descriptor sets or other systems yet, so no need to use
	// anything other than empty default
	VkPipelineLayoutCreateInfo pipeline_layout_info =
	    vk_init::pipelineLayoutCreateInfo();

	VK_CHECK(vkCreatePipelineLayout(_device, &pipeline_layout_info, nullptr,
	                                &_trianglePipelineLayout));

	// build the stage-create-info for both vertex and fragment stages. This
	// lets the pipeline know the shader modules per stage
	PipelineBuilder pipelineBuilder;

	pipelineBuilder._shaderStages.push_back(
	    vk_init::pipelineShaderStageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT,
	                                           triangleVertShader.value()));

	pipelineBuilder._shaderStages.push_back(
	    vk_init::pipelineShaderStageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT,
	                                           triangleFragShader.value()));

	// vertex input controls how to read vertices from vertex buffers. We aren't
	// using it yet
	pipelineBuilder._vertexInputInfo =
	    vk_init::pipelineVertexInputStateCreateInfo();

	// input assembly is the configuration for drawing triangle lists, strips,
	// or individual points. we are just going to draw triangle list
	pipelineBuilder._inputAssembly =
	    vk_init::pipelineInputAssemblyStateCreateInfo(
	        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

	// build viewport and scissor from the swapchain extents
	pipelineBuilder._viewport.x        = 0.0f;
	pipelineBuilder._viewport.y        = 0.0f;
	pipelineBuilder._viewport.width    = (float)_windowExtent.width;
	pipelineBuilder._viewport.height   = (float)_windowExtent.height;
	pipelineBuilder._viewport.minDepth = 0.0f;
	pipelineBuilder._viewport.maxDepth = 1.0f;

	pipelineBuilder._scissor.offset = {0, 0};
	pipelineBuilder._scissor.extent = _windowExtent;

	// configure the rasterizer to draw filled triangles
	pipelineBuilder._rasterizer =
	    vk_init::pipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL);

	// we don't use multisampling, so just run the default one
	pipelineBuilder._multisampling =
	    vk_init::pipelineMultisampleStateCreateInfo();

	// a single blend attachment with no blending and writing to RGBA
	pipelineBuilder._colorBlendAttachment =
	    vk_init::pipelineColorBlendAttachmentState();

	// use the triangle layout we created
	pipelineBuilder._pipelineLayout = _trianglePipelineLayout;

	// finally build the pipeline
	_trianglePipeline = pipelineBuilder.build_pipeline(_device, _renderPass);

	vkDestroyShaderModule(_device, triangleFragShader.value(), nullptr);
	vkDestroyShaderModule(_device, triangleVertShader.value(), nullptr);

	_deletionQueue.push_function([](const VkContext &ctx) {
		// destroy the 2 pipelines we have created
		vkDestroyPipeline(ctx._device, ctx._trianglePipeline, nullptr);

		// destroy the pipeline layout that they use
		vkDestroyPipelineLayout(ctx._device, ctx._trianglePipelineLayout,
		                        nullptr);
	});
}