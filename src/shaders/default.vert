#version 450
#extension GL_GOOGLE_include_directive : require

#include "scene.glsl"

layout(location = 0) out vec2 outUV;
layout(location = 1) out vec3 outNormal;

void main() {
    gl_Position = CAMERA.viewproj * pc.model * vec4(V.position, 1.0f);

    outUV = V.uv;
    outNormal = transpose(inverse(mat3(pc.model))) * V.normal;
}
