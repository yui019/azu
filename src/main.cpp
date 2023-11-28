#include "azu.h"
#include "src/util/geometry.h"

int main() {
	auto context = azu::Context("Title", 800, 600);

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

		if (mousex > 400) {
			context.drawQuad(Quad::create(mousex, mousey, 50, 50),
			                 Color::rgb(1.0, 0.0, 1.0));
		} else {
			context.drawQuad(Quad::create(mousex, mousey, 50, 50),
			                 Color::rgb(0.0, 1.0, 0.0));
		}

		context.endDraw();
	}

	return 0;
}