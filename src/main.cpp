#include "azu.h"
#include "util/geometry.h"
#include "util/color.h"

int main() {
	using namespace azu;

	const int screenWidth  = 800;
	const int screenHeight = 600;
	auto context           = Context("Test", screenWidth, screenHeight);

	context.createTextureFromFile("akkarin", "res/akkarin.jpg");

	SDL_Event e;
	bool quit = false;
	while (!quit) {
		while (SDL_PollEvent(&e) != 0) {
			if (e.type == SDL_QUIT)
				quit = true;
		}

		context.beginDraw();

		context.drawQuad(Quad::create(0, 0, 100, 100), "akkarin");

		context.endDraw();
	}

	return 0;
}