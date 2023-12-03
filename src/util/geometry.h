#ifndef UTIL_GEOMETRY_H
#define UTIL_GEOMETRY_H

namespace azu {

struct Vec2 {
	float x;
	float y;
};

struct Vec3 {
	float x;
	float y;
	float z;
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