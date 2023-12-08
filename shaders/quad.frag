#version 450

#extension GL_EXT_nonuniform_qualifier : enable

struct Quad {
	vec2 pos;
	vec2 size;
};

struct QuadData {
	Quad quad;
	vec4 color;
	int textureId;
	int fillType;
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

float sdf_quad(vec2 p, vec4 r) {
	vec2 b = vec2(0.5);
	p      = p - vec2(0.5, 0.5);

	r.xy   = (p.x > 0.0) ? r.xy : r.zw;
	r.x    = (p.y > 0.0) ? r.x : r.y;
	vec2 q = abs(p) - b + r.x;
	return min(max(q.x, q.y), 0.0) + length(max(q, 0.0)) - r.x;

	vec2 d = abs(p) - b;
	return length(max(d, 0.0)) + min(max(d.x, d.y), 0.0);
}

void main() {
	vec4 radius = vec4(0.25, 0.25, 0.25, 0.0);
	if (inQuadData.fillType == FILL_TYPE_COLOR) {
		float dist = sdf_quad(inUv, radius);
		float f    = fill_factor(dist, 0.0025);
		outColor   = composite(vec4(0.0, 0.0, 0.0, 0.0),
		                       apply_factor(inQuadData.color, f));
	} else if (inQuadData.fillType == FILL_TYPE_TEXTURE) {
		outColor =
		    texture(textureSamplers[nonuniformEXT(inQuadData.textureId)], inUv);
	} else {
		outColor = vec4(0.0, 0.0, 0.0, 1.0);
	}
}