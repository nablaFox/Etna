#pragma once

#include "engine.hpp"
#include "etna/etna_core.hpp"
#include "scene.hpp"

using namespace etna;

struct FirstPersonMovementOpts {
	bool flyAround{true};
	float cameraSpeed{6.f};
	float sensitivity{0.001f};
};

inline void firstPersonMovement(CameraNode camera,
								Window& window,
								const FirstPersonMovementOpts opts = {}) {
	Transform transform = camera->getTransform();

	transform.yaw += window.mouseDeltaX() * opts.sensitivity;
	transform.pitch += window.mouseDeltaY() * opts.sensitivity;

	if (transform.pitch > M_PI / 2) {
		transform.pitch = M_PI / 2;
	} else if (transform.pitch < -M_PI / 2) {
		transform.pitch = -M_PI / 2;
	}

	Vec3& position = transform.position;
	float yaw = -transform.yaw;

	const Vec3 playerForward =
		opts.flyAround ? transform.forward() : Vec3{sinf(yaw), 0, cosf(yaw)} * -1;

	const Vec3 playerRight =
		opts.flyAround ? transform.right() : Vec3{cosf(yaw), 0, -sinf(yaw)};

	const Vec3 up = opts.flyAround ? transform.up() : Vec3{0, 0, 0};

	float dt = engine::getDeltaTime();

	if (window.isKeyPressed(GLFW_KEY_0)) {
		position = {0, 1, 0};
	}

	if (window.isKeyPressed(GLFW_KEY_W)) {
		position += playerForward * opts.cameraSpeed * dt;
	}

	if (window.isKeyPressed(GLFW_KEY_S)) {
		position -= playerForward * opts.cameraSpeed * dt;
	}

	if (window.isKeyPressed(GLFW_KEY_D)) {
		position += playerRight * opts.cameraSpeed * dt;
	}

	if (window.isKeyPressed(GLFW_KEY_A)) {
		position -= playerRight * opts.cameraSpeed * dt;
	}

	if (window.isKeyPressed(GLFW_KEY_SPACE)) {
		position += up * opts.cameraSpeed * dt;
	}

	if (window.isKeyPressed(GLFW_KEY_LEFT_SHIFT)) {
		position -= up * opts.cameraSpeed * dt;
	}

	if (window.isKeyPressed(GLFW_KEY_ESCAPE)) {
		window.setCaptureMouse(false);
	}

	if (window.isKeyPressed(GLFW_KEY_ENTER)) {
		window.setCaptureMouse(true);
	}

	camera->updateTransform(transform);
}

struct FloorCreateInfo {
	Color color{PURPLE.setAlpha(0.3)};
	float gridSize{1};
	float lineThickness{0.02};
	float height{0};
	float floorScale{1000};
};

inline SceneNode createFloor(const FloorCreateInfo& info) {
	MeshHandle mesh = engine::getQuad();

	MaterialHandle mainGridMaterial = engine::createTransparentGridMaterial({
		.color = INVISIBLE,
		.gridColor = info.color,
		.gridSpacing = info.gridSize / info.floorScale,
		.thickness = info.lineThickness / info.floorScale,
	});

	MaterialHandle subGridMaterial = engine::createTransparentGridMaterial({
		.color = INVISIBLE,
		.gridColor = info.color * 0.8,
		.gridSpacing = info.gridSize / (info.floorScale * 2.f),
		.thickness = info.lineThickness / info.floorScale,
	});

	Transform transform{
		.position = {0, info.height, 0},
		.pitch = -M_PI / 2,
		.scale = Vec3(info.floorScale),
	};

	SceneNode floor = scene::createRoot("Floor", transform);

	floor->add(scene::createMeshNode({
		.name = "MainGrid",
		.mesh = mesh,
		.material = mainGridMaterial,
	}));

	floor->add(scene::createMeshNode({
		.name = "SubGrid",
		.mesh = mesh,
		.material = subGridMaterial,
	}));

	return floor;
}

struct OutlineMaterialParams {
	Color color{WHITE};
	Color outline{BLACK};
	float thickness{0.01f};
};

struct OutlinedBrickCreateInfo {
	Color color{WHITE};
	Color outlineColor{};
	std::string name{"OutlinedBrick"};
	float outlineThickness{0.01};
	float width{1};
	float height{1};
	float depth{1};
	bool transparent{false};
};

