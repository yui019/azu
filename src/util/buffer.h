#ifndef UTIL_BUFFER_H
#define UTIL_BUFFER_H

#include "vk_mem_alloc.h"
#include <vulkan/vulkan.h>

class Buffer {
  public:
	VkBuffer buffer;
	VmaAllocation allocation;

	Buffer() = default;

	Buffer(const VmaAllocator &allocator, uint32_t size,
	       VkBufferUsageFlags bufferUsage, VmaMemoryUsage memoryUsage);
};

#endif // UTIL_BUFFER_H