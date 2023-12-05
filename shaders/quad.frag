#version 450

#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) flat in int inFillType;
layout(location = 1) in vec4 inColor;
layout(location = 2) flat in int inTextureId;
layout(location = 3) in vec2 inUv;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 1) uniform sampler2D textureSamplers[];

#define FILL_TYPE_COLOR   1
#define FILL_TYPE_TEXTURE 2

void main() {
	if (inFillType == FILL_TYPE_COLOR) {
		outColor = inColor;
	} else if (inFillType == FILL_TYPE_TEXTURE) {
		outColor = texture(textureSamplers[nonuniformEXT(inTextureId)], inUv);
	} else {
		outColor = vec4(0.0, 0.0, 0.0, 1.0);
	}
}