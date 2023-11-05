#include "vk_engine.h"
#include <cstdio>

int main() {
	auto engine = VulkanEngine::create("Test", 800, 600);

	if (engine.has_value()) {
		engine->run();
	} else {
		printf("Error: %s\n", engine.error().c_str());
	}

	return 0;
}
