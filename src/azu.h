#include "vk_context.h"

#include <optional>
#include <string>

namespace azu {
class Context {
  public:
	SDL_Window *window;
	VkContext vkContext;

	Context(std::string_view title, uint32_t width, uint32_t height);

	~Context();
};

void run(Context &context);

} // namespace azu