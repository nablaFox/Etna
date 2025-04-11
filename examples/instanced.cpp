#include "shared.hpp"

constexpr float WINDOW_WIDTH{800};
constexpr float WINDOW_HEIGHT{600};
constexpr uint32_t INSTANCE_COUNT{25};

using namespace etna;

void initInstancedCube(MeshNode cube) {
	std::array<InstanceData, INSTANCE_COUNT> instances;

	constexpr uint32_t gridColumns = 5;
	constexpr uint32_t gridRows = 5;
	const float spacing = 2.0f;
	const float offsetX = ((gridColumns - 1) * spacing) / 2.0f;
	const float offsetY = ((gridRows - 1) * spacing) / 2.0f;

	for (uint32_t i = 0; i < INSTANCE_COUNT; i++) {
		uint32_t row = i / gridColumns;
		uint32_t col = i % gridColumns;

		float x = col * spacing - offsetX;
		float y = row * spacing - offsetY;

		instances[i].transform = Transform::getTransMatrix({x, y, 0.0f});

		float brightness =
			1.0f - (static_cast<float>(i) / static_cast<float>(INSTANCE_COUNT));
		instances[i].color = Color(1.0f, 1.0f, 1.0f, 1.0f) * brightness;
	}

	engine::immediateUpdate(cube->instanceBuffer, instances.data());
}

int main(void) {
	engine::init();

	Window window({
		.width = (uint32_t)WINDOW_WIDTH,
		.height = (uint32_t)WINDOW_HEIGHT,
		.title = "Instanced Rendering",
		.captureMouse = true,
	});

	Scene scene;

	CameraNode cameraNode = scene.createCameraNode({
		.name = "Main Camera",
		.transform = {.position = {0, 0, 10}},
	});

	MeshNode cube = scene.addMesh(createInstancedMesh({
		.name = "Cube",
		.mesh = engine::getCube(),
		.instanceCount = INSTANCE_COUNT,
	}));

	Renderer renderer({});

	initInstancedCube(cube);

	while (!window.shouldClose()) {
		window.pollEvents();

		engine::updateTime();

		firstPersonMovement(cameraNode, window);

		renderer.beginFrame(window);

		scene.render(renderer, cameraNode);

		renderer.endFrame();

		window.swapBuffers();
	}

	return 0;
}
