#include "shared.hpp"

using namespace etna;

constexpr uint32_t WINDOW_WIDTH = 800;
constexpr uint32_t WINDOW_HEIGHT = 600;

int main(void) {
	engine::init();

	Window window1({
		.width = WINDOW_WIDTH,
		.height = WINDOW_HEIGHT,
		.title = "Etna Window 1",
		.captureMouse = true,
	});

	Window window2({
		.width = WINDOW_WIDTH,
		.height = WINDOW_HEIGHT,
		.title = "Etna Window 2",
	});

	Scene scene;

	SceneNode root = scene.createRoot("root", {});

	root.addMesh("Brick", engine::getUVCube(),
				 {
					 .position = {0.f, 0.f, -2.f},
					 .scale = {1.f, 1.f, 2.f},
				 });

	Viewport const viewport1{
		.x = 0,
		.y = 0,
		.width = (float)WINDOW_WIDTH / 2,
		.height = (float)WINDOW_HEIGHT,
	};

	Viewport const viewport2{
		.x = (float)WINDOW_WIDTH / 2,
		.y = 0,
		.width = (float)WINDOW_WIDTH / 2,
		.height = (float)WINDOW_HEIGHT,
	};

	CameraNode& camera1 =
		root.addCamera("Camera1", {.position = {0, 0, 1}}, viewport1);

	CameraNode& camera2 =
		root.addCamera("Camera2", {.position = {0, 0, 1}}, viewport2);

	CameraNode& camera3 = root.addCamera("Camera3", {.position = {0, 0, 1}});

	Renderer renderer({});

	bool shouldClose = false;

	while (!shouldClose) {
		window1.pollEvents();
		window2.pollEvents();

		updateFirstPersonCamera(camera1, window1);
		updateFirstPersonCamera(camera2, window1);
		updateFirstPersonCamera(camera3, window1);

		if (window1.shouldClose() || window2.shouldClose()) {
			shouldClose = true;
		}

		renderer.beginFrame();

		renderer.renderScene(scene, window1, camera1);
		renderer.renderScene(scene, window1, camera2, LOAD_PREVIOUS);

		renderer.renderScene(scene, window2, camera3);

		renderer.endFrame();

		window1.swapBuffers();

		window2.swapBuffers();
	}

	return 0;
}
