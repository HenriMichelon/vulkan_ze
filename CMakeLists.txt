include(.env.cmake)
include(cmake/shaders.cmake)
include(FetchContent)

cmake_minimum_required(VERSION 3.27)
project(ZeroZero)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_COMPILE_WARNING_AS_ERROR ON)
set(Z0_ENGINE_DIR engine)

add_executable(${PROJECT_NAME}
        example/main.cpp
        ${Z0_ENGINE_DIR}/include/z0/vulkan/window_helper.hpp
        ${Z0_ENGINE_DIR}/include/z0/vulkan/vulkan_device.hpp
        ${Z0_ENGINE_DIR}/include/z0/vulkan/vulkan_buffer.hpp
        ${Z0_ENGINE_DIR}/include/z0/vulkan/vulkan_model.hpp
        ${Z0_ENGINE_DIR}/include/z0/vulkan/vulkan_renderer.hpp
        ${Z0_ENGINE_DIR}/include/z0/vulkan/vulkan_shader.hpp
        ${Z0_ENGINE_DIR}/include/z0/vulkan/vulkan_descriptors.hpp
        ${Z0_ENGINE_DIR}/include/z0/vulkan/vulkan_instance.hpp
        ${Z0_ENGINE_DIR}/include/z0/vulkan/default_renderer.hpp
        ${Z0_ENGINE_DIR}/src/vulkan/vulkan_image.cpp
        ${Z0_ENGINE_DIR}/include/z0/nodes/node.hpp
        ${Z0_ENGINE_DIR}/include/z0/nodes/camera.hpp
        ${Z0_ENGINE_DIR}/include/z0/application_config.hpp
        ${Z0_ENGINE_DIR}/include/z0/application.hpp
        ${Z0_ENGINE_DIR}/include/z0/log.hpp
        ${Z0_ENGINE_DIR}/include/z0/object.hpp
        ${Z0_ENGINE_DIR}/include/z0/transform.hpp
        ${Z0_ENGINE_DIR}/include/z0/window.hpp
        ${Z0_ENGINE_DIR}/include/z0/viewport.hpp
        ${Z0_ENGINE_DIR}/include/z0/mesh.hpp
        ${Z0_ENGINE_DIR}/src/vulkan/window_helper.cpp
        ${Z0_ENGINE_DIR}/src/vulkan/vulkan_device.cpp
        ${Z0_ENGINE_DIR}/src/vulkan/vulkan_buffer.cpp
        ${Z0_ENGINE_DIR}/src/vulkan/vulkan_model.cpp
        ${Z0_ENGINE_DIR}/src/vulkan/vulkan_renderer.cpp
        ${Z0_ENGINE_DIR}/src/vulkan/vulkan_shader.cpp
        ${Z0_ENGINE_DIR}/src/vulkan/vulkan_descriptors.cpp
        ${Z0_ENGINE_DIR}/src/vulkan/vulkan_instance.cpp
        ${Z0_ENGINE_DIR}/src/vulkan/vulkan_image.cpp
        ${Z0_ENGINE_DIR}/src/vulkan/default_renderer.cpp
        ${Z0_ENGINE_DIR}/src/nodes/node.cpp
        ${Z0_ENGINE_DIR}/src/nodes/camera.cpp
        ${Z0_ENGINE_DIR}/src/application.cpp
        ${Z0_ENGINE_DIR}/src/object.cpp
        ${Z0_ENGINE_DIR}/src/viewport.cpp
        ${Z0_ENGINE_DIR}/src/mesh.cpp
        ${Z0_ENGINE_DIR}/src/transform.cpp
        engine/include/z0/material.hpp
        engine/include/z0/texture.hpp
        engine/include/z0/image.hpp
        engine/src/image.cpp
        engine/src/texture.cpp
        engine/include/z0/nodes/mesh_instance.hpp
        engine/include/z0/resource.hpp
        engine/src/resource.cpp
)
target_include_directories(${PROJECT_NAME} PUBLIC ${Z0_ENGINE_DIR}/include)

file(GLOB_RECURSE Z0_GLSL_SOURCE_FILES
        "${Z0_ENGINE_DIR}/shaders/*.frag"
        "${Z0_ENGINE_DIR}/shaders/*.vert"
)
add_shaders(${PROJECT_NAME} ${Z0_GLSL_SOURCE_FILES})

include(cmake/libraries.cmake)
