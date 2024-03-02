find_package(Vulkan REQUIRED)
target_include_directories(${PROJECT_NAME} PUBLIC ${Vulkan_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} Vulkan::Vulkan)

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
        fetch_tinyobjloader
        GIT_REPOSITORY https://github.com/tinyobjloader/tinyobjloader
        GIT_TAG        v2.0.0rc13
)
FetchContent_MakeAvailable(fetch_tinyobjloader)
target_link_libraries(${PROJECT_NAME} tinyobjloader)