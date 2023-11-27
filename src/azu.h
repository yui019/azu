#ifndef AZU_H
#define AZU_H

#include "vk_context/vk_context.h"

#include "glm/glm.hpp"
#include <cstdint>
#include <optional>
#include <string>

namespace azu {
class Context {
	SDL_Window *_window;
	VkContext _vk;
	glm::mat4 _projectionMatrix;
	uint32_t _swapchainImageIndex;

  public:
	uint32_t frameNumber = 0;

	Context(std::string_view title, uint32_t width, uint32_t height);

	void beginDraw();
	void endDraw();

	~Context();
};

} // namespace azu

#endif // AZU_H