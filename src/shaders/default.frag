#version 450
#extension GL_GOOGLE_include_directive : require

#include "scene.glsl"

layout (location = 0) out vec4 outColor;
layout (location = 1) in vec3 inNormal;

DEF_MATERIAL({
    vec4 color;
});

void main() {
	outColor = lighten(MATERIAL.color, inNormal);
}
