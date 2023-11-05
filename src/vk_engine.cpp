#include "vk_engine.h"

#include "vk_initializers.h"

VulkanEngine::VulkanEngine(
    std::string_view title, uint32_t width, uint32_t height,
    std::optional<std::string> &error
) {
	frameNumber   = 0;
	_windowExtent = {width, height};

	// We initialize SDL and create a window with it.
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		error = SDL_GetError();
		return;
	}

	// create blank SDL window for our application
	_window = SDL_CreateWindow(
	    title.data(),            // window title
	    SDL_WINDOWPOS_UNDEFINED, // window position x (don't care)
	    SDL_WINDOWPOS_UNDEFINED, // window position y (don't care)
	    width,                   // window width in pixels
	    height,                  // window height in pixels
	    SDL_WINDOW_VULKAN
	);

	if (_window == NULL) {
		error = SDL_GetError();
		return;
	}
}

tl::expected<VulkanEngine, std::string>
VulkanEngine::create(std::string_view title, uint32_t width, uint32_t height) {
	std::optional<std::string> error = std::nullopt;
	VulkanEngine engine(title, width, height, error);

	if (error) {
		return tl::unexpected(error.value());
	}

	return std::move(engine);
}

VulkanEngine::~VulkanEngine() {
	SDL_DestroyWindow(_window);
}

void VulkanEngine::run() {
	SDL_Event e;
	bool shouldQuit = false;

	// main loop
	while (!shouldQuit) {
		frameNumber++;

		// Handle events on queue
		while (SDL_PollEvent(&e) != 0) {
			// close the window when user clicks the X button or alt-f4s
			if (e.type == SDL_QUIT)
				shouldQuit = true;
		}

		_draw();
	}
}

void VulkanEngine::_draw() {
	// nothing yet
}