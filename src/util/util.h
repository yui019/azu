#ifndef UTIL_H
#define UTIL_H

#include <cassert>
#include <stdexcept>
#include <string>

// Use (void) to silence unused warnings.
#define ASSERT(exp, msg) assert(((void)msg, exp))

#define VK_CHECK(x)                                                            \
	do {                                                                       \
		VkResult err = x;                                                      \
		if (err) {                                                             \
			throw std::runtime_error(std::string("Vulkan error, code: ") +     \
			                         std::to_string(err));                     \
		}                                                                      \
	} while (0)

#endif // UTIL_H