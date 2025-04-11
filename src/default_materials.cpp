#include "etna/default_materials.hpp"
#include "etna/engine.hpp"
#include "incbin.h"

using namespace etna;

namespace {

MaterialTemplateHandle g_colorMaterialTemplate{nullptr};
MaterialTemplateHandle g_pointMaterialTemplate{nullptr};
MaterialTemplateHandle g_gridTemplate{nullptr};
MaterialTemplateHandle g_transparentGridTemplate{nullptr};

}

// TEMP: look at incbin.h
EMBED_BINARY(g_default_vert_spv, "src/shaders/default.vert.spv");
EMBED_BINARY(g_default_frag_spv, "src/shaders/default.frag.spv");
EMBED_BINARY(g_grid_frag_spv, "src/shaders/grid.frag.spv");

const RawShader g_default_vert{
	g_default_vert_spv,
	g_default_vert_spv_size,
	VK_SHADER_STAGE_VERTEX_BIT,
};

const RawShader g_default_frag{
	g_default_frag_spv,
	g_default_frag_spv_size,
	VK_SHADER_STAGE_FRAGMENT_BIT,
};

const RawShader g_grid_frag{
	g_grid_frag_spv,
	g_grid_frag_spv_size,
	VK_SHADER_STAGE_FRAGMENT_BIT,
};

MaterialHandle engine::createColorMaterial(Color color) {
	initColorMaterial();

	return Material::create({
		.templateHandle = g_colorMaterialTemplate,
		.params = &color,
	});
}

MaterialHandle engine::createPointMaterial(Color color) {
	initPointMaterial();

	return Material::create({
		.templateHandle = g_pointMaterialTemplate,
		.params = &color,
	});
}

MaterialHandle engine::createGridMaterial(GridMaterialParams params) {
	initGridMaterial();

	return Material::create({
		.templateHandle = g_gridTemplate,
		.params = &params,
	});
}

MaterialHandle engine::createTransparentGridMaterial(GridMaterialParams params) {
	initTransparentGridMaterial();

	return Material::create({
		.templateHandle = g_transparentGridTemplate,
		.params = &params,
	});
}

void engine::initDefaultMaterials() {
	initColorMaterial();
	initPointMaterial();
	initGridMaterial();
	initTransparentGridMaterial();
}

void engine::initColorMaterial() {
	if (g_colorMaterialTemplate != nullptr) {
		return;
	}

	g_colorMaterialTemplate = MaterialTemplate::create({
		.rawShaders = {g_default_vert, g_default_frag},
		.paramsSize = sizeof(Color),
	});

	queueForDeletion([=] { g_colorMaterialTemplate.reset(); });
}

void engine::initPointMaterial() {
	if (g_pointMaterialTemplate != nullptr) {
		return;
	}

	g_pointMaterialTemplate = MaterialTemplate::create({
		.rawShaders = {g_default_vert, g_default_frag},
		.paramsSize = sizeof(Color),
		.polygonMode = VK_POLYGON_MODE_POINT,
	});

	queueForDeletion([=] { g_pointMaterialTemplate.reset(); });
}

void engine::initGridMaterial() {
	if (g_gridTemplate != nullptr) {
		return;
	}

	g_gridTemplate = MaterialTemplate::create({
		.rawShaders = {g_default_vert, g_grid_frag},
		.paramsSize = sizeof(GridMaterialParams),
	});

	queueForDeletion([=] { g_gridTemplate.reset(); });
}

void engine::initTransparentGridMaterial() {
	if (g_transparentGridTemplate != nullptr) {
		return;
	}

	g_transparentGridTemplate = MaterialTemplate::create({
		.rawShaders = {g_default_vert, g_grid_frag},
		.paramsSize = sizeof(GridMaterialParams),
		.transparency = true,
	});

	queueForDeletion([=] { g_transparentGridTemplate.reset(); });
}
