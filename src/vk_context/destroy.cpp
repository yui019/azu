#include "vk_context.h"

#include "VkBootstrap.h"

using namespace azu;

VkContext::~VkContext() {
	// if the VkInstance is set to nullptr, it's probably a VkContext left in an
	// invalid state after being moved, so the destructor shouldn't run
	if (Instance) {
		vkWaitForFences(Device, 1, &RenderFence, true, 1000000000);

		// unmap quads buffer memory before destroying it
		vmaUnmapMemory(Allocator, QuadsBuffer.Allocation);

		DeletionQueue.flush(*this);

		vkDestroyDevice(Device, nullptr);
		vkDestroySurfaceKHR(Instance, Surface, nullptr);
		vkb::destroy_debug_utils_messenger(Instance, DebugMessenger);
		vkDestroyInstance(Instance, nullptr);
	}
}
