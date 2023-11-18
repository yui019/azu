#version 450

layout(push_constant) uniform constants {
	mat4 projectionMatrix;
}
pushConstants;

void main() {

	// const array of positions for the triangle
	const vec3 positions[6] =
	    vec3[6](vec3(300.0f, 300.0f, 0.0f), // top left
	            vec3(400.0f, 300.0f, 0.0f), // top right
	            vec3(300.0f, 400.0f, 0.0f), // bottom left

	            vec3(300.0f, 400.0f, 0.0f), // bottom left
	            vec3(400.0f, 400.0f, 0.0f), // bottom right
	            vec3(400.0f, 300.0f, 0.0f)  // top right
	    );

	// output the position of each vertex
	gl_Position =
	    pushConstants.projectionMatrix * vec4(positions[gl_VertexIndex], 1.0f);
}