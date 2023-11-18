#version 450

layout(push_constant) uniform constants {
	mat4 projectionMatrix;
}
pushConstants;

layout(location = 0) out vec3 outFragColor;

struct QuadData {
	vec3 pos;
	vec2 size;
	vec3 color;
};

const QuadData QUADS[] = {
    {vec3(5.0,   5.0,  0), vec2(100, 70), vec3(1.0, 0.0, 0.0)},
    {vec3(110.0, 5.0,  0), vec2(100, 70), vec3(0.0, 1.0, 0.0)},
    {vec3(5.0,   80.0, 0), vec2(100, 70), vec3(0.0, 0.0, 1.0)},
    {vec3(120.0, 90.0, 0), vec2(100, 70), vec3(1.0, 1.0, 1.0)},
};

void main() {
	QuadData q = QUADS[gl_VertexIndex / 6];

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
	outFragColor = q.color;
}