#ifndef UTIL_COLOR_H
#define UTIL_COLOR_H

struct Color {
	float r;
	float g;
	float b;
	float a;

	static Color white() {
		return {1.0, 1.0, 1.0, 1.0};
	}

	static Color black() {
		return {0.0, 0.0, 0.0, 1.0};
	}

	static Color rgb(float r, float g, float b) {
		return {r, g, b, 1.0};
	}

	static Color rgba(float r, float g, float b, float a) {
		return {r, g, b, a};
	}
};

#endif // UTIL_COLOR_H