#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <algorithm>
#include "window.hpp"
#include "engine.hpp"
#include "semaphore.hpp"

using namespace etna;
using namespace ignis;

Window::Window(const CreateInfo& info)
	: RenderTarget({
		  .extent = {info.width, info.height},
		  .samples = std::min(
			  static_cast<uint32_t>(Engine::getDevice().getMaxSampleCount()),
			  8u),
	  }),
	  m_creationInfo(info) {
	Device& device = Engine::getDevice();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	m_window =
		glfwCreateWindow(info.width, info.height, info.title, nullptr, nullptr);

	if (glfwCreateWindowSurface(device.getInstance(), m_window, nullptr,
								&m_surface) != VK_SUCCESS) {
		throw std::runtime_error("failed to create window surface!");
	}

	m_swapchain = new Swapchain({
		.device = &device,
		.extent = {info.width, info.height},
		.surface = m_surface,
		.presentMode = VK_PRESENT_MODE_MAILBOX_KHR,
	});

	m_blitCmd = new Command({
		.device = device,
		.queue = device.getQueue(0),
	});

	m_imageAvailable = new Semaphore(device);

	m_finishedBlitting = new Semaphore(device);
}

Window::~Window() {
	Engine::getDevice().waitIdle();

	delete m_finishedBlitting;

	delete m_imageAvailable;

	delete m_blitCmd;

	delete m_swapchain;

	vkDestroySurfaceKHR(Engine::getDevice().getInstance(), m_surface, nullptr);

	glfwDestroyWindow(m_window);
}

bool Window::shouldClose() const {
	glfwPollEvents();
	return glfwWindowShouldClose(m_window);
}

bool Window::isKeyPressed(int key) const {
	return glfwGetKey(m_window, key) == GLFW_PRESS;
}

bool Window::isKeyClicked(int key) {
	bool currentState = isKeyPressed(key);
	bool wasPressed = false;

	auto it = m_prevKeyStates.find(key);
	if (it != m_prevKeyStates.end()) {
		wasPressed = it->second;
	}

	bool clicked = currentState && !wasPressed;

	m_prevKeyStates[key] = currentState;
	return clicked;
}

double Window::getMouseX() const {
	double x, y;
	glfwGetCursorPos(m_window, &x, &y);
	return x;
}

double Window::getMouseY() const {
	double x, y;
	glfwGetCursorPos(m_window, &x, &y);
	return y;
}

double Window::mouseDeltaX() {
	float toReturn = getMouseX() - m_lastMouseX;

	m_lastMouseX = getMouseX();

	return toReturn;
}

double Window::mouseDeltaY() {
	float toReturn = getMouseY() - m_lastMouseY;

	m_lastMouseY = getMouseY();

	return toReturn;
}

void Window::swapBuffers() {
	Image& swapchainImage = m_swapchain->acquireNextImage(m_imageAvailable);
	Image& resolvedImage = *m_resolvedImage;

	m_blitCmd->begin();

	m_blitCmd->transitionImageLayout(resolvedImage,
									 VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

	m_blitCmd->transitionImageLayout(swapchainImage,
									 VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	m_blitCmd->blitImage(resolvedImage, swapchainImage);

	m_blitCmd->transitionToOptimalLayout(swapchainImage);

	m_blitCmd->transitionToOptimalLayout(*m_resolvedImage);

	m_blitCmd->end();

	SubmitCmdInfo blitCmdInfo{
		.command = *m_blitCmd,
		.waitSemaphores = {m_imageAvailable},
		.signalSemaphores = {m_finishedBlitting},
	};

	Engine::getDevice().submitCommands({blitCmdInfo}, nullptr);

	m_swapchain->presentCurrent({.waitSemaphores = {m_finishedBlitting}});
}
