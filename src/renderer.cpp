#include "forward_renderer.hpp"
#include "fence.hpp"

using namespace ignis;
using namespace etna;

Renderer::Renderer(const CreateInfo& info) : m_framesInFlight(info.framesInFlight) {
	assert(m_framesInFlight > 0);

	Device& device = Engine::getDevice();

	m_framesData.resize(m_framesInFlight);

	for (uint32_t i{0}; i < m_framesInFlight; i++) {
		m_framesData[i].inFlight = new ignis::Fence(device);

		m_framesData[i].cmd = new ignis::Command({
			.device = device,
			.queue = Engine::getGraphicsQueue(),
		});

		m_framesData[i].sceneDataBuff = device.createSSBO(sizeof(SceneData));
	}
}

Renderer::~Renderer() {
	Device& device = Engine::getDevice();

	for (uint32_t i{0}; i < m_framesInFlight; i++) {
		delete m_framesData[i].inFlight;
		delete m_framesData[i].cmd;
		device.destroyBuffer(m_framesData[i].sceneDataBuff);
	}
}

void Renderer::beginFrame() {
	getCommand().begin();

	m_framesData[m_currentFrame].inFlight->reset();
}

void Renderer::endFrame() {
	getCommand().end();

	SubmitCmdInfo cmdInfo{.command = getCommand()};

	Engine::getDevice().submitCommands({cmdInfo},
									   m_framesData[m_currentFrame].inFlight);

	m_framesData[m_currentFrame].inFlight->wait();

	m_currentFrame = (m_currentFrame + 1) % m_framesInFlight;
}

void Renderer::renderScene(const Scene& scene,
						   const RenderTarget& renderTarget,
						   const Camera& camera,
						   const RenderSettings sceneInfo) {
	Command& cmd = getCommand();
	VkViewport viewport = sceneInfo.viewport;
	Color clearColor = sceneInfo.clearColor;

	BufferId sceneDataBuff = m_framesData[m_currentFrame].sceneDataBuff;

	SceneData sceneData = {
		.viewproj = camera.getViewProjMatrix(),
	};

	cmd.updateBuffer(sceneDataBuff, &sceneData);

	VkClearColorValue clearColorValue = {
		clearColor.r,
		clearColor.g,
		clearColor.b,
		clearColor.a,
	};

	DrawAttachment* drawAttachment = new DrawAttachment({
		.drawImage = renderTarget.getDrawImage(),
		.loadAction = sceneInfo.colorLoadOp,
		.storeAction = sceneInfo.colorStoreOp,
		.clearColor = clearColorValue,
	});

	DepthAttachment* depthAttachment =
		renderTarget.getCreationInfo().hasDepth
			? new DepthAttachment({
				  .depthImage = renderTarget.getDepthImage(),
				  .loadAction = sceneInfo.depthLoadOp,
				  .storeAction = sceneInfo.depthStoreOp,
			  })
			: nullptr;

	cmd.beginRender(drawAttachment, depthAttachment);

#ifndef NDEBUG
	for (const auto& node : scene.getNodes()) {
		MaterialHandle material = node.material;

		const MaterialHandle materialToUse =
			material != nullptr ? material : Engine::getDefaultMaterial();

		if (materialToUse->getTemplate().hasDepth !=
			renderTarget.getCreationInfo().hasDepth) {
			throw std::runtime_error("Material has different depth test value");
		}
	}
#endif

	for (const auto& node : scene.getNodes()) {
		MeshHandle mesh = node.mesh;
		MaterialHandle material = node.material;

		const MaterialHandle materialToUse =
			material != nullptr ? material : Engine::getDefaultMaterial();

		Pipeline& pipeline = materialToUse->getTemplate().getPipeline();

		cmd.bindPipeline(pipeline);

		if (viewport.width == 0) {
			viewport.x = 0;
			viewport.width = (float)renderTarget.getExtent().width;
		}

		if (viewport.height == 0) {
			viewport.y = 0;
			viewport.height = (float)renderTarget.getExtent().height;
		}

		viewport.maxDepth = 1.f;
		viewport.minDepth = 0.f;

		cmd.setViewport(viewport);

		cmd.setScissor(
			{0, 0, renderTarget.getExtent().width, renderTarget.getExtent().height});

		cmd.bindIndexBuffer(*mesh->getIndexBuffer());

		m_pushConstants = {
			.worldTransform = node.transform.getWorldMatrix(),
			.vertices = mesh->getVertexBuffer(),
			.material = materialToUse->getParamsUBO(),
			.sceneData = sceneDataBuff,
		};

		cmd.pushConstants(pipeline, m_pushConstants);

		cmd.draw(mesh->indexCount());
	}

	cmd.endRendering();

	if (!renderTarget.isMultiSampled())
		return;

	Image& drawImage = *renderTarget.getDrawImage();
	Image& resolvedDrawImage = *renderTarget.getResolvedImage();

	cmd.transitionImageLayout(drawImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
	cmd.transitionImageLayout(resolvedDrawImage,
							  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	cmd.resolveImage(drawImage, resolvedDrawImage);

	cmd.transitionToOptimalLayout(drawImage);
	cmd.transitionToOptimalLayout(resolvedDrawImage);
}
