#ifndef VK_PIPELINE_H
#define VK_PIPELINE_H

#include <vulkan/vulkan.h>
#include <vector>
#include <optional>

namespace azu {

struct PipelineBuilder {
	std::vector<VkPipelineShaderStageCreateInfo> ShaderStages;
	VkPipelineVertexInputStateCreateInfo VertexInputInfo;
	VkPipelineInputAssemblyStateCreateInfo InputAssembly;
	VkViewport Viewport;
	VkRect2D Scissor;
	VkPipelineRasterizationStateCreateInfo Rasterizer;
	VkPipelineColorBlendAttachmentState ColorBlendAttachment;
	VkPipelineMultisampleStateCreateInfo Multisampling;
	VkPipelineLayout PipelineLayout;

	std::optional<VkPipeline> build(VkDevice device, VkRenderPass pass);
};

} // namespace azu

#endif // VK_PIPELINE_H