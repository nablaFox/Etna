## Etna Engine

Simple C++ 20 graphics engine written with [Ignis](https://github.com/nablaFox/Ignis).

### Example

```cpp
#include "etna/etna_core.hpp"
#include "etna/scene.hpp"

constexpr uint32_t WINDOW_WIDTH{800};
constexpr uint32_t WINDOW_HEIGHT{600};

using namespace etna;

int main(void) {
	engine::init();

	Window window({
		.width = WINDOW_WIDTH,
		.height = WINDOW_HEIGHT,
		.title = "Basic Etna",
		.captureMouse = true,
	});

	Scene scene({});

	MeshNode sphere = scene.createMeshNode({
		.name = "Sphere",
		.mesh = engine::getSphere(),
		.transform =
			{
				.position = {0, 0, -3},
				.scale = Vec3(0.5),
			},
		.material = engine::createGridMaterial({
			.color = BLUE * 0.08,
			.gridColor = BLUE,
			.gridSpacing = 0.1,
			.thickness = 0.005,
		}),
	});

	CameraNode playerCamera = scene.createCameraNode({
		.name = "PlayerCamera",
		.transform = {.position = {0, 0, 1}},
	});

	Renderer renderer({});

	while (!window.shouldClose()) {
		window.pollEvents();

		sphere->rotate(0, 0.01, 0.01);

		renderer.beginFrame(window);

		scene.render(renderer, playerCamera);

		renderer.endFrame();

		window.swapBuffers();
	}
}
```

**Output**:

![Etna Demo Gif](./docs/example.gif)

For more check-out [`examples`](./examples).

### Building/Installation

Etna is builded with CMake as a static library:

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
sudo cmake --install build  # Omit 'sudo' for local user install
```

You can find a prebuilt version [here]().

### Usage

Standalone Build:

```bash
g++ main.cpp -Ipath/to/etna/include -Lpath/to/etna/lib -letna
```

With CMake:

```cmake
add_subdirectory(etna_path) # or find_package(etna REQUIRED) if system-wide installed
target_link_libraries(your_target PRIVATE etna::etna)
```

### Dependencies

- [Ignis](https://github.com/nablaFox/Ignis)
- [GLFW](https://github.com/glfw/glfw)

### Documentation

Work in progress.
