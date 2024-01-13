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
	WindowExtent = windowExtent;

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

	Instance       = vkbInstance.instance;
	DebugMessenger = vkbInstance.debug_messenger;

	SDL_Vulkan_CreateSurface(window, Instance, &Surface);

	vkb::PhysicalDeviceSelector selector{vkbInstance};
	auto physicalDeviceResult =
	    selector.set_minimum_version(1, 3).set_surface(Surface).select();
	if (!physicalDeviceResult) {
		throw physicalDeviceResult.error();
	}

	vkb::PhysicalDevice physicalDevice = physicalDeviceResult.value();

	vkb::DeviceBuilder deviceBuilder{physicalDevice};

	VkPhysicalDeviceDescriptorIndexingFeatures indexingFeatures{};
	indexingFeatures.sType =
	    VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
	indexingFeatures.shaderSampledImageArrayNonUniformIndexing =
	    VK_TRUE; // can index into descriptor arrays with a uniform in shader
	indexingFeatures.runtimeDescriptorArray =
	    VK_TRUE; // can have descriptor arrays with dynamic size in shader
	indexingFeatures.descriptorBindingPartiallyBound =
	    VK_TRUE; // don't need to update unused descriptors

	auto vkbDeviceResult = deviceBuilder.add_pNext(&indexingFeatures).build();
	if (!vkbDeviceResult) {
		throw vkbDeviceResult.error();
	}

	vkb::Device vkbDevice = vkbDeviceResult.value();

	Device    = vkbDevice.device;
	chosenGPU = physicalDevice.physical_device;

	auto graphicsQueueResult = vkbDevice.get_queue(vkb::QueueType::graphics);
	if (!graphicsQueueResult) {
		throw graphicsQueueResult.error();
	}

	auto graphicsQueueFamilyResult =
	    vkbDevice.get_queue_index(vkb::QueueType::graphics);
	if (!graphicsQueueFamilyResult) {
		throw graphicsQueueFamilyResult.error();
	}

	GraphicsQueue       = graphicsQueueResult.value();
	GraphicsQueueFamily = graphicsQueueFamilyResult.value();

	// MEMORY ALLOCATOR
	// ----------------

	VmaAllocatorCreateInfo allocatorInfo = {};
	allocatorInfo.physicalDevice         = chosenGPU;
	allocatorInfo.device                 = Device;
	allocatorInfo.instance               = Instance;
	VK_CHECK(vmaCreateAllocator(&allocatorInfo, &Allocator));

	DeletionQueue.pushFunction(
	    [](const VkContext &ctx) { vmaDestroyAllocator(ctx.Allocator); });
}

void VkContext::_initSwapchain() {
	vkb::SwapchainBuilder vkbSwapchainBuilder{chosenGPU, Device, Surface};

	vkb::Swapchain vkbSwapchain =
	    vkbSwapchainBuilder
	        .use_default_format_selection()
	        // use vsync present mode
	        .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
	        .set_desired_extent(WindowExtent.width, WindowExtent.height)
	        .build()
	        .value();

	Swapchain            = vkbSwapchain.swapchain;
	SwapchainImages      = vkbSwapchain.get_images().value();
	SwapchainImageViews  = vkbSwapchain.get_image_views().value();
	SwapchainImageFormat = vkbSwapchain.image_format;

	DeletionQueue.pushFunction([](const VkContext &ctx) {
		vkDestroySwapchainKHR(ctx.Device, ctx.Swapchain, nullptr);
	});
}

void VkContext::_initDefaultRenderpass() {
	// COLOR ATTACHMENT
	// ----------------

	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format                  = SwapchainImageFormat;
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

	VK_CHECK(vkCreateRenderPass(Device, &renderPassInfo, nullptr, &RenderPass));

	DeletionQueue.pushFunction([](const VkContext &ctx) {
		vkDestroyRenderPass(ctx.Device, ctx.RenderPass, nullptr);
	});
}

void VkContext::_initFramebuffers() {
	VkFramebufferCreateInfo framebufferInfo =
	    vk_init::framebufferCreateInfo(RenderPass, WindowExtent);

	const uint32_t swapchainImageCount = SwapchainImages.size();
	Framebuffers = std::vector<VkFramebuffer>(swapchainImageCount);

	for (uint32_t i = 0; i < swapchainImageCount; i++) {
		framebufferInfo.pAttachments = &SwapchainImageViews[i];
		VK_CHECK(vkCreateFramebuffer(Device, &framebufferInfo, nullptr,
		                             &Framebuffers[i]));
	}

	DeletionQueue.pushFunction([](const VkContext &ctx) {
		for (uint32_t i = 0; i < ctx.Framebuffers.size(); i++) {
			vkDestroyFramebuffer(ctx.Device, ctx.Framebuffers[i], nullptr);
			vkDestroyImageView(ctx.Device, ctx.SwapchainImageViews[i], nullptr);
		}
	});
}

