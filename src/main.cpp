#include "azu.h"
#include "util/geometry.h"
#include <cstdlib>
#include <ctime>
#include <vector>

float gravity = 0.5;
int maxX      = 0;
int maxY      = 0;
int minX      = 0;
int minY      = 0;

struct Bunny {
	azu::Vec2 position;
	azu::Vec2 velocity;
};

// https://github.com/HaxeFoundation/hxcpp/blob/master/src/hx/StdLibs.cpp#L190
double rand_scale = 1.0 / (1 << 16) / (1 << 16);
double rng() {
	unsigned int lo  = rand() & 0xfff;
	unsigned int mid = rand() & 0xfff;
	unsigned int hi  = rand() & 0xff;
	double result    = (lo | (mid << 12) | (hi << 24)) * rand_scale;
	return result;
}

int main() {
	const int screenWidth  = 800;
	const int screenHeight = 600;
	auto context = azu::Context("Bunnymark", screenWidth, screenHeight);

	context.createTextureFromFile("alien", "res/smug_alien.png");
	azu::Vec2 alienSize = context.getTextureDimensions("alien");
	alienSize.x         = alienSize.x / 30;
	alienSize.y         = alienSize.y / 30;

	maxX = screenWidth - alienSize.x;
	maxY = screenHeight - alienSize.y;

	std::vector<Bunny> bunnies;

	srand(time(0));

	SDL_Event e;
	bool quit = false;
	while (!quit) {
		while (SDL_PollEvent(&e) != 0) {
			if (e.type == SDL_QUIT)
				quit = true;
			else if (e.type == SDL_MOUSEBUTTONDOWN) {
				for (int i = 0; i < 100; i++) {
					Bunny bunny;
					bunny.position.x = 0;
					bunny.position.y = 0;
					bunny.velocity.x = rng() * 8;
					bunny.velocity.y = rng() * 5 - 2.5;

					bunnies.push_back(bunny);
				}
			}
		}

		for (size_t i = 0; i < bunnies.size(); i++) {
			bunnies[i].position.x += bunnies[i].velocity.x;
			bunnies[i].position.y += bunnies[i].velocity.y;
			bunnies[i].velocity.y += gravity;

			if (bunnies[i].position.x > maxX) {
				bunnies[i].velocity.x *= -1;
				bunnies[i].position.x = maxX;
			} else if (bunnies[i].position.x < minX) {
				bunnies[i].velocity.x *= -1;
				bunnies[i].position.x = minX;
			}

			if (bunnies[i].position.y > maxY) {
				bunnies[i].velocity.y *= -0.8;
				bunnies[i].position.y = maxY;

				if (rng() > 0.5)
					bunnies[i].velocity.y -= 3 + rng() * 4;

			} else if (bunnies[i].position.y < minY) {
				bunnies[i].velocity.y = 0;
				bunnies[i].position.y = minY;
			}
		}

		context.beginDraw();

		for (size_t i = 0; i < bunnies.size(); i++) {
			context.drawQuad(azu::Quad::create(bunnies[i].position, alienSize),
			                 "alien");
		}

		printf("bunnies: %zu\n", bunnies.size());

		context.endDraw();
	}

	return 0;
}