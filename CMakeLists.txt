cmake_minimum_required(VERSION 3.14 FATAL_ERROR)
project(etna)

include(cmake/CPM.cmake)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(ETNA_BUILD_EXAMPLES "Build the examples" ${PROJECT_IS_TOP_LEVEL})
option(ETNA_INSTALL "Install the library" ${PROJECT_IS_TOP_LEVEL})

find_program(GLSLC glslc REQUIRED)

file(GLOB ETNA_SRC "src/*.cpp")
add_library(etna STATIC ${ETNA_SRC})

target_include_directories(etna PUBLIC 
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_compile_definitions(etna PUBLIC
  ETNA_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
)

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -ffunction-sections -fdata-sections -Wl,--gc-sections")

CPMAddPackage(
  NAME Ignis
  GITHUB_REPOSITORY nablaFox/Ignis
  GIT_TAG main
)

CPMAddPackage(
  NAME glfw
  GITHUB_REPOSITORY glfw/glfw
  GIT_TAG 3.3.8
  OPTIONS
    "GLFW_BUILD_EXAMPLES OFF"
    "GLFW_BUILD_TESTS OFF"
    "GLFW_BUILD_DOCS OFF"
	"GLFW_INSTALL FALSE"
)

target_link_libraries(etna PUBLIC
    glfw
    Ignis
)

function(compile_shaders TARGET_NAME SHADER_SRC_DIR SHADER_DST_DIR)
    file(GLOB SHADER_SOURCES
        "${SHADER_SRC_DIR}/*.vert"
        "${SHADER_SRC_DIR}/*.frag"
    )

    set(SHADER_OUTPUTS "")
    foreach(SHADER_SOURCE ${SHADER_SOURCES})
        get_filename_component(SHADER_NAME ${SHADER_SOURCE} NAME)
        set(SHADER_OUTPUT "${SHADER_DST_DIR}/${SHADER_NAME}.spv")
        list(APPEND SHADER_OUTPUTS ${SHADER_OUTPUT})
        set(GLSLC_ARGS "-I${CMAKE_CURRENT_SOURCE_DIR}/include/etna/shaders")

        add_custom_command(
            OUTPUT ${SHADER_OUTPUT}
            COMMAND ${CMAKE_COMMAND} -E make_directory "${SHADER_DST_DIR}"
            COMMAND ${GLSLC} ${GLSLC_ARGS} "${SHADER_SOURCE}" -o "${SHADER_OUTPUT}"
            DEPENDS "${SHADER_SOURCE}"
            COMMENT "Compiling shader ${SHADER_NAME}"
        )
    endforeach()

    add_custom_target(compile_shaders_${TARGET_NAME} DEPENDS ${SHADER_OUTPUTS})
endfunction()

# these should go inside src/shaders
compile_shaders("src" "${CMAKE_CURRENT_SOURCE_DIR}/src/shaders" "${CMAKE_CURRENT_SOURCE_DIR}/src/shaders")
add_dependencies(etna compile_shaders_src)

if (ETNA_INSTALL)
  install(TARGETS etna
    EXPORT etnaTargets
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
  )

  install(DIRECTORY include/
    DESTINATION include
  )
endif()

if(ETNA_BUILD_EXAMPLES)
    # these should go inside build/shaders
    compile_shaders("examples" "${CMAKE_CURRENT_SOURCE_DIR}/examples/shaders" "${CMAKE_CURRENT_BINARY_DIR}/shaders")

    file(GLOB EXAMPLES_SRC "examples/*.cpp")
    foreach(EXAMPLE_SRC ${EXAMPLES_SRC})
        get_filename_component(EXAMPLE_NAME ${EXAMPLE_SRC} NAME_WE)
        add_executable(${EXAMPLE_NAME} ${EXAMPLE_SRC})
        add_dependencies(${EXAMPLE_NAME} compile_shaders_examples)
        target_link_libraries(${EXAMPLE_NAME} PRIVATE etna)
        target_include_directories(${EXAMPLE_NAME} PRIVATE "include")
    endforeach()
endif()
