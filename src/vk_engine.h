#pragma once

#include "expected.hpp"
#include "vk_types.h"
#include <SDL.h>
#include <optional>
#include <string>
#include <string_view>

class VulkanEngine {
	public:
		int frameNumber;

		static tl::expected<VulkanEngine, std::string>
		create(std::string_view title, uint32_t width, uint32_t height);

		VulkanEngine(const VulkanEngine &other)            = delete;
		VulkanEngine &operator=(const VulkanEngine &other) = delete;

		VulkanEngine(VulkanEngine &&other)
		    : frameNumber(other.frameNumber),
		      _windowExtent(other._windowExtent),
		      _window(std::exchange(other._window, nullptr)) {}

		VulkanEngine &operator=(VulkanEngine &&other) {
			std::swap(_window, other._window);
			return *this;
		}

		~VulkanEngine();

		// run main loop
		void run();

	private:
		VkExtent2D _windowExtent;
		SDL_Window *_window;

		VulkanEngine(
		    std::string_view title, uint32_t width, uint32_t height,
		    std::optional<std::string> &error
		);

		// draw loop
		void _draw();
};