cmake_minimum_required(VERSION 3.14 FATAL_ERROR)
project(etna)

include(cmake/CPM.cmake)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(ETNA_BUILD_EXAMPLES "Build the examples" ON)

file(GLOB ETNA_SRC "src/*.cpp")
add_library(etna STATIC ${ETNA_SRC})

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
	Ignis::Ignis
)

target_include_directories(etna PUBLIC 
	"include/etna" 
	${Ignis_SOURCE_DIR}/include
)

if(ETNA_BUILD_EXAMPLES)
	find_program(GLSLC glslc REQUIRED)

	file(GLOB ALL_SHADER_SOURCES
	  "${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.vert"
	  "${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.frag"
	  "${CMAKE_CURRENT_SOURCE_DIR}/examples/shaders/*.vert"
	  "${CMAKE_CURRENT_SOURCE_DIR}/examples/shaders/*.frag"
	)

	set(SHADER_SPV_FILES)

foreach(SHADER_SOURCE ${ALL_SHADER_SOURCES})
    get_filename_component(SHADER_DIR ${SHADER_SOURCE} DIRECTORY)
    file(RELATIVE_PATH SHADER_REL_DIR ${CMAKE_CURRENT_SOURCE_DIR} ${SHADER_DIR})
    get_filename_component(SHADER_NAME ${SHADER_SOURCE} NAME)

    set(GLSLC_ARGS)
    set(SHADER_DEPENDENCIES ${SHADER_SOURCE})

    set(OUTPUT_SUBDIR "") 
    if(SHADER_REL_DIR STREQUAL "examples/shaders")
        set(OUTPUT_SUBDIR "examples")
        list(APPEND GLSLC_ARGS "-I${CMAKE_CURRENT_SOURCE_DIR}/shaders")
	endif()

    set(BASE_SHADER_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/shaders")
    if(OUTPUT_SUBDIR)
        set(SHADER_OUTPUT "${BASE_SHADER_OUTPUT_DIR}/${OUTPUT_SUBDIR}/${SHADER_NAME}.spv")
    else()
        set(SHADER_OUTPUT "${BASE_SHADER_OUTPUT_DIR}/${SHADER_NAME}.spv")
    endif()

    get_filename_component(SHADER_OUTPUT_DIR ${SHADER_OUTPUT} DIRECTORY)
    file(MAKE_DIRECTORY ${SHADER_OUTPUT_DIR})

    add_custom_command(
        OUTPUT ${SHADER_OUTPUT}
        COMMAND ${GLSLC} ${GLSLC_ARGS} ${SHADER_SOURCE} -o ${SHADER_OUTPUT}
        DEPENDS ${SHADER_DEPENDENCIES}
        COMMENT "Compiling ${SHADER_REL_DIR}/${SHADER_NAME} to ${SHADER_OUTPUT}"
        VERBATIM
    )

    list(APPEND SHADER_SPV_FILES ${SHADER_OUTPUT})
endforeach()

	add_custom_target(compile_shaders ALL DEPENDS ${SHADER_SPV_FILES})

	file(GLOB EXAMPLES_SRC "examples/*.cpp")

	foreach(EXAMPLE_SRC ${EXAMPLES_SRC})
		get_filename_component(EXAMPLE_NAME ${EXAMPLE_SRC} NAME_WE)
		add_executable(${EXAMPLE_NAME} ${EXAMPLE_SRC})
		target_link_libraries(${EXAMPLE_NAME} PRIVATE etna)
		target_include_directories(${EXAMPLE_NAME} PRIVATE "include")
	endforeach()
endif()