void VkContext::_initCommands() {
	VkCommandPoolCreateInfo commandPoolInfo = vk_init::commandPoolCreateInfo(
	    GraphicsQueueFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

	VK_CHECK(
	    vkCreateCommandPool(Device, &commandPoolInfo, nullptr, &CommandPool));

	VkCommandBufferAllocateInfo cmdAllocInfo =
	    vk_init::commandBufferAllocateInfo(CommandPool, 1);

	VK_CHECK(
	    vkAllocateCommandBuffers(Device, &cmdAllocInfo, &MainCommandBuffer));

	DeletionQueue.pushFunction([](const VkContext &ctx) {
		vkDestroyCommandPool(ctx.Device, ctx.CommandPool, nullptr);
	});

	// also a command pool and command buffer for the immediateSubmit function

	VkCommandPoolCreateInfo immediateSubmitPool =
	    vk_init::commandPoolCreateInfo(GraphicsQueueFamily);
	VK_CHECK(vkCreateCommandPool(Device, &immediateSubmitPool, nullptr,
	                             &_immediateSubmitContext.commandPool));

	DeletionQueue.pushFunction([](const VkContext &ctx) {
		vkDestroyCommandPool(ctx.Device,
		                     ctx._immediateSubmitContext.commandPool, nullptr);
	});

	VkCommandBufferAllocateInfo immediateSubmitBuffer =
	    vk_init::commandBufferAllocateInfo(_immediateSubmitContext.commandPool,
	                                       1);

	VK_CHECK(vkAllocateCommandBuffers(Device, &immediateSubmitBuffer,
	                                  &_immediateSubmitContext.commandBuffer));
}

void VkContext::_initSyncStructures() {
	// one fence to control when the gpu has finished rendering the frame, and 2
	// semaphores to syncronize rendering with swapchain the fence starts
	// signaled so I can wait on it on the first frame

	VkFenceCreateInfo fenceCreateInfo =
	    vk_init::fenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);

	VK_CHECK(vkCreateFence(Device, &fenceCreateInfo, nullptr, &RenderFence));

	DeletionQueue.pushFunction([](const VkContext &ctx) {
		vkDestroyFence(ctx.Device, ctx.RenderFence, nullptr);
	});

	VkSemaphoreCreateInfo semaphoreCreateInfo = vk_init::semaphoreCreateInfo();

	VK_CHECK(vkCreateSemaphore(Device, &semaphoreCreateInfo, nullptr,
	                           &PresentSemaphore));
	VK_CHECK(vkCreateSemaphore(Device, &semaphoreCreateInfo, nullptr,
	                           &RenderSemaphore));

	DeletionQueue.pushFunction([](const VkContext &ctx) {
		vkDestroySemaphore(ctx.Device, ctx.PresentSemaphore, nullptr);
		vkDestroySemaphore(ctx.Device, ctx.RenderSemaphore, nullptr);
	});

	// also a fence for the immedateSubmit function

	VkFenceCreateInfo immediateSubmitFence = vk_init::fenceCreateInfo();

	VK_CHECK(vkCreateFence(Device, &immediateSubmitFence, nullptr,
	                       &_immediateSubmitContext.fence));
	DeletionQueue.pushFunction([](const VkContext &ctx) {
		vkDestroyFence(ctx.Device, ctx._immediateSubmitContext.fence, nullptr);
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

	VK_CHECK(vkCreateDescriptorPool(Device, &pool_info, nullptr,
	                                &GlobalDescriptorPool));

	DeletionQueue.pushFunction([](const VkContext &ctx) {
		vkDestroyDescriptorPool(ctx.Device, ctx.GlobalDescriptorPool, nullptr);
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

	VkDescriptorBindingFlags flags[2];
	flags[0] = 0;
	flags[1] = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;

	VkDescriptorSetLayoutBindingFlagsCreateInfoEXT extendedInfo{};
	extendedInfo.sType =
	    VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO_EXT;
	extendedInfo.pNext         = nullptr;
	extendedInfo.bindingCount  = 2;
	extendedInfo.pBindingFlags = flags;

	VkDescriptorSetLayoutCreateInfo layoutInfo = {};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.pNext = nullptr;
	layoutInfo.bindingCount = 2;
	layoutInfo.pBindings    = bindings;
	layoutInfo.flags        = 0;
	layoutInfo.pNext        = &extendedInfo;

	VK_CHECK(vkCreateDescriptorSetLayout(Device, &layoutInfo, nullptr,
	                                     &GlobalDescriptorSetLayout));

	DeletionQueue.pushFunction([](const VkContext &ctx) {
		vkDestroyDescriptorSetLayout(ctx.Device, ctx.GlobalDescriptorSetLayout,
		                             nullptr);
	});

	// CREATE QUADS BUFFER
	// -------------------

	QuadsBuffer =
	    Buffer(Allocator, INITIAL_QUADS_BUFFER_SIZE,
	           VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

	DeletionQueue.pushFunction([](const VkContext &ctx) {
		vmaDestroyBuffer(ctx.Allocator, ctx.QuadsBuffer.VulkanBuffer,
		                 ctx.QuadsBuffer.Allocation);
	});

	// ALLOCATE DESCRIPTOR SET
	// -----------------------

	VkDescriptorSetAllocateInfo allocateInfo = {};
	allocateInfo.pNext                       = nullptr;
	allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocateInfo.descriptorPool     = GlobalDescriptorPool;
	allocateInfo.descriptorSetCount = 1;
	allocateInfo.pSetLayouts        = &GlobalDescriptorSetLayout;

	VK_CHECK(
	    vkAllocateDescriptorSets(Device, &allocateInfo, &GlobalDescriptorSet));

	// UPDATE DESCRIPTOR SET TO POINT TO QUADS BUFFER
	// ----------------------------------------------

	VkDescriptorBufferInfo descriptorBufferInfo;
	descriptorBufferInfo.buffer = QuadsBuffer.VulkanBuffer;
	descriptorBufferInfo.offset = 0;
	descriptorBufferInfo.range  = INITIAL_QUADS_BUFFER_SIZE;

	VkWriteDescriptorSet setWriteBuffer = {};
	setWriteBuffer.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	setWriteBuffer.pNext           = nullptr;
	setWriteBuffer.dstBinding      = 0;
	setWriteBuffer.dstSet          = GlobalDescriptorSet;
	setWriteBuffer.descriptorCount = 1;
	setWriteBuffer.descriptorType  = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	setWriteBuffer.pBufferInfo     = &descriptorBufferInfo;

	vkUpdateDescriptorSets(Device, 1, &setWriteBuffer, 0, nullptr);
}

void VkContext::_initSampler() {
	VkSamplerCreateInfo samplerInfo = vk_init::samplerCreateInfo(
	    VK_FILTER_NEAREST, VK_SAMPLER_ADDRESS_MODE_REPEAT);

	VK_CHECK(vkCreateSampler(Device, &samplerInfo, nullptr, &GlobalSampler));

	DeletionQueue.pushFunction([](const VkContext &ctx) {
		vkDestroySampler(ctx.Device, ctx.GlobalSampler, nullptr);
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
	VkDescriptorSetLayout descriptorSetLayouts[] = {GlobalDescriptorSetLayout};
	VkPipelineLayoutCreateInfo pipeline_layout_info =
	    vk_init::pipelineLayoutCreateInfo(pushConstantRanges,
	                                      descriptorSetLayouts);

	VK_CHECK(vkCreatePipelineLayout(Device, &pipeline_layout_info, nullptr,
	                                &PipelineLayout));

	// BUILD PIPELINE
	// --------------

	PipelineBuilder pipelineBuilder;

	pipelineBuilder.ShaderStages = {
	    vk_init::pipelineShaderStageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT,
	                                           triangleVertShader.value()),
	    vk_init::pipelineShaderStageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT,
	                                           triangleFragShader.value())};

	pipelineBuilder.VertexInputInfo =
	    vk_init::pipelineVertexInputStateCreateInfo();

	pipelineBuilder.InputAssembly =
	    vk_init::pipelineInputAssemblyStateCreateInfo(
	        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

	pipelineBuilder.Rasterizer =
	    vk_init::pipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL);

	pipelineBuilder.Multisampling =
	    vk_init::pipelineMultisampleStateCreateInfo();

	pipelineBuilder.ColorBlendAttachment =
	    vk_init::pipelineColorBlendAttachmentState();

	// build viewport and scissor from the swapchain extents
	pipelineBuilder.Viewport.x        = 0.0f;
	pipelineBuilder.Viewport.y        = 0.0f;
	pipelineBuilder.Viewport.width    = (float)WindowExtent.width;
	pipelineBuilder.Viewport.height   = (float)WindowExtent.height;
	pipelineBuilder.Viewport.minDepth = 0.0f;
	pipelineBuilder.Viewport.maxDepth = 1.0f;
	pipelineBuilder.Scissor.offset    = {0, 0};
	pipelineBuilder.Scissor.extent    = WindowExtent;

	pipelineBuilder.PipelineLayout = PipelineLayout;

	// finally build the pipeline
	auto pipeline = pipelineBuilder.build(Device, RenderPass);
	if (pipeline) {
		Pipeline = pipeline.value();
	} else {
		throw std::runtime_error("Failed to create pipeline");
	}

	vkDestroyShaderModule(Device, triangleFragShader.value(), nullptr);
	vkDestroyShaderModule(Device, triangleVertShader.value(), nullptr);

	DeletionQueue.pushFunction([](const VkContext &ctx) {
		vkDestroyPipeline(ctx.Device, ctx.Pipeline, nullptr);
		vkDestroyPipelineLayout(ctx.Device, ctx.PipelineLayout, nullptr);
	});
}