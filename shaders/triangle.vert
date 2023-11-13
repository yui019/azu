#version 450

void main() {
	// const array of positions for the triangle
	const vec3 positions[6] = vec3[6](vec3(-0.5f, -0.5f, 0.0f), // top left
	                                  vec3(0.5f, -0.5f, 0.0f),  // top right
	                                  vec3(-0.5f, 0.5f, 0.0f),  // bottom left

	                                  vec3(-0.5f, 0.5f, 0.0f), // bottom left
	                                  vec3(0.5f, 0.5f, 0.0f),  // bottom right
	                                  vec3(0.5f, -0.5f, 0.0f)  // top right
	);

	// output the position of each vertex
	gl_Position = vec4(positions[gl_VertexIndex], 1.0f);
}