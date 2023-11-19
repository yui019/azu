#ifndef UTIL_H
#define UTIL_H

#include <cassert>

// Use (void) to silence unused warnings.
#define ASSERT(exp, msg) assert(((void)msg, exp))

#define VK_CHECK(x)                                                            \
	do {                                                                       \
		VkResult err = x;                                                      \
		if (err) {                                                             \
			throw err;                                                         \
		}                                                                      \
	} while (0)

#endif // UTIL_H