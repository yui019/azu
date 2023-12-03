#include <vulkan/vulkan_core.h>
#define VMA_IMPLEMENTATION
#include "vk_context.h"

#include "../util/util.h"
#include "../vk_init/vk_init.h"
#include "../vk_pipeline/vk_pipeline.h"
#include "VkBootstrap.h"
#include <SDL_vulkan.h>
#include <cstdio>

using namespace azu;

VkContext::VkContext(SDL_Window *window, VkExtent2D windowExtent,
                     bool useValidationLayers) {
	_windowExtent = windowExtent;

	_initVulkan(window, useValidationLayers);
	_initSwapchain();
	_initDefaultRenderpass();
	_initFramebuffers();
	_initCommands();
	_initSyncStructures();
	_initDescriptors();
	_initSampler();
	_initPipelines();
}

void VkContext::_initVulkan(SDL_Window *window, bool useValidationLayers) {
	vkb::InstanceBuilder builder;

	auto vkbInstanceResult = builder.set_app_name("Azu Application")
	                             .request_validation_layers(useValidationLayers)
	                             .use_default_debug_messenger()
	                             .require_api_version(1, 3, 0)
	                             .build();
	if (!vkbInstanceResult) {
		throw vkbInstanceResult.error();
	}

	vkb::Instance vkbInstance = vkbInstanceResult.value();

	_instance        = vkbInstance.instance;
	_debug_messenger = vkbInstance.debug_messenger;

	SDL_Vulkan_CreateSurface(window, _instance, &_surface);

	VkPhysicalDeviceVulkan12Features features          = {};
	features.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
	vkb::PhysicalDeviceSelector selector{vkbInstance};
	auto physicalDeviceResult = selector.set_minimum_version(1, 3)
	                                .set_surface(_surface)
	                                .set_required_features_12(features)
	                                .select();
	if (!physicalDeviceResult) {
		throw physicalDeviceResult.error();
	}

	vkb::PhysicalDevice physicalDevice = physicalDeviceResult.value();

	vkb::DeviceBuilder deviceBuilder{physicalDevice};

	auto vkbDeviceResult = deviceBuilder.build();
	if (!vkbDeviceResult) {
		throw vkbDeviceResult.error();
	}

	vkb::Device vkbDevice = vkbDeviceResult.value();

	_device    = vkbDevice.device;
	_chosenGPU = physicalDevice.physical_device;

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

	// MEMORY ALLOCATOR
	// ----------------

	VmaAllocatorCreateInfo allocatorInfo = {};
	allocatorInfo.physicalDevice         = _chosenGPU;
	allocatorInfo.device                 = _device;
	allocatorInfo.instance               = _instance;
	VK_CHECK(vmaCreateAllocator(&allocatorInfo, &_allocator));

	_deletionQueue.push_function(
	    [](const VkContext &ctx) { vmaDestroyAllocator(ctx._allocator); });
}

void VkContext::_initSwapchain() {
	vkb::SwapchainBuilder vkbSwapchainBuilder{_chosenGPU, _device, _surface};

	vkb::Swapchain vkbSwapchain =
	    vkbSwapchainBuilder
	        .use_default_format_selection()
	        // use vsync present mode
	        .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
	        .set_desired_extent(_windowExtent.width, _windowExtent.height)
	        .build()
	        .value();

	_swapchain            = vkbSwapchain.swapchain;
	_swapchainImages      = vkbSwapchain.get_images().value();
	_swapchainImageViews  = vkbSwapchain.get_image_views().value();
	_swapchainImageFormat = vkbSwapchain.image_format;

	_deletionQueue.push_function([](const VkContext &ctx) {
		vkDestroySwapchainKHR(ctx._device, ctx._swapchain, nullptr);
	});
}