inline MeshNode createOutlinedBrick(const OutlinedBrickCreateInfo& info) {
	static MaterialTemplateHandle g_outlineTemplate{nullptr};
	static MaterialTemplateHandle g_outlineTemplateTransparent{nullptr};

	if (g_outlineTemplate == nullptr) {
		g_outlineTemplate = MaterialTemplate::create({
			.shaders = {"default.vert", "examples/brick_outline.frag"},
			.paramsSize = sizeof(OutlineMaterialParams),
		});

		engine::queueForDeletion([=] { g_outlineTemplate.reset(); });
	}

	// PONDER: maybe enable dynamic transparency
	if (g_outlineTemplateTransparent == nullptr && info.transparent) {
		g_outlineTemplateTransparent = MaterialTemplate::create({
			.shaders = {"default.vert.spv", "examples/brick_outline.frag.spv"},
			.paramsSize = sizeof(OutlineMaterialParams),
			.transparency = true,
		});

		engine::queueForDeletion([=] { g_outlineTemplateTransparent.reset(); });
	}

	MeshHandle cube = engine::getCube();

	const OutlineMaterialParams outlineParams{
		.color = info.color,
		.outline = info.outlineColor,
		.thickness = info.outlineThickness,
	};

	MaterialHandle material = Material::create({
		.templateHandle =
			info.transparent ? g_outlineTemplateTransparent : g_outlineTemplate,
		.params = &outlineParams,
	});

	Transform transform{
		.scale = Vec3(info.width, info.height, info.depth),
	};

	return scene::createMeshNode({
		.name = info.name,
		.mesh = cube,
		.transform = transform,
		.material = material,
	});
}

struct OutlinedCubeCreateInfo {
	Color color{WHITE};
	Color outlineColor{};
	std::string name{"OutlinedCube"};
	float outlineThickness{0.01};
	float size{1};
};

inline MeshNode createBox(const OutlinedCubeCreateInfo& info) {
	return createOutlinedBrick({
		.color = info.color,
		.outlineColor = info.outlineColor,
		.name = info.name,
		.outlineThickness = info.outlineThickness,
		.width = info.size,
		.height = info.size,
		.depth = info.size,
		.transparent = true,
	});
}

struct InstanceData {
	Mat4 transform;
	Color color;
};

struct InstanceMeshCreateInfo {
	std::string name{"InstancedMesh"};
	MeshHandle mesh{nullptr};
	uint32_t instanceCount{0};
};

inline MeshNode createInstancedMesh(const InstanceMeshCreateInfo& info) {
	static MaterialHandle g_instancedMaterial{nullptr};

	if (g_instancedMaterial == nullptr) {
		g_instancedMaterial = Material::create({
			.shaders = {"examples/instanced.vert", "examples/instanced.frag"},
		});

		engine::queueForDeletion([=] { g_instancedMaterial.reset(); });
	}

	return scene::createMeshNode({
		.name = "InstancedMesh",
		.mesh = info.mesh,
		.material = g_instancedMaterial,
		.instanceBuffer = engine::getDevice().createSSBO(info.instanceCount *
														 sizeof(InstanceData)),
		.instanceCount = info.instanceCount,
	});
}

struct PhysicalObject {
	float mass{1};
	Vec3 force{};
	Vec3 acc{};
	Vec3 vel{};
	Vec3 pos{};
	Vec3 prevPos{};
	float timeAccumulator{0};
	float timeStep{1.f / 60.f};

	void update() {
		float dt = engine::getDeltaTime();

		if (dt > 0.25f) {
			dt = 0.25f;
		}

		timeAccumulator += dt;

		while (timeAccumulator >= timeStep) {
			prevPos = pos;

			Vec3 oldAcc = acc;

			pos += vel * timeStep + oldAcc * square(timeStep) * 0.5f;

			Vec3 newAcc = force / mass;

			vel += (oldAcc + newAcc) * timeStep * 0.5f;

			timeAccumulator -= timeStep;
		}
	}

	Vec3 getRenderPosition(float wuPerMeter = 1.f) {
		float alpha = timeAccumulator / timeStep;
		Vec3 interPos = pos * alpha + prevPos * (1.0f - alpha);
		return interPos * wuPerMeter;
	}
};
