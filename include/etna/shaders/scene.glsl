#extension GL_GOOGLE_include_directive : require

#include "etna.glsl"

#define MAX_LIGHTS 16

DEF_UBO(SceneLights, {
	uint lights[MAX_LIGHTS];
});

DEF_UBO(SceneData, {
	vec4 ambientColor;
	uint lightsBuffer;
	uint lightCount;
});

#define SCENE (uSceneData[pc.buff1])

#define CAMERA (uCameraData[pc.buff2])

#define LIGHTS (uSceneLights[SCENE.lightsBuffer])

#define LIGHT(i) (uDirectionalLights[LIGHTS.lights[i]])

#define AMBIENT (SCENE.ambientColor)

vec4 lighten(vec4 color, vec3 normal) {
	vec3 N = normalize(normal);

	vec4 outColor = AMBIENT * color;

    for (uint i = 0; i < SCENE.lightCount; i++) {
        vec3 lightDir = normalize(-LIGHT(i).direction);
        float diff = max(dot(N, lightDir), 0.0);
        outColor += color * (diff * LIGHT(i).intensity * LIGHT(i).color);
    }

	return outColor;
}

