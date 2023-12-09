#include "azu.h"
#include "util/quad_data.h"
#include "util/geometry.h"
#include "util/color.h"

int main() {
	using namespace azu;

	const int screenWidth  = 800;
	const int screenHeight = 600;
	auto context           = Context("Test", screenWidth, screenHeight);

	context.createTextureFromFile("akkarin", "res/akkarin.jpg");

	int width     = 300;
	int height    = 300;
	int growSpeed = 10.0;

	SDL_Event e;
	bool quit = false;
	while (!quit) {
		while (SDL_PollEvent(&e) != 0) {
			if (e.type == SDL_QUIT) {
				quit = true;
			} else if (e.type == SDL_KEYDOWN) {
				if (e.key.keysym.sym == SDLK_LEFT) {
					width -= growSpeed;
				} else if (e.key.keysym.sym == SDLK_RIGHT) {
					width += growSpeed;
				} else if (e.key.keysym.sym == SDLK_DOWN) {
					height -= growSpeed;
				} else if (e.key.keysym.sym == SDLK_UP) {
					height += growSpeed;
				}
			}
		}

		context.beginDraw();

		context.drawQuad(Quad::create(0, 0, 200, 200), "akkarin",
		                 DrawQuadOptions(QuadCornerValues(0.25), 1.0));
		context.drawQuad(
		    Quad::create(screenWidth / 2 - width / 2,
		                 screenHeight / 2 - height / 2, width, height),
		    Color::white(),
		    DrawQuadOptions(QuadCornerValues(0.0, 0.5, 0.5, 0.0), 1.0));

		context.endDraw();
	}

	return 0;
}