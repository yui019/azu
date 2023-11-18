#version 450

layout(location = 0) in vec3 inFragColor;

// output write
layout(location = 0) out vec4 outFragColor;

void main() {
	// return color
	outFragColor = vec4(inFragColor, 1.0f);
}