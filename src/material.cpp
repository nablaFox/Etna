#include "etna/material.hpp"
#include "etna/engine.hpp"

using namespace ignis;
using namespace etna;

MaterialTemplate::MaterialTemplate(const CreateInfo& info)
	: m_paramsSize(info.paramsSize) {
	const uint32_t sampleCount{engine::clampSampleCount(info.samples)};

	for (const auto& shaderPath : info.shaders) {
		m_shaders.push_back(engine::newShader(shaderPath));
	}

	for (const auto& shader : info.rawShaders) {
		m_shaders.push_back(
			engine::newShader(shader.code, shader.size, shader.stage));
	}

	PipelineCreateInfo pipelineInfo{
		.device = &_device,
		.shaders = m_shaders,
		.colorFormat = engine::COLOR_FORMAT,
		.cullMode = VK_CULL_MODE_NONE,
		.polygonMode = info.polygonMode,
		.lineWidth = info.lineWidth,
		.sampleCount = static_cast<VkSampleCountFlagBits>(sampleCount),
		.sampleShadingEnable = _device.isFeatureEnabled("SampleRateShading"),
	};

	if (info.polygonMode != VK_POLYGON_MODE_FILL &&
		!_device.isFeatureEnabled("FillModeNonSolid")) {
		pipelineInfo.polygonMode = VK_POLYGON_MODE_FILL;
	}

	if (info.enableDepth) {
		pipelineInfo.depthFormat = engine::DEPTH_FORMAT;
		pipelineInfo.enableDepthTest = true;
		pipelineInfo.enableDepthWrite = true;
	}

	if (info.transparency) {
		pipelineInfo.enableDepthWrite = false;
		pipelineInfo.blendEnable = true;
		pipelineInfo.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		pipelineInfo.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		pipelineInfo.colorBlendOp = VK_BLEND_OP_ADD;
	}

	m_pipeline = new Pipeline(pipelineInfo);

#ifndef NDEBUG
	samples = sampleCount;
#endif
}

MaterialTemplate::~MaterialTemplate() {
	for (ignis::Shader* shader : m_shaders) {
		delete shader;
	}

	delete m_pipeline;
}

MaterialTemplateHandle MaterialTemplate::create(const CreateInfo& info) {
	return std::shared_ptr<MaterialTemplate>(new MaterialTemplate(info));
}

Material::Material(const CreateInfo& info)
	: m_materialTemplate(info.templateHandle) {
	size_t paramsSize = m_materialTemplate->getParamsSize();

	if (!paramsSize) {
		return;
	}

	m_paramsUBO = _device.createUBO(paramsSize, info.params);
}

Material::Material(const MaterialTemplate::CreateInfo& info) {
	m_materialTemplate = MaterialTemplate::create({
		.rawShaders = info.rawShaders,
		.paramsSize = info.paramsSize,
		.enableDepth = info.enableDepth,
		.transparency = info.transparency,
		.polygonMode = info.polygonMode,
		.lineWidth = info.lineWidth,
	});

	if (!info.paramsSize) {
		return;
	}

	m_paramsUBO = _device.createUBO(info.paramsSize);
}

MaterialHandle Material::create(const CreateInfo& info) {
	return std::shared_ptr<Material>(new Material(info));
}

MaterialHandle Material::create(const MaterialTemplate::CreateInfo& info) {
	return std::shared_ptr<Material>(new Material(info));
}

Material::~Material() {
	if (m_paramsUBO != IGNIS_INVALID_BUFFER_ID)
		_device.destroyBuffer(m_paramsUBO);
}

void Material::updateParams(const void* data) const {
	if (m_paramsUBO != IGNIS_INVALID_BUFFER_ID) {
		_device.updateBuffer(m_paramsUBO, data);
	}
}
