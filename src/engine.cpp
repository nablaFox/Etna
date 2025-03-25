#include <GLFW/glfw3.h>
#include "ignis/device.hpp"
#include "engine.hpp"
#include "scene.hpp"
#include "transform.hpp"
#include "render_target.hpp"

using namespace etna;
using namespace ignis;

Device* g_device = nullptr;

Device& Engine::getDevice() {
	if (g_device != nullptr)
		return *g_device;

	throw std::runtime_error("Engine must be initialized to access the GPU device");
}

Engine::Engine() {
	glfwInit();

	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions =
		glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<const char*> extensions(glfwExtensions,
										glfwExtensions + glfwExtensionCount);

	g_device = new ignis::Device({
		.shadersFolder = "shaders",
		.extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME},
		.instanceExtensions = extensions,
		.optionalFeatures = {"FillModeNonSolid", "SampleRateShading"},
	});

	m_defaultMaterial =
		new _Material({.shaders = {"default.vert.spv", "default.frag.spv"}});

	for (uint32_t i{0}; i < FRAMES_IN_FLIGHT; i++) {
		m_framesData[i].m_inFlight = new ignis::Fence(*g_device);

		m_framesData[i].m_cmd = new ignis::Command({
			.device = *g_device,
			.queue = g_device->getQueue(m_graphicsQueue),
		});
	}
}

Engine::~Engine() {
	// TODO: use m_device->waitIdle() from ignis
	vkDeviceWaitIdle(g_device->getDevice());

	glfwTerminate();

	delete m_defaultMaterial;
	delete g_device;
}

void Engine::beginFrame() {
	getCommand().begin();

	m_framesData[m_currentFrame].m_inFlight->reset();
}

void Engine::endFrame() {
	SubmitCmdInfo renderCmdInfo{.command = getCommand()};

	g_device->submitCommands({renderCmdInfo},
							 m_framesData[m_currentFrame].m_inFlight);

	m_currentFrame = (m_currentFrame + 1) % FRAMES_IN_FLIGHT;

	m_framesData[m_currentFrame].m_inFlight->wait();
}

void Engine::renderScene(const Scene& scene,
						 const RenderTarget& renderTarget,
						 const Camera& camera,
						 VkViewport viewport) {
	Command& cmd = getCommand();

	cmd.beginRender(&renderTarget.getDrawAttachment(),
					renderTarget.getDepthAttachment());

#ifndef NDEBUG
	for (const auto& node : scene.getNodes()) {
		Material material = node.material;

		const _Material* materialToUse =
			material != nullptr ? material.get() : m_defaultMaterial;

		if (materialToUse->hasDepthTest() !=
			renderTarget.getCreationInfo().hasDepth) {
			throw std::runtime_error("Material has different depth test value");
		}
	}
#endif

	for (const auto& node : scene.getNodes()) {
		Mesh mesh = node.mesh;
		Transform transform = *node.transform;
		Material material = node.material;

		const _Material* materialToUse =
			material != nullptr ? material.get() : m_defaultMaterial;

		Pipeline& pipeline = materialToUse->getPipeline();

		cmd.bindPipeline(pipeline);

		if (viewport.width == 0) {
			viewport.width = (float)renderTarget.getExtent().width;
		}

		if (viewport.height == 0) {
			viewport.height = (float)renderTarget.getExtent().height;
		}

		cmd.setViewport(viewport);

		cmd.setScissor(
			{0, 0, renderTarget.getExtent().width, renderTarget.getExtent().height});

		cmd.bindIndexBuffer(mesh->indexBuffer);

		m_pushConstants = {
			.worldTransform = transform.getWorldMatrix(),
			.vertices = mesh->vertexBuffer,
			.material = materialToUse->getParamsUBO(),
			.sceneData = scene.getSceneDataUBO(),
		};

		cmd.pushConstants(pipeline, m_pushConstants);

		cmd.draw(mesh->indexCount());
	}

	cmd.endRendering();

	if (!renderTarget.isMultiSampled())
		return;

	// TODO: resolve image
}
