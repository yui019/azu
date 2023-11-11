#include "azu.h"
#include "SDL_error.h"
#include <vulkan/vulkan.h>

using namespace azu;

Context::Context(std::string_view title, uint32_t width, uint32_t height) {
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		throw SDL_GetError();
	}

	window = SDL_CreateWindow(
	    title.data(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width,
	    height, SDL_WINDOW_VULKAN
	);
	if (window == NULL)
		throw SDL_GetError();

	vkContext = VkContext(window, VkExtent2D{width, height}, true);
}

Context::~Context() {
	SDL_DestroyWindow(window);
}

void azu::run(Context &context) {
	SDL_Event e;
	bool bQuit = false;

	// main loop
	while (!bQuit) {
		// Handle events on queue
		while (SDL_PollEvent(&e) != 0) {
			// close the window when user alt-f4s or clicks the X button
			if (e.type == SDL_QUIT)
				bQuit = true;
		}

		///
	}
}