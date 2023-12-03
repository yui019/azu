#include "vk_pipeline.h"

using namespace azu;

std::optional<VkPipeline> PipelineBuilder::build(VkDevice device,
                                                 VkRenderPass pass) {
	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.pNext = nullptr;
	viewportState.viewportCount = 1;
	viewportState.pViewports    = &viewport;
	viewportState.scissorCount  = 1;
	viewportState.pScissors     = &scissor;

	VkPipelineColorBlendStateCreateInfo colorBlending = {};
	colorBlending.sType =
	    VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.pNext             = nullptr;
	colorBlending.logicOpEnable     = VK_FALSE;
	colorBlending.logicOp           = VK_LOGIC_OP_COPY;
	colorBlending.attachmentCount   = 1;
	colorBlending.pAttachments      = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 1.0f;
	colorBlending.blendConstants[1] = 1.0f;
	colorBlending.blendConstants[2] = 1.0f;
	colorBlending.blendConstants[3] = 1.0f;

	// build the pipeline
	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType      = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.pNext      = nullptr;
	pipelineInfo.stageCount = shaderStages.size();
	pipelineInfo.pStages    = shaderStages.data();
	pipelineInfo.pVertexInputState   = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState      = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState   = &multisampling;
	pipelineInfo.pColorBlendState    = &colorBlending;
	pipelineInfo.layout              = pipelineLayout;
	pipelineInfo.renderPass          = pass;
	pipelineInfo.subpass             = 0;
	pipelineInfo.basePipelineHandle  = VK_NULL_HANDLE;

	VkPipeline newPipeline;
	if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo,
	                              nullptr, &newPipeline) != VK_SUCCESS) {
		// failed to create pipeline
		return std::nullopt;
	} else {
		return newPipeline;
	}
}