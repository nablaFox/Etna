#include "default_materials.hpp"
#include "engine.hpp"

using namespace etna;

MaterialTemplateHandle g_colorMaterialTemplate{nullptr};
MaterialTemplateHandle g_pointMaterialTemplate{nullptr};
MaterialTemplateHandle g_gridTemplate{nullptr};
MaterialTemplateHandle g_transparentGridTemplate{nullptr};

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
		.shaders = {"default.vert", "default.frag"},
		.paramsSize = sizeof(Color),
	});

	queueForDeletion([=] { g_colorMaterialTemplate.reset(); });
}

void engine::initPointMaterial() {
	if (g_pointMaterialTemplate != nullptr) {
		return;
	}

	g_pointMaterialTemplate = MaterialTemplate::create({
		.shaders = {"default.vert", "default.frag"},
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
		.shaders = {"default.vert", "grid.frag"},
		.paramsSize = sizeof(GridMaterialParams),
	});

	queueForDeletion([=] { g_gridTemplate.reset(); });
}

void engine::initTransparentGridMaterial() {
	if (g_transparentGridTemplate != nullptr) {
		return;
	}

	g_transparentGridTemplate = MaterialTemplate::create({
		.shaders = {"default.vert", "grid.frag"},
		.paramsSize = sizeof(GridMaterialParams),
		.transparency = true,
	});

	queueForDeletion([=] { g_transparentGridTemplate.reset(); });
}
