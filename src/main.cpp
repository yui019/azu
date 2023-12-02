#include "azu.h"
#include "src/util/geometry.h"

int main() {
	auto context = azu::Context("Title", 800, 600);

	context.createTextureFromFile("akkarin", "res/akkarin.jpg");
	context.createTextureFromFile("alien", "res/smug_alien.png");

	// TODO: this Vec2 (and VkContext and everything else) should be in the azu
	// namespace
	Vec2 akkarinSize = context.getTextureDimensions("akkarin");
	Vec2 alienSize   = context.getTextureDimensions("alien");

	SDL_Event e;
	bool quit = false;

	int mousex, mousey;

	while (!quit) {
		while (SDL_PollEvent(&e) != 0) {
			if (e.type == SDL_QUIT)
				quit = true;
		}

		context.beginDraw();

		SDL_GetMouseState(&mousex, &mousey);

		context.drawQuad(Quad::create(0.0, 0.0, akkarinSize.x, akkarinSize.y),
		                 "akkarin");

		context.drawQuad(Quad::create(0, 0, 50, 50), Color::rgb(1.0, 0.0, 0.0));

		context.drawQuad(Quad::create(0.0, 600.0 - alienSize.y / 3,
		                              alienSize.x / 3, alienSize.y / 3),
		                 "alien");

		context.drawQuad(Quad::create(mousex, mousey, 50, 50),
		                 Color::rgb(1.0, 0.0, 1.0));

		context.endDraw();
	}

	return 0;
}