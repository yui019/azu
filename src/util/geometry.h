#ifndef UTIL_GEOMETRY_H
#define UTIL_GEOMETRY_H

namespace azu {

struct Vec2 {
	float x;
	float y;

	Vec2(float v = 0.0) : x(v), y(v) {}

	Vec2(float x, float y) : x(x), y(y) {}

	Vec2(int x, int y) : x((float)x), y((float)y) {}

	static Vec2 zero() {
		return {0, 0};
	}
};

struct Vec3 {
	float x;
	float y;
	float z;

	Vec3(float v = 0.0) : x(v), y(v), z(v) {}

	Vec3(float x, float y, float z) : x(x), y(y), z(z) {}

	Vec3(int x, int y, int z) : x((float)x), y((float)y), z((float)z) {}
};

struct Quad {
	Vec2 pos;
	Vec2 size;

	Quad(Vec2 pos, Vec2 size) : pos(pos), size(size) {}

	Quad(float x, float y, float w, float h) : pos({x, y}), size({w, h}) {}

	Quad(int x, int y, int w, int h)
	    : pos({(float)x, (float)y}), size({(float)w, (float)h}) {}
};

} // namespace azu

#endif // UTIL_GEOMETRY_H