cmake_minimum_required(VERSION 3.22)
cmake_policy(SET CMP0002 NEW)

project(ZeroZero)

set(Z0_ENGINE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/engine)
set(Z0_SHADERS_DIR ${Z0_ENGINE_DIR}/shaders)
set(Z0_SHADERS_BUILD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/shaders)
make_directory(${Z0_SHADERS_BUILD_DIR})

include(FetchContent)
if(EXISTS .env.cmake)
	include(.env.cmake)
endif ()
include(cmake/shaders.cmake)
include(cmake/imgui.cmake)

set(CMAKE_CXX_STANDARD 23)
#set(CMAKE_COMPILE_WARNING_AS_ERROR ON)
add_compile_definitions(VULKAN_STATS)

file(GLOB_RECURSE Z0_GLSL_SOURCE_FILES
		"${Z0_SHADERS_DIR}/*.frag"
		"${Z0_SHADERS_DIR}/*.vert"
)
add_shaders(${PROJECT_NAME}_shaders ${Z0_GLSL_SOURCE_FILES})

add_executable(${PROJECT_NAME}
        ${Z0_ENGINE_DIR}/include/z0/helpers/window_helper.hpp
        ${Z0_ENGINE_DIR}/include/z0/vulkan/vulkan_device.hpp
        ${Z0_ENGINE_DIR}/include/z0/vulkan/vulkan_buffer.hpp
        ${Z0_ENGINE_DIR}/include/z0/vulkan/vulkan_model.hpp
        ${Z0_ENGINE_DIR}/include/z0/vulkan/vulkan_renderer.hpp
        ${Z0_ENGINE_DIR}/include/z0/vulkan/vulkan_shader.hpp
        ${Z0_ENGINE_DIR}/include/z0/vulkan/vulkan_descriptors.hpp
        ${Z0_ENGINE_DIR}/include/z0/vulkan/vulkan_instance.hpp
		${Z0_ENGINE_DIR}/include/z0/vulkan/vulkan_cubemap.hpp
		${Z0_ENGINE_DIR}/include/z0/vulkan/renderers/base_renderer.hpp
		${Z0_ENGINE_DIR}/include/z0/vulkan/renderers/scene_renderer.hpp
        ${Z0_ENGINE_DIR}/include/z0/vulkan/renderers/shadowmap_renderer.hpp
		${Z0_ENGINE_DIR}/include/z0/vulkan/renderers/depth_prepass_renderer.hpp
		${Z0_ENGINE_DIR}/include/z0/vulkan/renderers/base_meshes_renderer.hpp
		${Z0_ENGINE_DIR}/include/z0/vulkan/renderers/skybox_renderer.hpp
		${Z0_ENGINE_DIR}/include/z0/vulkan/renderers/tonemapping_renderer.hpp
		${Z0_ENGINE_DIR}/include/z0/vulkan/framebuffers/shadow_map.hpp
		${Z0_ENGINE_DIR}/include/z0/vulkan/framebuffers/depth_buffer.hpp
		${Z0_ENGINE_DIR}/include/z0/vulkan/framebuffers/base_frame_buffer.hpp
		${Z0_ENGINE_DIR}/include/z0/vulkan/framebuffers/color_attachement_multisampled.hpp
		${Z0_ENGINE_DIR}/include/z0/vulkan/framebuffers/color_attachement_hdr.hpp
		${Z0_ENGINE_DIR}/include/z0/vulkan/vulkan_stats.hpp
        ${Z0_ENGINE_DIR}/include/z0/nodes/node.hpp
        ${Z0_ENGINE_DIR}/include/z0/nodes/camera.hpp
        ${Z0_ENGINE_DIR}/include/z0/nodes/mesh_instance.hpp
        ${Z0_ENGINE_DIR}/include/z0/nodes/light.hpp
        ${Z0_ENGINE_DIR}/include/z0/nodes/directional_light.hpp
        ${Z0_ENGINE_DIR}/include/z0/nodes/environment.hpp
        ${Z0_ENGINE_DIR}/include/z0/nodes/omni_light.hpp
        ${Z0_ENGINE_DIR}/include/z0/nodes/spot_light.hpp
		${Z0_ENGINE_DIR}/include/z0/nodes/skybox.hpp
		${Z0_ENGINE_DIR}/include/z0/ui/debug_ui.hpp
        ${Z0_ENGINE_DIR}/include/z0/application_config.hpp
        ${Z0_ENGINE_DIR}/include/z0/mainloop.hpp
        ${Z0_ENGINE_DIR}/include/z0/log.hpp
        ${Z0_ENGINE_DIR}/include/z0/object.hpp
        ${Z0_ENGINE_DIR}/include/z0/transform.hpp
        ${Z0_ENGINE_DIR}/include/z0/window.hpp
        ${Z0_ENGINE_DIR}/include/z0/viewport.hpp
        ${Z0_ENGINE_DIR}/include/z0/mesh.hpp
        ${Z0_ENGINE_DIR}/include/z0/resource.hpp
        ${Z0_ENGINE_DIR}/include/z0/material.hpp
        ${Z0_ENGINE_DIR}/include/z0/texture.hpp
        ${Z0_ENGINE_DIR}/include/z0/image.hpp
        ${Z0_ENGINE_DIR}/include/z0/color.hpp
        ${Z0_ENGINE_DIR}/include/z0/vertex.hpp
        ${Z0_ENGINE_DIR}/include/z0/scene.hpp
        ${Z0_ENGINE_DIR}/include/z0/loader.hpp
		${Z0_ENGINE_DIR}/include/z0/input.hpp
		${Z0_ENGINE_DIR}/include/z0/cubemap.hpp
		${Z0_ENGINE_DIR}/include/z0/application.hpp
		${Z0_ENGINE_DIR}/include/z0/input_event.hpp
        ${Z0_ENGINE_DIR}/src/helpers/window_helper_glfw.cpp
        ${Z0_ENGINE_DIR}/src/vulkan/vulkan_device.cpp
        ${Z0_ENGINE_DIR}/src/vulkan/vulkan_buffer.cpp
        ${Z0_ENGINE_DIR}/src/vulkan/vulkan_model.cpp
        ${Z0_ENGINE_DIR}/src/vulkan/vulkan_shader.cpp
        ${Z0_ENGINE_DIR}/src/vulkan/vulkan_descriptors.cpp
        ${Z0_ENGINE_DIR}/src/vulkan/vulkan_instance.cpp
        ${Z0_ENGINE_DIR}/src/vulkan/vulkan_image.cpp
        ${Z0_ENGINE_DIR}/src/vulkan/vulkan_stats.cpp
		${Z0_ENGINE_DIR}/src/vulkan/vulkan_cubemap.cpp
		${Z0_ENGINE_DIR}/src/vulkan/renderers/base_renderer.cpp
		${Z0_ENGINE_DIR}/src/vulkan/renderers/scene_renderer.cpp
        ${Z0_ENGINE_DIR}/src/vulkan/renderers/shadowmap_renderer.cpp
		${Z0_ENGINE_DIR}/src/vulkan/renderers/depth_prepass_renderer.cpp
		${Z0_ENGINE_DIR}/src/vulkan/renderers/base_meshes_renderer.cpp
		${Z0_ENGINE_DIR}/src/vulkan/renderers/skybox_renderer.cpp
		${Z0_ENGINE_DIR}/src/vulkan/framebuffers/depth_buffer.cpp
		${Z0_ENGINE_DIR}/src/vulkan/framebuffers/shadow_map.cpp
		${Z0_ENGINE_DIR}/src/vulkan/framebuffers/color_attachement_hdr.cpp
		${Z0_ENGINE_DIR}/src/vulkan/framebuffers/base_frame_buffer.cpp
		${Z0_ENGINE_DIR}/src/vulkan/framebuffers/color_attachement_multisampled.cpp
		${Z0_ENGINE_DIR}/src/ui/debug_ui.cpp
        ${Z0_ENGINE_DIR}/src/nodes/node.cpp
        ${Z0_ENGINE_DIR}/src/nodes/camera.cpp
        ${Z0_ENGINE_DIR}/src/nodes/mesh_instance.cpp
        ${Z0_ENGINE_DIR}/src/nodes/omni_light.cpp
        ${Z0_ENGINE_DIR}/src/nodes/spot_light.cpp
		${Z0_ENGINE_DIR}/src/nodes/skybox.cpp
        ${Z0_ENGINE_DIR}/src/mainloop.cpp
        ${Z0_ENGINE_DIR}/src/object.cpp
        ${Z0_ENGINE_DIR}/src/viewport.cpp
        ${Z0_ENGINE_DIR}/src/mesh.cpp
        ${Z0_ENGINE_DIR}/src/transform.cpp
        ${Z0_ENGINE_DIR}/src/image.cpp
        ${Z0_ENGINE_DIR}/src/texture.cpp
        ${Z0_ENGINE_DIR}/src/resource.cpp
        ${Z0_ENGINE_DIR}/src/loader.cpp
		${Z0_ENGINE_DIR}/src/cubemap.cpp
		${Z0_ENGINE_DIR}/src/input.cpp
		${Z0_ENGINE_DIR}/src/input_event.cpp
		${Z0_ENGINE_DIR}/src/libraries.cpp
		${IMGUI_SOURCES}
		example/main.cpp
		engine/src/vulkan/renderers/tonemapping_renderer.cpp
		engine/include/z0/vulkan/framebuffers/color_attachement.hpp
		engine/src/vulkan/framebuffers/color_attachement.cpp
		engine/src/vulkan/framebuffers/color_attachement.cpp
		engine/include/z0/vulkan/renderers/postprocessing_renderer.hpp
		engine/src/vulkan/renderers/postprocessing_renderer.cpp
)

add_dependencies(${PROJECT_NAME} ${PROJECT_NAME}_shaders)
target_include_directories(${PROJECT_NAME} PUBLIC ${Z0_ENGINE_DIR}/include)

include(cmake/libraries.cmake)
