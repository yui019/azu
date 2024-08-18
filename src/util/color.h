#ifndef UTIL_COLOR_H
#define UTIL_COLOR_H

namespace azu {

struct Color {
	float r;
	float g;
	float b;
	float a;

	static Color rgb(float r, float g, float b) {
		return {r, g, b, 1.0};
	}

	static Color rgba(float r, float g, float b, float a) {
		return {r, g, b, a};
	}

	static Color red() {
		return {1.0, 0.0, 0.0, 1.0};
	}

	static Color green() {
		return {0.0, 1.0, 0.0, 1.0};
	}

	static Color blue() {
		return {0.0, 0.0, 1.0, 1.0};
	}

	static Color white() {
		return {1.0, 1.0, 1.0, 1.0};
	}

	static Color black() {
		return {0.0, 0.0, 0.0, 1.0};
	}
};

} // namespace azu

#endif // UTIL_COLOR_H