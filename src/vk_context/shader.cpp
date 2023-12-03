#include "vk_context.h"

#include <fstream>
#include <optional>
#include <stdexcept>

using namespace azu;

std::optional<VkShaderModule>
VkContext::_loadShaderModuleFromFile(const char *path) const {
	std::ifstream file(path, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		return std::nullopt;
	}

	size_t fileSize = (size_t)file.tellg();

	std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));

	// put file cursor at beginning
	file.seekg(0);
	// load the entire file into the buffer
	file.read((char *)buffer.data(), fileSize);
	// now that the file is loaded into the buffer, close it
	file.close();

	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.pNext = nullptr;
	createInfo.pCode = buffer.data();
	createInfo.codeSize =
	    buffer.size() *
	    sizeof(uint32_t); // codeSize has to be in bytes, so multiply the ints
	                      // in the buffer by size of int to know the real size
	                      // of the buffer

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(_device, &createInfo, nullptr, &shaderModule) !=
	    VK_SUCCESS) {
		return std::nullopt;
	}

	return shaderModule;
}