#include "shared.hpp"

using namespace etna;

constexpr float WINDOW_WIDTH{800};
constexpr float WINDOW_HEIGHT{600};

int main(void) {
	engine::init();

	Window window1({
		.width = (uint32_t)WINDOW_WIDTH,
		.height = (uint32_t)WINDOW_HEIGHT,
		.title = "Etna Window 1",
		.captureMouse = true,
	});

	Window window2({
		.width = (uint32_t)WINDOW_WIDTH,
		.height = (uint32_t)WINDOW_HEIGHT,
		.title = "Etna Window 2",
	});

	Scene scene;

	scene.createMeshNode({
		.name = "Brick",
		.mesh = engine::getCube(),
		.transform = {.scale = {1.f, 1.f, 2.f}},
	});

	scene.addNode(createFloor({}));

	CameraNode camera = scene.createCameraNode({
		.name = "Main Camera",
		.transform = {.position = {0, 0.5, 3}},
	});

	Renderer renderer({});

	bool shouldClose{false};

	while (!shouldClose) {
		window1.pollEvents();
		window2.pollEvents();

		engine::updateTime();

		firstPersonMovement(camera, window1);

		if (window1.shouldClose() || window2.shouldClose()) {
			shouldClose = true;
		}

		renderer.beginFrame(window1);

		scene.render(renderer, camera, {.viewport = {.width = WINDOW_WIDTH / 2.f}});

		scene.render(
			renderer, camera,
			{.viewport = {.x = WINDOW_WIDTH / 2.f, .width = WINDOW_WIDTH / 2.f}});

		renderer.endFrame();

		renderer.beginFrame(window2);

		scene.render(renderer, camera);

		renderer.endFrame();

		window1.swapBuffers();

		window2.swapBuffers();
	}

	return 0;
}