void VkContext::_initDefaultRenderpass() {
	// COLOR ATTACHMENT
	// ----------------

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

	// SUBPASS
	// -------

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments    = &colorAttachmentRef;

	VkSubpassDependency dependency = {};
	dependency.srcSubpass          = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass          = 0;
	dependency.srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	// CREATE RENDERPASS
	// -----------------

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

void VkContext::_initFramebuffers() {
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

void VkContext::_initCommands() {
	VkCommandPoolCreateInfo commandPoolInfo = vk_init::commandPoolCreateInfo(
	    _graphicsQueueFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

	VK_CHECK(
	    vkCreateCommandPool(_device, &commandPoolInfo, nullptr, &_commandPool));

	VkCommandBufferAllocateInfo cmdAllocInfo =
	    vk_init::commandBufferAllocateInfo(_commandPool, 1);

	VK_CHECK(
	    vkAllocateCommandBuffers(_device, &cmdAllocInfo, &_mainCommandBuffer));

	_deletionQueue.push_function([](const VkContext &ctx) {
		vkDestroyCommandPool(ctx._device, ctx._commandPool, nullptr);
	});

	// also a command pool and command buffer for the immediateSubmit function

	VkCommandPoolCreateInfo immediateSubmitPool =
	    vk_init::commandPoolCreateInfo(_graphicsQueueFamily);
	VK_CHECK(vkCreateCommandPool(_device, &immediateSubmitPool, nullptr,
	                             &_immediateSubmitContext.commandPool));

	_deletionQueue.push_function([](const VkContext &ctx) {
		vkDestroyCommandPool(ctx._device,
		                     ctx._immediateSubmitContext.commandPool, nullptr);
	});

	VkCommandBufferAllocateInfo immediateSubmitBuffer =
	    vk_init::commandBufferAllocateInfo(_immediateSubmitContext.commandPool,
	                                       1);

	VK_CHECK(vkAllocateCommandBuffers(_device, &immediateSubmitBuffer,
	                                  &_immediateSubmitContext.commandBuffer));
}

void VkContext::_initSyncStructures() {
	// one fence to control when the gpu has finished rendering the frame, and 2
	// semaphores to syncronize rendering with swapchain the fence starts
	// signaled so I can wait on it on the first frame

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

	// also a fence for the immedateSubmit function

	VkFenceCreateInfo immediateSubmitFence = vk_init::fenceCreateInfo();

	VK_CHECK(vkCreateFence(_device, &immediateSubmitFence, nullptr,
	                       &_immediateSubmitContext.fence));
	_deletionQueue.push_function([](const VkContext &ctx) {
		vkDestroyFence(ctx._device, ctx._immediateSubmitContext.fence, nullptr);
	});
}

void VkContext::_initDescriptors() {
	// CREATE DESCRIPTOR POOL
	// ----------------------

	std::vector<VkDescriptorPoolSize> sizes = {
	    {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,         10},
	    {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
	     INITIAL_ARRAY_OF_TEXTURES_LENGTH             }
    };

	VkDescriptorPoolCreateInfo pool_info = {};
	pool_info.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_info.flags         = 0;
	pool_info.maxSets       = 10;
	pool_info.poolSizeCount = (uint32_t)sizes.size();
	pool_info.pPoolSizes    = sizes.data();

	VK_CHECK(vkCreateDescriptorPool(_device, &pool_info, nullptr,
	                                &_globalDescriptorPool));

	_deletionQueue.push_function([](const VkContext &ctx) {
		vkDestroyDescriptorPool(ctx._device, ctx._globalDescriptorPool,
		                        nullptr);
	});

	// CREATE DESCRIPTOR SET LAYOUT
	// ----------------------------

	VkDescriptorSetLayoutBinding quadsBufferBinding = {};
	quadsBufferBinding.binding                      = 0;
	quadsBufferBinding.descriptorCount              = 1;
	quadsBufferBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	quadsBufferBinding.stageFlags     = VK_SHADER_STAGE_VERTEX_BIT;

	VkDescriptorSetLayoutBinding texturesBinding = {};
	texturesBinding.binding                      = 1;
	texturesBinding.descriptorCount = INITIAL_ARRAY_OF_TEXTURES_LENGTH;
	texturesBinding.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	texturesBinding.stageFlags      = VK_SHADER_STAGE_FRAGMENT_BIT;

	VkDescriptorSetLayoutBinding bindings[] = {quadsBufferBinding,
	                                           texturesBinding};

	VkDescriptorSetLayoutCreateInfo layoutInfo = {};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.pNext = nullptr;
	layoutInfo.bindingCount = 2;
	layoutInfo.pBindings    = bindings;
	layoutInfo.flags        = 0;

	VK_CHECK(vkCreateDescriptorSetLayout(_device, &layoutInfo, nullptr,
	                                     &_globalDescriptorSetLayout));

	_deletionQueue.push_function([](const VkContext &ctx) {
		vkDestroyDescriptorSetLayout(ctx._device,
		                             ctx._globalDescriptorSetLayout, nullptr);
	});

	// CREATE QUADS BUFFER
	// -------------------

	_quadsBuffer =
	    Buffer(_allocator, INITIAL_QUADS_BUFFER_SIZE,
	           VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

	_deletionQueue.push_function([](const VkContext &ctx) {
		vmaDestroyBuffer(ctx._allocator, ctx._quadsBuffer.buffer,
		                 ctx._quadsBuffer.allocation);
	});

	// ALLOCATE DESCRIPTOR SET
	// -----------------------

	VkDescriptorSetAllocateInfo allocateInfo = {};
	allocateInfo.pNext                       = nullptr;
	allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocateInfo.descriptorPool     = _globalDescriptorPool;
	allocateInfo.descriptorSetCount = 1;
	allocateInfo.pSetLayouts        = &_globalDescriptorSetLayout;

	VK_CHECK(vkAllocateDescriptorSets(_device, &allocateInfo,
	                                  &_globalDescriptorSet));

	// UPDATE DESCRIPTOR SET TO POINT TO QUADS BUFFER
	// ----------------------------------------------

	VkDescriptorBufferInfo descriptorBufferInfo;
	descriptorBufferInfo.buffer = _quadsBuffer.buffer;
	descriptorBufferInfo.offset = 0;
	descriptorBufferInfo.range  = INITIAL_QUADS_BUFFER_SIZE;

	VkWriteDescriptorSet setWriteBuffer = {};
	setWriteBuffer.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	setWriteBuffer.pNext           = nullptr;
	setWriteBuffer.dstBinding      = 0;
	setWriteBuffer.dstSet          = _globalDescriptorSet;
	setWriteBuffer.descriptorCount = 1;
	setWriteBuffer.descriptorType  = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	setWriteBuffer.pBufferInfo     = &descriptorBufferInfo;

	vkUpdateDescriptorSets(_device, 1, &setWriteBuffer, 0, nullptr);
}

void VkContext::_initSampler() {
	VkSamplerCreateInfo samplerInfo = vk_init::samplerCreateInfo(
	    VK_FILTER_NEAREST, VK_SAMPLER_ADDRESS_MODE_REPEAT);

	VK_CHECK(vkCreateSampler(_device, &samplerInfo, nullptr, &_globalSampler));

	_deletionQueue.push_function([](const VkContext &ctx) {
		vkDestroySampler(ctx._device, ctx._globalSampler, nullptr);
	});
}

void VkContext::_initPipelines() {
	// BUILD SHADERS
	// -------------

	auto triangleFragShader =
	    _loadShaderModuleFromFile("./shaders/quad.frag.spv");

	if (!triangleFragShader) {
		printf("FAILED to build quad fragment shader.\n");
		return;
	} else {
		printf("SUCCESSFULLY built quad fragment shader.\n");
	}

	auto triangleVertShader =
	    _loadShaderModuleFromFile("./shaders/quad.vert.spv");

	if (!triangleVertShader) {
		printf("FAILED to build quad vertex shader.\n");
		return;
	} else {
		printf("SUCCESSFULLY built quad vertex shader.\n");
	}

	// CREATE PIPELINE LAYOUT
	// ----------------------

	VkPushConstantRange pushConstantRanges[] = {
	    {VK_SHADER_STAGE_VERTEX_BIT, 0, 4 * 4 * 4}
    };
	VkDescriptorSetLayout descriptorSetLayouts[] = {_globalDescriptorSetLayout};
	VkPipelineLayoutCreateInfo pipeline_layout_info =
	    vk_init::pipelineLayoutCreateInfo(pushConstantRanges,
	                                      descriptorSetLayouts);

	VK_CHECK(vkCreatePipelineLayout(_device, &pipeline_layout_info, nullptr,
	                                &_pipelineLayout));

	// BUILD PIPELINE
	// --------------

	PipelineBuilder pipelineBuilder;

	pipelineBuilder.shaderStages = {
	    vk_init::pipelineShaderStageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT,
	                                           triangleVertShader.value()),
	    vk_init::pipelineShaderStageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT,
	                                           triangleFragShader.value())};

	pipelineBuilder.vertexInputInfo =
	    vk_init::pipelineVertexInputStateCreateInfo();

	pipelineBuilder.inputAssembly =
	    vk_init::pipelineInputAssemblyStateCreateInfo(
	        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

	pipelineBuilder.rasterizer =
	    vk_init::pipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL);

	pipelineBuilder.multisampling =
	    vk_init::pipelineMultisampleStateCreateInfo();

	pipelineBuilder.colorBlendAttachment =
	    vk_init::pipelineColorBlendAttachmentState();

	// build viewport and scissor from the swapchain extents
	pipelineBuilder.viewport.x        = 0.0f;
	pipelineBuilder.viewport.y        = 0.0f;
	pipelineBuilder.viewport.width    = (float)_windowExtent.width;
	pipelineBuilder.viewport.height   = (float)_windowExtent.height;
	pipelineBuilder.viewport.minDepth = 0.0f;
	pipelineBuilder.viewport.maxDepth = 1.0f;
	pipelineBuilder.scissor.offset    = {0, 0};
	pipelineBuilder.scissor.extent    = _windowExtent;

	pipelineBuilder.pipelineLayout = _pipelineLayout;

	// finally build the pipeline
	auto pipeline = pipelineBuilder.build(_device, _renderPass);
	if (pipeline) {
		_pipeline = pipeline.value();
	} else {
		throw std::runtime_error("Failed to create pipeline");
	}

	vkDestroyShaderModule(_device, triangleFragShader.value(), nullptr);
	vkDestroyShaderModule(_device, triangleVertShader.value(), nullptr);

	_deletionQueue.push_function([](const VkContext &ctx) {
		vkDestroyPipeline(ctx._device, ctx._pipeline, nullptr);
		vkDestroyPipelineLayout(ctx._device, ctx._pipelineLayout, nullptr);
	});
}