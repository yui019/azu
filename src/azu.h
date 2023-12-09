#ifndef AZU_H
#define AZU_H

#include "util/texture.h"
#include "util/geometry.h"
#include "util/color.h"
#include "util/quad_data.h"
#include "vk_context/vk_context.h"

#include "glm/glm.hpp"
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace azu {
class Context {
	SDL_Window *_window;
	VkContext _vk;
	glm::mat4 _projectionMatrix;

	uint32_t _swapchainImageIndex; // is set at the beginning of beginDraw and
	                               // used throughout the rendering loop

	std::vector<QuadData> _quadData; // cleared at beginDraw and gets new
	                                 // elements on every user call of drawQuad

	std::unordered_map<std::string, Texture>
	    _textures; // cleared at beginDraw and gets new
	               // elements on every user call of drawQuad

  public:
	uint32_t frameNumber = 0;

	Context(std::string_view title, uint32_t width, uint32_t height);

	void beginDraw();
	void endDraw();

	void drawQuad(Quad quad, Color color,
	              std::optional<DrawQuadOptions> options = std::nullopt);
	void drawQuad(Quad quad, const char *textureName,
	              std::optional<DrawQuadOptions> options = std::nullopt);

	bool createTextureFromFile(const char *name, const char *path);

	Vec2 getTextureDimensions(const char *name);

	~Context();
};

} // namespace azu

#endif // AZU_H