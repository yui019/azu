#ifndef UTIL_BUFFER_H
#define UTIL_BUFFER_H

#include "vk_mem_alloc.h"
#include <vulkan/vulkan.h>

namespace azu {

class Buffer {
  public:
	VkBuffer VulkanBuffer;
	VmaAllocation Allocation;

	void *Data;

	Buffer() = default;

	Buffer(const VmaAllocator &allocator, uint32_t size,
	       VkBufferUsageFlags bufferUsage, VmaMemoryUsage memoryUsage);
};

} // namespace azu

#endif // UTIL_BUFFER_H