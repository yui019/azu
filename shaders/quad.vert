#version 450

layout(push_constant) uniform constants {
	mat4 projectionMatrix;
}
pushConstants;

layout(location = 0) out vec3 outColor;
layout(location = 1) out vec2 outUv;

struct Quad {
	vec2 pos;
	vec2 size;
};

struct QuadData {
	Quad quad;
	vec4 color;
};

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

	const vec3 vertices[6] = vec3[6](vec3(x, y, 0.0),     // top left
	                                 vec3(x + w, y, 0.0), // top right
	                                 vec3(x, y + h, 0.0), // bottom left

	                                 vec3(x, y + h, 0.0),     // bottom left
	                                 vec3(x + w, y + h, 0.0), // bottom right
	                                 vec3(x + w, y, 0.0)      // top right
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
	outUv    = uv[gl_VertexIndex % 6];
	outColor = d.color.rgb;
}