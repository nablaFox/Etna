#pragma once

#include "engine.hpp"
#include "camera.hpp"
#include "window.hpp"

using namespace etna;

struct FirstPersonCamera : Camera {
	struct CreateInfo {
		float fov{70};
		float aspect{1.77};
		float near{0.1f};
		float far{100.f};
		float cameraSpeed{0.1f};
		float sensitivity{0.001f};
		float flyAround{false};
		Transform transform;
	};

	FirstPersonCamera(CreateInfo info)
		: Camera(info.fov, info.aspect, info.near, info.far, info.transform),
		  flyAround(info.flyAround),
		  sensitivity(info.sensitivity),
		  cameraSpeed(info.cameraSpeed) {}

	float flyAround = false;
	float sensitivity = 0.001f;

	// it isn't a speed;
	// it should probably be "cameraMovementIncrement"
	float cameraSpeed = 0.1f;

	void update(etna::Window& window, float deltaTime) {
		// CHECK multiplying by deltaTime gives weird artifacts
		transform.yaw += window.mouseDeltaX() * sensitivity;
		transform.pitch += window.mouseDeltaY() * sensitivity;

		if (transform.pitch > M_PI / 2) {
			transform.pitch = M_PI / 2;
		} else if (transform.pitch < -M_PI / 2) {
			transform.pitch = -M_PI / 2;
		}

		Vec3& position = transform.position;
		float yaw = -transform.yaw;

		const Vec3 playerForward =
			flyAround ? forward() : Vec3{sinf(yaw), 0, cosf(yaw)} * -1;

		const Vec3 playerRight =
			flyAround ? right() : Vec3{cosf(yaw), 0, -sinf(yaw)};

		const Vec3 up = flyAround ? this->up() : Vec3{0, 0, 0};

		if (window.isKeyPressed(GLFW_KEY_0)) {
			position = {0, 1, 0};
		}

		if (window.isKeyPressed(GLFW_KEY_F)) {
			flyAround = true;
		}

		if (window.isKeyPressed(GLFW_KEY_G)) {
			flyAround = false;
		}

		if (window.isKeyPressed(GLFW_KEY_W)) {
			position += playerForward * cameraSpeed * deltaTime;
		}

		if (window.isKeyPressed(GLFW_KEY_S)) {
			position -= playerForward * cameraSpeed * deltaTime;
		}

		if (window.isKeyPressed(GLFW_KEY_D)) {
			position += playerRight * cameraSpeed * deltaTime;
		}

		if (window.isKeyPressed(GLFW_KEY_A)) {
			position -= playerRight * cameraSpeed * deltaTime;
		}

		if (window.isKeyPressed(GLFW_KEY_SPACE)) {
			position += up * cameraSpeed * deltaTime;
		}

		if (window.isKeyPressed(GLFW_KEY_LEFT_SHIFT)) {
			position -= up * cameraSpeed * deltaTime;
		}
	}
};
