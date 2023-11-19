#version 450

layout(push_constant) uniform constants {
	mat4 projectionMatrix;
}
pushConstants;

layout(location = 0) out vec3 outFragColor;

struct QuadData {
	vec4 pos;
	vec4 size;
	vec4 color;
};

layout(std140, set = 0, binding = 0) readonly buffer QuadsBuffer {
	QuadData quads[];
}
quadsBuffer;

void main() {
	QuadData q = quadsBuffer.quads[gl_VertexIndex / 6];

	// const array of positions for the triangle
	const vec3 positions[6] = vec3[6](
	    vec3(q.pos.x, q.pos.y, q.pos.z),            // top left
	    vec3(q.pos.x + q.size.x, q.pos.y, q.pos.z), // top right
	    vec3(q.pos.x, q.pos.y + q.size.y, q.pos.z), // bottom left

	    vec3(q.pos.x, q.pos.y + q.size.y, q.pos.z),            // bottom left
	    vec3(q.pos.x + q.size.x, q.pos.y + q.size.y, q.pos.z), // bottom right
	    vec3(q.pos.x + q.size.x, q.pos.y, q.pos.z)             // top right
	);

	// output the position of each vertex
	gl_Position = pushConstants.projectionMatrix *
	              vec4(positions[gl_VertexIndex % 6], 1.0f);
	outFragColor = q.color.rgb;
}