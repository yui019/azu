#version 450

layout(push_constant) uniform constants {
	mat4 projectionMatrix;
}
pushConstants;

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

layout(location = 0) out vec2 outUv;
layout(location = 1) out QuadData outQuadData;

layout(std140, set = 0, binding = 0) readonly buffer QuadsBuffer {
	QuadData quads[];
}
quadsBuffer;

void main() {
	QuadData d = quadsBuffer.quads[gl_VertexIndex / 6];

	float x = d.quad.pos.x;
	float y = d.quad.pos.y;
	float w = d.quad.size.x;
	float h = d.quad.size.y;

	vec2 tl = vec2(x, y);
	vec2 tr = vec2(x + w, y);
	vec2 bl = vec2(x, y + h);
	vec2 br = vec2(x + w, y + h);

	if (w > h) {
		tl.y = y + h / 2 - w / 2;
		tr.y = y + h / 2 - w / 2;
		bl.y = y + h - h / 2 + w / 2;
		br.y = y + h - h / 2 + w / 2;
	} else {
		tl.x = x + w / 2 - h / 2;
		bl.x = x + w / 2 - h / 2;
		tr.x = x + w - w / 2 + h / 2;
		br.x = x + w - w / 2 + h / 2;
	}

	const vec3 vertices[6] = vec3[6](vec3(tl, 0.0), // top left
	                                 vec3(tr, 0.0), // top right
	                                 vec3(bl, 0.0), // bottom left

	                                 vec3(bl, 0.0), // bottom left
	                                 vec3(br, 0.0), // bottom right
	                                 vec3(tr, 0.0)  // top right
	);

	const vec2 uv[6] = vec2[6](vec2(0.0, 0.0), // top left
	                           vec2(1.0, 0.0), // top right
	                           vec2(0.0, 1.0), // bottom left

	                           vec2(0.0, 1.0), // bottom left
	                           vec2(1.0, 1.0), // bottom right
	                           vec2(1.0, 0.0)  // top right
	);

	// output the position of each vertex
	gl_Position = pushConstants.projectionMatrix *
	              vec4(vertices[gl_VertexIndex % 6], 1.0f);
	outUv       = uv[gl_VertexIndex % 6];
	outQuadData = d;
}