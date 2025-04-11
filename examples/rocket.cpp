#include "shared.hpp"

constexpr float WINDOW_WIDTH{1100};
constexpr float WINDOW_HEIGHT{700};
constexpr float GRAVITY{9.81};
constexpr float WU_PER_METER{0.04};	 // every meter is 0.04 world units (pixels)

// Units in SI (meters, kilograms)
constexpr float ROCKET_MASS{11000};
constexpr float ROCKET_INITIAL_VELOCITY{20};
constexpr float ROCKET_HEIGHT{40};
constexpr float ROCKET_WIDTH{18};
constexpr float ROCKET_INIT_HEIGHT{300};
const Vec3 ROCKET_INITIAL_POS{0, ROCKET_INIT_HEIGHT, 0};
constexpr float ROCKET_MINIMAP_SIZE{0.25};

void simulateRocket(PhysicalObject& rocket, SceneNode rocketMesh) {
	std::cout << "\033[2J\033[1;1H";

	const float currHeight = rocket.pos[1] - (ROCKET_HEIGHT / 2);

	if (currHeight <= 1e-3) {
		std::cout << "\033[32mRocket has landed!\033[0m\n" << std::endl;
		return;
	}

	const float vel = rocket.vel[1];
	const float kineticEnergy = ROCKET_MASS * square(vel) / 2;

	const float weight = ROCKET_MASS * GRAVITY;
	const float decForce = weight + kineticEnergy / currHeight;

	const float contactTime = ROCKET_MASS * vel / (weight - decForce);

	std::cout << "Height: " << currHeight << std::endl;
	std::cout << "Velocity: " << vel << "m/s\n"
			  << std::fixed << std::setprecision(2);
	std::cout << "Contact in: " << contactTime << "s\n" << std::endl;

	rocket.force = {0, decForce - weight, 0};

	rocket.update();
	rocketMesh->updatePosition(rocket.getRenderPosition(WU_PER_METER));
}

SceneNode createRocketModel() {
	const float width = ROCKET_WIDTH * WU_PER_METER;
	const float height = ROCKET_HEIGHT * WU_PER_METER;

	SceneNode rocket = scene::createRoot("Rocket");

	rocket->add(createOutlinedBrick({
		.color = WHITE,
		.outlineColor = BLACK,
		.name = "Body",
		.outlineThickness = 0.01,
		.width = width,
		.height = height,
		.depth = width,
	}));

	rocket->add(scene::createMeshNode({
		.name = "Nose",
		.mesh = engine::getPyramid(),
		.transform = {.position = {0, height / 2, 0}},
		.material = engine::createColorMaterial(RED),
	}));

	return rocket;
}

int main(void) {
	engine::init();

	Window window({
		.width = (int)WINDOW_WIDTH,
		.height = (int)WINDOW_HEIGHT,
		.title = "Etna Rocket",
		.captureMouse = true,
	});

	Scene scene;

	scene.addNode(createFloor({}));

	CameraNode playerCamera = scene.createCameraNode({
		.name = "PlayerCamera",
		.transform = {.position = {2, 1, -14}, .yaw = -3.25f, .pitch = -0.27f},
	});

	PhysicalObject rocket{
		.mass = ROCKET_MASS,
		.vel = {0, -ROCKET_INITIAL_VELOCITY, 0},
		.pos = ROCKET_INITIAL_POS,
	};

	SceneNode rocketMesh = scene.addNode(createRocketModel());

	const Viewport rocketViewport{
		.x = WINDOW_WIDTH * (1 - ROCKET_MINIMAP_SIZE),
		.y = WINDOW_HEIGHT * 0,
		.width = WINDOW_WIDTH * ROCKET_MINIMAP_SIZE,
		.height = WINDOW_WIDTH * ROCKET_MINIMAP_SIZE,
	};

	CameraNode rocketCamera = rocketMesh->createCameraNode({
		.name = "RocketCamera",
		.transform = {.position = {0, 0, 0.5}, .pitch = M_PI / 2},
	});

	Renderer renderer({});

	bool simulate{false};

	while (!window.shouldClose()) {
		window.pollEvents();

		engine::updateTime();

		if (simulate) {
			simulateRocket(rocket, rocketMesh);
		}

		firstPersonMovement(playerCamera, window);

		if (window.isKeyClicked(GLFW_KEY_ENTER)) {
			simulate = !simulate;
		}

		renderer.beginFrame(window);

		scene.render(renderer, playerCamera);

		renderer.endFrame();

		renderer.beginFrame(window, LOAD_PREVIOUS);

		renderer.clearViewport(rocketViewport, BLACK);

		scene.render(renderer, rocketCamera, {.viewport = rocketViewport});

		renderer.endFrame();

		window.swapBuffers();
	}

	return 0;
}
