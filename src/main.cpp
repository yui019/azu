#include "azu.h"
#include "src/util/geometry.h"

int main() {
	auto context = azu::Context("Title", 800, 600);

	context.loadTextureFromFile("res/akkarin.jpg");

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

		context.drawQuad(Quad::create(mousex, mousey, 257, 262),
		                 Color::rgb(1.0, 0.0, 1.0));

		context.endDraw();
	}

	return 0;
}