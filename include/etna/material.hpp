#pragma once

#include <memory>
#include "ignis/types.hpp"
#include "ignis/pipeline.hpp"

namespace etna {

class MaterialTemplate {
public:
	struct CreateInfo {
		std::vector<std::string> shaders;
		size_t paramsSize{0};  // TODO: reflect this
		bool enableDepth{true};
		bool transparency{false};
		VkPolygonMode polygonMode{VK_POLYGON_MODE_FILL};
		float lineWidth{1.0f};
	};

	~MaterialTemplate();

	static std::shared_ptr<MaterialTemplate> create(const CreateInfo&);

	auto& getPipeline() const { return *m_pipeline; }

	auto getParamsSize() const { return m_paramsSize; }

#ifndef NDEBUG
	bool hasDepth;
#endif

private:
	MaterialTemplate(const CreateInfo&);

	ignis::Pipeline* m_pipeline{nullptr};

	size_t m_paramsSize{0};
};

using MaterialTemplateHandle = std::shared_ptr<MaterialTemplate>;

class Material {
public:
	struct CreateInfo {
		std::shared_ptr<MaterialTemplate> templateHandle;
		void* params{nullptr};
	};

	struct CreateInfo2 {
		std::vector<std::string> shaders;
		void* paramsData{nullptr};
		size_t paramsSize{0};  // TODO: reflect this
		bool enableDepth{true};
		bool transparency{false};
		VkPolygonMode polygonMode{VK_POLYGON_MODE_FILL};
		float lineWidth{1.0f};
	};

	Material(const CreateInfo&);

	Material(const CreateInfo2&);

	static std::shared_ptr<Material> create(const CreateInfo&);

	static std::shared_ptr<Material> create(const CreateInfo2&);

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
