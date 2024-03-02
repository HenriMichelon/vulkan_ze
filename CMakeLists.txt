include(.env.cmake)
include(cmake/shaders.cmake)
include(FetchContent)

cmake_minimum_required(VERSION 3.27)
project(ZeroZero)

set(CMAKE_CXX_STANDARD 23)
set(Z0_ENGINE_DIR engine)

add_executable(${PROJECT_NAME}
        example/main.cpp
        ${Z0_ENGINE_DIR}/include/z0/vulkan/window.hpp
        ${Z0_ENGINE_DIR}/include/z0/vulkan/window.hpp
        ${Z0_ENGINE_DIR}/include/z0/vulkan/main_loop.hpp
        ${Z0_ENGINE_DIR}/include/z0/run_loop.hpp
        ${Z0_ENGINE_DIR}/include/z0/vulkan/os.hpp
        ${Z0_ENGINE_DIR}/include/z0/log.hpp
        ${Z0_ENGINE_DIR}/include/z0/vulkan/device.hpp
        ${Z0_ENGINE_DIR}/include/z0/application.hpp
        ${Z0_ENGINE_DIR}/src/vulkan/window_glfw.cpp
        ${Z0_ENGINE_DIR}/src/vulkan/main_loop.cpp
        ${Z0_ENGINE_DIR}/src/vulkan/device.cpp
        ${Z0_ENGINE_DIR}/src/application.cpp)
target_include_directories(${PROJECT_NAME} PUBLIC ${Z0_ENGINE_DIR}/include)

file(GLOB_RECURSE Z0_GLSL_SOURCE_FILES
        "${Z0_ENGINE_DIR}/shaders/*.frag"
        "${Z0_ENGINE_DIR}/shaders/*.vert"
)
add_shaders(${PROJECT_NAME} ${Z0_GLSL_SOURCE_FILES})

include(cmake/libraries.cmake)
