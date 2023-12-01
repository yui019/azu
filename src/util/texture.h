#ifndef UTIL_TEXTURE_H
#define UTIL_TEXTURE_H

#include "vk_mem_alloc.h"
#include <vulkan/vulkan.h>
#include <cstdint>

class Texture {
  public:
	VkImage image;
	VmaAllocation allocation;
	VkImageView imageView;
	uint32_t width;
	uint32_t height;
};

#endif // UTIL_TEXTURE_H