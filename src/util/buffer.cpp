#include "buffer.h"
#include "../vk_init/vk_init.h"
#include "util.h"

using namespace azu;

Buffer::Buffer(const VmaAllocator &allocator, uint32_t size,
               VkBufferUsageFlags bufferUsage, VmaMemoryUsage memoryUsage) {
	VkBufferCreateInfo bufferInfo =
	    vk_init::bufferCreateInfo(size, bufferUsage);

	VmaAllocationCreateInfo vmaallocInfo = {};
	vmaallocInfo.usage                   = memoryUsage;

	VK_CHECK(vmaCreateBuffer(allocator, &bufferInfo, &vmaallocInfo, &buffer,
	                         &allocation, nullptr));

	vmaMapMemory(allocator, allocation, &data);
}