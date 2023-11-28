#include "vk_context.h"

void VkContext::fillQuadsBuffer(tcb::span<QuadData> quadData) {
	memset(_quadsBuffer.data, 0, INITIAL_QUADS_BUFFER_SIZE);
	memcpy(_quadsBuffer.data, quadData.data(), quadData.size_bytes());
}