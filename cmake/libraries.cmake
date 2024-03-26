find_package(Vulkan REQUIRED)
target_include_directories(${PROJECT_NAME} PUBLIC ${Vulkan_INCLUDE_DIRS})

FetchContent_Declare(
        fetch_volk
        GIT_REPOSITORY https://github.com/zeux/volk
        GIT_TAG        vulkan-sdk-1.3.275.0
)
FetchContent_MakeAvailable(fetch_volk)
if (WIN32)
    set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_WIN32_KHR)
endif()
target_link_libraries(${PROJECT_NAME} volk::volk)

FetchContent_Declare(
        fetch_glfw
        GIT_REPOSITORY https://github.com/glfw/glfw
        GIT_TAG        3.4
)
FetchContent_MakeAvailable(fetch_glfw)
target_link_libraries(${PROJECT_NAME} glfw)

FetchContent_Declare(
        fetch_glm
        GIT_REPOSITORY https://github.com/g-truc/glm
        GIT_TAG        1.0.1
)
FetchContent_MakeAvailable(fetch_glm)
target_link_libraries(${PROJECT_NAME} glm::glm)

FetchContent_Declare(
        fetch_fastgltf
        GIT_REPOSITORY https://github.com/spnda/fastgltf
        GIT_TAG        v0.7.1
)
FetchContent_MakeAvailable(fetch_fastgltf)
target_link_libraries(${PROJECT_NAME} fastgltf)
