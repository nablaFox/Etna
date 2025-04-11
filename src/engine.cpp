#include <GLFW/glfw3.h>
#include <deque>
#include "ignis/device.hpp"
#include "ignis/command.hpp"
#include "engine.hpp"

using namespace etna;
using namespace ignis;

Device* g_device{nullptr};
VkQueue g_graphicsQueue{nullptr};
VkQueue g_immediateQueue{nullptr};
VkQueue g_presentQueue{nullptr};
float g_deltaTime{0};

std::deque<std::function<void()>> g_deletionQueue;

std::string g_shadersFolder;

#define CHECK_INIT assert(g_device != nullptr && "Engine not initialized");

static void cleanup() {
	g_device->waitIdle();

	glfwTerminate();

	for (auto& func : g_deletionQueue) {
		func();
	}

	delete g_device;
}

void engine::init(const InitInfo& info) {
	glfwInit();

	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions =
		glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<const char*> extensions(glfwExtensions,
										glfwExtensions + glfwExtensionCount);

	g_device = new ignis::Device({
		.appName = info.appName,
		.extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME},
		.instanceExtensions = extensions,
		.optionalFeatures = {"FillModeNonSolid", "SampleRateShading"},
	});

	// TODO: choose graphics & upload queues
	g_graphicsQueue = g_device->getQueue(0);
	g_immediateQueue = g_device->getQueue(0);
	g_presentQueue = g_device->getQueue(0);

	g_shadersFolder = info.shadersFolder;

	std::atexit(cleanup);
}

Device& engine::getDevice() {
	CHECK_INIT;

	return *g_device;
}

void engine::immediateSubmit(std::function<void(ignis::Command&)>&& func) {
	CHECK_INIT;

	Command cmd{{
		.device = *g_device,
		.queue = g_graphicsQueue,
	}};

	cmd.begin();

	func(cmd);

	cmd.end();

	g_device->submitCommands({{.command = cmd}}, nullptr);
	g_device->waitIdle();
}

void engine::immediateUpdate(ignis::BufferId buffer,
							 const void* data,
							 VkDeviceSize offset,
							 VkDeviceSize size) {
	immediateSubmit(
		[&](ignis::Command& cmd) { cmd.updateBuffer(buffer, data, offset, size); });
}

void engine::presentCurrent(const ignis::Swapchain& swapchain,
							std::vector<const ignis::Semaphore*> waitSemaphores) {
	swapchain.presentCurrent({
		.presentationQueue = g_presentQueue,
		.waitSemaphores = waitSemaphores,
	});
}

ignis::Command engine::createGraphicsCommand() {
	CHECK_INIT;

	return Command({.device = *g_device, .queue = g_graphicsQueue});
}

ignis::Command* engine::newGraphicsCommand() {
	CHECK_INIT;

	return new Command({.device = *g_device, .queue = g_graphicsQueue});
}

void engine::queueForDeletion(std::function<void()> func) {
	CHECK_INIT;

	g_deletionQueue.push_back(func);
}

uint32_t engine::clampSampleCount(uint32_t sampleCount) {
	CHECK_INIT;

	if (sampleCount == 0) {
		return MAX_SAMPLE_COUNT;
	}

	const uint32_t deviceMaxSampleCount = g_device->getMaxSampleCount();

	return deviceMaxSampleCount > MAX_SAMPLE_COUNT ? MAX_SAMPLE_COUNT
												   : deviceMaxSampleCount;
}

ignis::Shader* engine::newShader(const std::string& path) {
	CHECK_INIT;

	const std::string::size_type extPos = path.find_last_of('.');
	const std::string ext = path.substr(extPos + 1);

	const VkShaderStageFlagBits stage =
		ext == "vert" ? VK_SHADER_STAGE_VERTEX_BIT : VK_SHADER_STAGE_FRAGMENT_BIT;

	const std::string shaderPath = g_shadersFolder + "/" + path + ".spv";

	ignis::Shader* shader =
		new Shader(g_device->createShader(shaderPath, stage, sizeof(PushConstants)));

	return shader;
}

float engine::getDeltaTime() {
	CHECK_INIT;

	return g_deltaTime;
}

float engine::updateTime() {
	CHECK_INIT;

	static float lastTime = 0;
	static float currentTime = 0;

	lastTime = currentTime;
	currentTime = (float)glfwGetTime();

	g_deltaTime = currentTime - lastTime;

	return g_deltaTime;
}
