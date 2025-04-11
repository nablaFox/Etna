#include "etna/renderer.hpp"
#include "etna/engine.hpp"
#include "etna/default_materials.hpp"
#include "ignis/fence.hpp"

using namespace etna;
using namespace ignis;

static MaterialHandle g_defaultMaterial{nullptr};

Renderer::Renderer(const CreateInfo& info) : m_framesInFlight(info.framesInFlight) {
	assert(m_framesInFlight > 0);

	m_frames.resize(m_framesInFlight);

	for (uint32_t i{0}; i < m_framesInFlight; i++) {
		m_frames[i].inFlight = new Fence(_device.createFence());
		m_frames[i].cmd = engine::newGraphicsCommand();
	}

	g_defaultMaterial = engine::createColorMaterial(WHITE);
}

Renderer::~Renderer() {
	for (uint32_t i{0}; i < m_framesInFlight; i++) {
		delete m_frames[i].inFlight;
		delete m_frames[i].cmd;
	}

	g_defaultMaterial.reset();
}

void Renderer::beginFrame(const RenderTarget& target,
						  const RenderFrameSettings& settings) {
	Command& cmd = getCommand();
	FrameData& frame = m_frames[m_currentFrame];

	m_currTarget = &target;

	frame.inFlight->reset();

	cmd.begin();

	const VkClearColorValue clearColorValue{
		settings.clearColor.r,
		settings.clearColor.g,
		settings.clearColor.b,
		settings.clearColor.a,
	};

	DrawAttachment* drawAttachment = new DrawAttachment({
		.drawImage = m_currTarget->getDrawImage(),
		.loadAction = settings.colorLoadOp,
		.storeAction = settings.colorStoreOp,
		.clearColor = clearColorValue,
	});

	DepthAttachment* depthAttachment =
		settings.renderDepth ? new DepthAttachment({
								   .depthImage = m_currTarget->getDepthImage(),
								   .loadAction = settings.depthLoadOp,
								   .storeAction = settings.depthStoreOp,
							   })
							 : nullptr;

	cmd.beginRender(drawAttachment, depthAttachment);
}

void Renderer::endFrame() {
	Command& cmd = getCommand();

	cmd.endRendering();

	if (!m_currTarget->isMultiSampled())
		return;

	ignis::Image& drawImage = *m_currTarget->getDrawImage();
	ignis::Image& resolvedDrawImage = *m_currTarget->getResolvedImage();

	cmd.transitionImageLayout(drawImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
	cmd.transitionImageLayout(resolvedDrawImage,
							  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	cmd.resolveImage(drawImage, resolvedDrawImage);

	cmd.transitionToOptimalLayout(drawImage);
	cmd.transitionToOptimalLayout(resolvedDrawImage);

	cmd.end();

	SubmitCmdInfo cmdInfo{.command = cmd};

	_device.submitCommands({cmdInfo}, m_frames[m_currentFrame].inFlight);

	m_frames[m_currentFrame].inFlight->wait();

	m_currentFrame = (m_currentFrame + 1) % m_framesInFlight;
}

void Renderer::draw(const DrawSettings& settings) {
	Command& cmd = getCommand();

	assert(settings.mesh != nullptr && "Mesh is null");
	assert(settings.instanceCount > 0 && "Instance count is zero");

	VkViewport vp{
		.x = settings.viewport.x,
		.y = settings.viewport.y,
		.width = settings.viewport.width,
		.height = settings.viewport.height,
		.minDepth = 0.f,
		.maxDepth = 1.f,
	};

	const MaterialHandle materialToUse =
		settings.material != nullptr ? settings.material : g_defaultMaterial;

	Pipeline& pipeline = materialToUse->getTemplate().getPipeline();

	cmd.bindPipeline(pipeline);

	cmd.setViewport(vp);

	cmd.setScissor(
		{0, 0, m_currTarget->getExtent().width, m_currTarget->getExtent().height});

	cmd.bindIndexBuffer(*settings.mesh->getIndexBuffer());

	const engine::PushConstants m_pushConstants{
		.model = settings.transform,
		.vertices = settings.mesh->getVertexBuffer(),
		.material = materialToUse->getParamsUBO(),
		.instanceBuffer = settings.instanceBuffer,
		.buff1 = settings.buff1,
		.buff2 = settings.buff2,
		.buff3 = settings.buff3,
	};

	cmd.pushConstants(pipeline, m_pushConstants);

	cmd.drawInstanced(settings.mesh->indexCount(), settings.instanceCount);
}

void Renderer::clearViewport(Viewport vp, Color color) {
	const VkClearColorValue clearColorValue{
		color.r,
		color.g,
		color.b,
		color.a,
	};

	getCommand().clearViewport(vp.x, vp.y, vp.width, vp.height, clearColorValue);
}
