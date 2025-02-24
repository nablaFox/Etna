#pragma once

#include <cassert>
#include <chrono>
#include "types.hpp"
#include "mesh.hpp"
#include "window.hpp"
#include "camera.hpp"
#include "ignis/ignis.hpp"

using namespace ignis;

struct SceneData {
	Mat4 viewproj;
	Color ambientColor;
	DirectionalLight sun;
};

struct PushConstants {
	Mat4 worldTransform;
	VkDeviceAddress verticesAddress;
	uint32_t materialHandle;
};

class Renderer {
public:
	Renderer(const Window&);
	~Renderer();

public:
	void beginScene(Camera camera,
					DirectionalLight sun,
					Color ambientColor = Color{});

	void endScene();

	void draw(Mesh&, WorldTransform);

	float getFps() const { return m_fps; }
	void updateFps();

private:
	ColorImage* m_drawImage;
	DepthImage* m_depthImage;
	Buffer* m_sceneDataUBO;
	Swapchain* m_swapchain;

	static constexpr uint32_t FRAMES_IN_FLIGHT = 2;

	struct FrameData {
		Semaphore* finishedRendering;
		Fence* waitForRenderingCompletion;
		Command* cmd;
	} frames[FRAMES_IN_FLIGHT];

	uint32_t m_currentFrame{0};

	FrameData& currFrame() { return frames[m_currentFrame % FRAMES_IN_FLIGHT]; }

	DrawAttachment m_drawAttachment;
	DepthAttachment m_depthAttachment;

	PushConstants m_pushConstants;

	std::chrono::time_point<std::chrono::high_resolution_clock> m_lastTime =
		std::chrono::high_resolution_clock::now();

	uint32_t m_fps{0};
};
