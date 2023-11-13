#include "vk_context.h"

#include "VkBootstrap.h"

VkContext::~VkContext() {
	// if the VkInstance is set to nullptr, it's probably a VkContext left in an
	// invalid state after being moved, so the destructor shouldn't run
	if (_instance) {
		vkWaitForFences(_device, 1, &_renderFence, true, 1000000000);

		_deletionQueue.flush(*this);

		vkDestroyDevice(_device, nullptr);
		vkDestroySurfaceKHR(_instance, _surface, nullptr);
		vkb::destroy_debug_utils_messenger(_instance, _debug_messenger);
		vkDestroyInstance(_instance, nullptr);
	}
}
