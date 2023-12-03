#ifndef UTIL_QUAD_DATA_H
#define UTIL_QUAD_DATA_H

#include "src/util/geometry.h"
#include "src/util/color.h"
#include <cstdint>

namespace azu {

enum class QuadDataFillType {
	Color   = 1,
	Texture = 2
};

struct QuadData {
	Quad quad;
	Color color;
	uint32_t textureId;
	QuadDataFillType fillType;

	char _padding[8];
};

} // namespace azu

#endif // UTIL_QUAD_DATA_H