#pragma once

#include <functional>
#include "ignis/image.hpp"
#include "ignis/device.hpp"
#include "ignis/swapchain.hpp"
#include "ignis/semaphore.hpp"
#include "math.hpp"

namespace etna::engine {

struct InitInfo {
	std::string appName{"Etna App"};
	std::string shadersFolder{"shaders"};
};

void init(const InitInfo& = {});

ignis::Device& getDevice();

void immediateSubmit(std::function<void(ignis::Command&)>&&);

void immediateUpload(ignis::BufferId,
					 const void* data,
					 VkDeviceSize offset = 0,
					 VkDeviceSize size = 0);

void immediateUpdate(ignis::BufferId,
					 const void* data,
					 VkDeviceSize offset = 0,
					 VkDeviceSize size = 0);

void presentCurrent(const ignis::Swapchain&, std::vector<const ignis::Semaphore*>);

ignis::Command createGraphicsCommand();

ignis::Command* newGraphicsCommand();

void queueForDeletion(std::function<void()>);

uint32_t clampSampleCount(uint32_t sampleCount);

ignis::Shader* newShader(const std::string& path);

float getDeltaTime();

float updateTime();

constexpr ignis::ColorFormat COLOR_FORMAT{ignis::ColorFormat::RGBA16};

constexpr ignis::DepthFormat DEPTH_FORMAT{ignis::DepthFormat::D32_SFLOAT};

constexpr uint32_t MAX_SAMPLE_COUNT{8};

struct PushConstants {
	Mat4 model;
	ignis::BufferId vertices;
	ignis::BufferId material;
	ignis::BufferId instanceBuffer;
	ignis::BufferId buff1;
	ignis::BufferId buff2;
	ignis::BufferId buff3;
};

}  // namespace etna::engine

#define _device etna::engine::getDevice()
