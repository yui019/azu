#ifndef UTIL_QUAD_DATA_H
#define UTIL_QUAD_DATA_H

#include "src/util/geometry.h"
#include "src/util/color.h"
#include <cstdint>

namespace azu {

struct QuadCornerValues {
	float top_left;
	float top_right;
	float bottom_left;
	float bottom_right;

	QuadCornerValues(float value = 0.0)
	    : top_left(value), top_right(value), bottom_left(value),
	      bottom_right(value) {}

	QuadCornerValues(float top_left, float top_right, float bottom_left,
	                 float bottom_right)
	    : top_left(top_left), top_right(top_right), bottom_left(bottom_left),
	      bottom_right(bottom_right) {}
};

struct DrawQuadOptions {
	QuadCornerValues radius;
	float opacity;

	DrawQuadOptions(const QuadCornerValues &radius, float opacity)
	    : radius(radius), opacity(opacity) {}

	DrawQuadOptions() : radius(QuadCornerValues()), opacity(1.0) {}
};

enum class QuadDataFillType {
	Color   = 1,
	Texture = 2
};

struct QuadData {
	Quad quad;
	Color color;
	uint32_t textureId;
	QuadDataFillType fillType;
	uint8_t __padding1[8];
	DrawQuadOptions options;
	uint8_t __padding2[12];

	QuadData(Quad quad, Color color, DrawQuadOptions options)
	    : quad(quad), color(color), textureId(0),
	      fillType(QuadDataFillType::Color), options(options) {}

	QuadData(Quad quad, uint32_t textureId, DrawQuadOptions options)
	    : quad(quad), color(Color::rgba(0.0, 0.0, 0.0, 0.0)),
	      textureId(textureId), fillType(QuadDataFillType::Texture),
	      options(options) {}
};

} // namespace azu

#endif // UTIL_QUAD_DATA_H