#pragma once

#include <memory>
#include "ignis/types.hpp"
#include "ignis/pipeline.hpp"

namespace etna {

struct RawShader {
	const unsigned char* code;
	size_t size;
	VkShaderStageFlagBits stage;
};

class MaterialTemplate {
public:
	struct CreateInfo {
		std::vector<std::string> shaders;
		std::vector<RawShader> rawShaders;
		size_t paramsSize{0};  // TODO: reflect this
		bool enableDepth{true};
		bool transparency{false};
		VkPolygonMode polygonMode{VK_POLYGON_MODE_FILL};
		float lineWidth{1.0f};
		uint32_t samples{0};
	};

	~MaterialTemplate();

	static std::shared_ptr<MaterialTemplate> create(const CreateInfo&);

	auto& getPipeline() const { return *m_pipeline; }

	auto getParamsSize() const { return m_paramsSize; }

#ifndef NDEBUG
	uint32_t samples;
#endif

private:
	MaterialTemplate(const CreateInfo&);

	std::vector<ignis::Shader*> m_shaders;
	ignis::Pipeline* m_pipeline{nullptr};

	size_t m_paramsSize{0};
};

using MaterialTemplateHandle = std::shared_ptr<MaterialTemplate>;

class Material {
public:
	struct CreateInfo {
		std::shared_ptr<MaterialTemplate> templateHandle;
		const void* params{nullptr};
	};

	Material(const CreateInfo&);

	Material(const MaterialTemplate::CreateInfo&);

	static std::shared_ptr<Material> create(const CreateInfo&);

	static std::shared_ptr<Material> create(const MaterialTemplate::CreateInfo&);

	~Material();

	void updateParams(const void* data) const;

	auto& getTemplate() const { return *m_materialTemplate; }

	auto getParamsUBO() const { return m_paramsUBO; }

private:
	ignis::BufferId m_paramsUBO{IGNIS_INVALID_BUFFER_ID};
	std::shared_ptr<MaterialTemplate> m_materialTemplate;

public:
	Material(const Material&) = delete;
	Material(Material&&) = delete;
	Material& operator=(const Material&) = delete;
	Material& operator=(Material&&) = delete;
};

using MaterialHandle = std::shared_ptr<Material>;

}  // namespace etna
