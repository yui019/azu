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