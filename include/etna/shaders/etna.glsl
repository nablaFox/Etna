#extension GL_EXT_buffer_reference : require
#extension GL_EXT_nonuniform_qualifier : require

#define STORAGE_BUFFER_BINDING 0
#define UNIFORM_BINDING 1

#define DEF_UBO(Name, Struct) \
 layout(set = 0, binding = UNIFORM_BINDING) \
 uniform Name Struct u##Name[]

#define DEF_SSBO(Name, Struct) \
 layout(std430, set = 0, binding = STORAGE_BUFFER_BINDING) \
 readonly buffer Name Struct b##Name[]

layout(push_constant) uniform constants {
	mat4 model;
	uint vertices;
	uint material;
	uint instanceBuff;
	uint buff1;
	uint buff2;
	uint buff3;
} pc;

// Vertices
struct Vertex {
    vec3 position;
	vec3 normal;
	vec2 uv;
};

DEF_SSBO(VertexBuffer, {
	Vertex vertices[];
});

#define V (bVertexBuffer[pc.vertices].vertices[gl_VertexIndex])

// Material
#define DEF_MATERIAL(Struct) DEF_UBO(Material, Struct)

#define MATERIAL (uMaterial[pc.material])

// Instanced
#define DEF_INSTANCE_DATA(Struct) \
layout(std430, set = 0, binding = STORAGE_BUFFER_BINDING) readonly buffer InstanceDataBuffer { \
    Struct instances[]; \
} bInstanceDataBuffer[] \

#define I (bInstanceDataBuffer[pc.instanceBuff].instances[gl_InstanceIndex])

// Camera
DEF_UBO(CameraData, {
	mat4 viewproj;
	mat4 view;
	mat4 proj;
});

// Lights
DEF_UBO(DirectionalLights, {
	vec3 direction;
	float intensity;
	vec4 color;
});
