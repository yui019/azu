#ifndef AZU_H
#define AZU_H

#include "util/texture.h"
#include "util/geometry.h"
#include "util/color.h"
#include "util/quad_data.h"
#include "vk_context/vk_context.h"

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace azu {
class Context {
	float _projectionMatrix[4][4];

	uint32_t _swapchainImageIndex; // is set at the beginning of beginDraw and
	                               // used throughout the rendering loop

	std::vector<QuadData> _quadData; // cleared at beginDraw and gets new
	                                 // elements on every user call of drawQuad

	std::unordered_map<std::string, Texture>
	    _textures; // cleared at beginDraw and gets new
	               // elements on every user call of drawQuad

	void _calculateProjectionMatrix(float windowWidth, float windowHeight);

	void _handleResize();

  public:
	uint32_t FrameNumber = 0;

	SDL_Window *Window;
	VkContext Vk;

	Context(std::string_view title, uint32_t width, uint32_t height);

	void BeginDraw();
	void EndDraw();

	// Just a wrapper over calling BeginDraw and EndDraw
	template <typename F> void Draw(F f) {
		BeginDraw();
		f();
		EndDraw();
	}

	void DrawQuad(Quad quad, Color color,
	              std::optional<DrawQuadOptions> options = std::nullopt);
	void DrawQuad(Quad quad, const char *textureName,
	              std::optional<DrawQuadOptions> options = std::nullopt);

	bool CreateTextureFromFile(const char *name, const char *path);

	Vec2 GetTextureDimensions(const char *name);

	~Context();
};

} // namespace azu

#endif // AZU_H