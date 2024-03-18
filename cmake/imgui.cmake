set(IMGUI_IMPL_VULKAN_HAS_DYNAMIC_RENDERING)
set(IMGUI_SOURCES
    ${Z0_LIBRARIES_DIR}/imgui/imgui.cpp
    ${Z0_LIBRARIES_DIR}/imgui/imgui_draw.cpp
    ${Z0_LIBRARIES_DIR}/imgui/imgui_widgets.cpp
    ${Z0_LIBRARIES_DIR}/imgui/imgui_tables.cpp
    ${Z0_LIBRARIES_DIR}/imgui/imgui_demo.cpp
    ${Z0_LIBRARIES_DIR}/imgui/backends/imgui_impl_vulkan.cpp
    ${Z0_LIBRARIES_DIR}/imgui/backends/imgui_impl_glfw.cpp
)

include_directories(${Z0_LIBRARIES_DIR}/imgui)
include_directories(${Z0_LIBRARIES_DIR}/imgui/backends)
