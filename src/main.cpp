#include "azu.h"

int main() {
	auto context = azu::Context("Title", 800, 600);

	SDL_Event e;
	bool quit = false;

	while (!quit) {
		while (SDL_PollEvent(&e) != 0) {
			if (e.type == SDL_QUIT)
				quit = true;
		}

		context.beginDraw();
		context.endDraw();
	}

	return 0;
}