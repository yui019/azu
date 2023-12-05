#ifndef UTIL_GEOMETRY_H
#define UTIL_GEOMETRY_H

namespace azu {

struct Vec2 {
	float x;
	float y;

	static Vec2 zero() {
		return {0.0, 0.0};
	}

	static Vec2 create(float x, float y) {
		return {x, y};
	}

	static Vec2 create(int x, int y) {
		return {(float)x, (float)y};
	}
};

struct Vec3 {
	float x;
	float y;
	float z;

	static Vec3 zero() {
		return {0.0, 0.0, 0.0};
	}

	static Vec3 create(float x, float y, float z) {
		return {x, y, z};
	}

	static Vec3 create(int x, int y, int z) {
		return {(float)x, (float)y, (float)z};
	}
};

struct Quad {
	Vec2 pos;
	Vec2 size;

	static Quad create(Vec2 pos, Vec2 size) {
		return {pos, size};
	}

	static Quad create(float x, float y, float w, float h) {
		return {
		    {x, y},
            {w, h}
        };
	}

	static Quad create(int x, int y, int w, int h) {
		return {
		    {(float)x, (float)y},
            {(float)w, (float)h}
        };
	}
};

} // namespace azu

#endif // UTIL_GEOMETRY_H