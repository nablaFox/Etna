#include "shared.hpp"

constexpr uint32_t WINDOW_WIDTH{800};
constexpr uint32_t WINDOW_HEIGHT{600};

int main(void) {
	engine::init();

	Window window({
		.width = WINDOW_WIDTH,
		.height = WINDOW_HEIGHT,
		.title = "Basic Etna",
		.captureMouse = true,
	});

	Scene scene;

	scene.createMeshNode({
		.name = "Sphere1",
		.mesh = engine::getSphere(),
		.transform =
			{
				.position = {1.5, 0.5, -5},
				.pitch = M_PI / 2,
				.scale = Vec3(0.5),
			},
		.material = engine::createGridMaterial({
			.color = BLUE * 0.08,
			.gridColor = BLUE,
			.gridSpacing = 0.1,
			.thickness = 0.005,
		}),
	});

	scene.createMeshNode({
		.name = "Sphere2",
		.mesh = engine::getSphere(),
		.transform = {.position = {0, 2.5, -9}},
		.material = engine::createPointMaterial(GREEN),
	});

	scene.addNode(createOutlinedBrick({}),
				  {.position = {-2.5, 0.5, -5}, .yaw = M_PI / 4});

	scene.addNode(createFloor({}));

	CameraNode playerCamera = scene.createCameraNode({
		.name = "PlayerCamera",
		.transform = {.position = {0, 1, 0}},
	});

	// rendering
	Renderer renderer({});

	while (!window.shouldClose()) {
		window.pollEvents();

		engine::updateTime();

		firstPersonMovement(playerCamera, window);

		renderer.beginFrame(window);

		scene.render(renderer, playerCamera);

		renderer.endFrame();

		window.swapBuffers();
	}

	return 0;
}
