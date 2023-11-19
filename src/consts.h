#ifndef VK_CONTEXT_CONSTS_H
#define VK_CONTEXT_CONSTS_H

#include "vk_context/quad_data.h"
#include "glm/glm.hpp"

const uint32_t INITIAL_QUADS_BUFFER_SIZE = sizeof(QuadData) * 10000;

const uint32_t QUAD_COUNT        = 2;
const QuadData QUADS[QUAD_COUNT] = {
    {glm::vec4(5,   5, 0, 0), glm::vec4(100, 70, 0, 0),
     glm::vec4(1.0, 0.0, 0.0, 0.0)},
    {glm::vec4(110, 5, 0, 0), glm::vec4(100, 70, 0, 0),
     glm::vec4(0.0, 1.0, 0.0, 0.0)},
};

#endif // VK_CONTEXT_CONSTS_H