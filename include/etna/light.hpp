#pragma once

#include "ignis/types.hpp"
#include "color.hpp"
#include "math.hpp"

namespace etna {

struct DirectionalLight {
	struct CreateInfo {
		std::string name;
		Vec3 direction;
		float intensity{1.f};
		Color color;
	};

	DirectionalLight(const CreateInfo&);
	~DirectionalLight();

	ignis::BufferId getDataBuffer() const { return m_buffer; }

	Vec3 getDirection() const { return m_direction; }
	float getIntensity() const { return m_intensity; }

	std::string getName() const { return m_name; }

private:
	Vec3 m_direction;
	float m_intensity;
	Color m_color;
	std::string m_name;

	ignis::BufferId m_buffer{IGNIS_INVALID_BUFFER_ID};

	// TEMP: will contain also info for shadows (viewproj etc.)
	struct DirectionalLightData {
		Vec3 direction;
		float intensity;
		Color color;
	};
};

}  // namespace etna
