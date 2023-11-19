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

  public:
	VkContext vk;
	uint32_t frameNumber = 0;
	glm::mat4 projectionMatrix;

	Context(std::string_view title, uint32_t width, uint32_t height);

	~Context();
};

void run(Context &context);

} // namespace azu

#endif // AZU_H