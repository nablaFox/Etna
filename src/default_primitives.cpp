#include "etna/default_primitives.hpp"
#include "etna/primitives.hpp"
#include "etna/engine.hpp"

using namespace etna;

namespace {

MeshHandle g_sphere{nullptr};
MeshHandle g_cube{nullptr};
MeshHandle g_pyramid{nullptr};
MeshHandle g_quad{nullptr};

}

MeshHandle engine::getSphere() {
	initSphere();

	return g_sphere;
}

MeshHandle engine::getCube() {
	initCube();

	return g_cube;
}

MeshHandle engine::getPyramid() {
	initPyramid();

	return g_pyramid;
}

MeshHandle engine::getQuad() {
	initQuad();

	return g_quad;
}

void engine::initDefaultPrimitives() {
	initSphere();
	initPyramid();
	initQuad();
}

void engine::initSphere() {
	if (g_sphere != nullptr) {
		return;
	}

	g_sphere = createSphere(DEFAULT_SPHERE_RADIUS, DEFAULT_SPHERE_PRECISION);

	engine::queueForDeletion([=] { g_sphere.reset(); });
}

void engine::initCube() {
	if (g_cube != nullptr) {
		return;
	}

	g_cube = createCube(DEFAULT_CUBE_SIDE);

	engine::queueForDeletion([=] { g_cube.reset(); });
}

void engine::initPyramid() {
	if (g_pyramid != nullptr) {
		return;
	}

	g_pyramid = createPyramid(DEFAULT_PYRAMID_HEIGHT, DEFAULT_PYRAMID_SIDE_LENGTH);

	engine::queueForDeletion([=] { g_pyramid.reset(); });
}

void engine::initQuad() {
	if (g_quad != nullptr) {
		return;
	}

	g_quad = createQuad(DEFAULT_QUAD_SIDE, DEFAULT_QUAD_SIDE);

	engine::queueForDeletion([=] { g_quad.reset(); });
}
