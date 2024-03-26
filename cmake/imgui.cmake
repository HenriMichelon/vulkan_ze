FetchContent_Declare(
        fetch_imgui
        GIT_REPOSITORY https://github.com/ocornut/imgui
        GIT_TAG        v1.90.4
)
FetchContent_MakeAvailable(fetch_imgui)

add_compile_definitions(IMGUI_IMPL_VULKAN_NO_PROTOTYPES)
set(IMGUI_SOURCES_DIR ${CMAKE_BINARY_DIR}/_deps/fetch_imgui-src)
set(IMGUI_SOURCES
    ${IMGUI_SOURCES_DIR}/imgui.cpp
    ${IMGUI_SOURCES_DIR}/imgui_draw.cpp
    ${IMGUI_SOURCES_DIR}/imgui_widgets.cpp
    ${IMGUI_SOURCES_DIR}/imgui_tables.cpp
    ${IMGUI_SOURCES_DIR}/backends/imgui_impl_vulkan.cpp
    ${IMGUI_SOURCES_DIR}/backends/imgui_impl_glfw.cpp
)

include_directories(${IMGUI_SOURCES_DIR})
include_directories(${IMGUI_SOURCES_DIR}/backends)