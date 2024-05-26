#version 450

#extension GL_EXT_nonuniform_qualifier : enable

struct QuadCornerPoints {
	vec2 tl; // top left
	vec2 tr; // top right
	vec2 bl; // bottom left
	vec2 br; // bottom right
};

struct QuadCornerValues {
	float tl; // top left
	float tr; // top right
	float bl; // bottom left
	float br; // bottom right
};

struct DrawQuadOptions {
	QuadCornerValues radius;
	float opacity;
};

struct Quad {
	vec2 pos;
	vec2 size;
};

struct QuadData {
	Quad quad;
	vec4 color;
	int textureId;
	int fillType;
	DrawQuadOptions options;
};

layout(location = 0) in vec2 inUv;
layout(location = 1) flat in QuadData inQuadData;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 1) uniform sampler2D textureSamplers[];

#define FILL_TYPE_COLOR   1
#define FILL_TYPE_TEXTURE 2

vec4 composite(vec4 back, vec4 front) {
	return mix(back, front, front.a);
}

float fill_factor(float dist, float softness) {
	return smoothstep(-softness, softness, -dist);
}

vec4 apply_factor(vec4 color, float f) {
	return vec4(color.rgb, color.a * f);
}

bool between_noninclusive(float v, float from, float to) {
	return (v > from) && (v < to);
}

float sdf_quad(vec2 p, QuadCornerPoints quadPoints, QuadCornerValues radius,
               QuadCornerPoints radiusPoints) {
	if (between_noninclusive(p.x, quadPoints.tl.x, radiusPoints.tl.x) &&
	    between_noninclusive(p.y, radiusPoints.tl.y, quadPoints.tl.y)) {
		return length(p - radiusPoints.tl) - radius.tl;
	}

	if (between_noninclusive(p.x, radiusPoints.tr.x, quadPoints.tr.x) &&
	    between_noninclusive(p.y, radiusPoints.tr.y, quadPoints.tr.y)) {
		return length(p - radiusPoints.tr) - radius.tr;
	}

	if (between_noninclusive(p.x, quadPoints.bl.x, radiusPoints.bl.x) &&
	    between_noninclusive(p.y, quadPoints.bl.y, radiusPoints.bl.y)) {
		return length(p - radiusPoints.bl) - radius.bl;
	}

	if (between_noninclusive(p.x, radiusPoints.br.x, quadPoints.br.x) &&
	    between_noninclusive(p.y, quadPoints.br.y, radiusPoints.br.y)) {
		return length(p - radiusPoints.br) - radius.br;
	}

	if (between_noninclusive(p.x, quadPoints.tl.x, quadPoints.tr.x) &&
	    between_noninclusive(p.y, quadPoints.bl.y, quadPoints.tl.y)) {
		return -1.0;
	}

	return 1.0;
}

void main() {
	vec2 p = vec2(inUv.x - 0.5, inUv.y - 0.5);

	vec2 size = inQuadData.quad.size;
	if (size.x > size.y) {
		size = vec2(1.0, size.y / size.x);
	} else {
		size = vec2(size.x / size.y, 1.0);
	}

	QuadCornerPoints quadPoints;
	quadPoints.tl = vec2(-size.x / 2, size.y / 2);
	quadPoints.tr = vec2(size.x / 2, size.y / 2);
	quadPoints.bl = vec2(-size.x / 2, -size.y / 2);
	quadPoints.br = vec2(size.x / 2, -size.y / 2);

	QuadCornerValues radius;
	radius.tl = inQuadData.options.radius.tl * 0.5 * min(size.x, size.y);
	radius.tr = inQuadData.options.radius.tr * 0.5 * min(size.x, size.y);
	radius.bl = inQuadData.options.radius.bl * 0.5 * min(size.x, size.y);
	radius.br = inQuadData.options.radius.br * 0.5 * min(size.x, size.y);

	QuadCornerPoints radiusPoints;
	radiusPoints.tl =
	    vec2(quadPoints.tl.x + radius.tl, quadPoints.tl.y - radius.tl);
	radiusPoints.tr =
	    vec2(quadPoints.tr.x - radius.tr, quadPoints.tr.y - radius.tr);
	radiusPoints.bl =
	    vec2(quadPoints.bl.x + radius.bl, quadPoints.bl.y + radius.bl);
	radiusPoints.br =
	    vec2(quadPoints.br.x - radius.br, quadPoints.br.y + radius.br);

	if (inQuadData.fillType == FILL_TYPE_COLOR) {
		float dist      = sdf_quad(p, quadPoints, radius, radiusPoints);
		float f         = fill_factor(dist, 0.0025);
		vec4 foreground = vec4(inQuadData.color.rgb,
		                       inQuadData.color.a * inQuadData.options.opacity);
		outColor        = composite(vec4(inQuadData.color.rgb, 0.0), apply_factor(foreground, f));
	} else if (inQuadData.fillType == FILL_TYPE_TEXTURE) {
		float dist = sdf_quad(p, quadPoints, radius, radiusPoints);
		float f    = fill_factor(dist, 0.0025);
		vec4 textureColor =
		    texture(textureSamplers[nonuniformEXT(inQuadData.textureId)], inUv);
		vec4 foreground =
		    vec4(textureColor.rgb, textureColor.a * inQuadData.options.opacity);
		outColor = composite(vec4(0.0), apply_factor(foreground, f));
	} else {
		outColor = vec4(0.0, 0.0, 0.0, 1.0);
	}
}