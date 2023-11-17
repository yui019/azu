#include "../vk_context/vk_context.h"
#include "vk_mem_alloc.h"
#include <vulkan/vulkan.h>

class Buffer {
  public:
	VkBuffer buffer;
	VmaAllocation allocation;

	Buffer(const VmaAllocator &allocator, uint32_t size,
	       VkBufferUsageFlags bufferUsage, VmaMemoryUsage memoryUsage);
};