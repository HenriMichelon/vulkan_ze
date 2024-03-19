#pragma once

#include <volk.h>
#include <iostream>

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#ifdef _WIN64
  #define GLFW_EXPOSE_NATIVE_WIN32
  #include "GLFW/glfw3native.h"
#endif

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

static void check_vk_result(VkResult err)
{
    if (err == 0)
        return;
    std::cout << "[vulkan] Error: VkResult\n" << std::endl;
    if (err < 0)
        abort();
}

#include "z0/window.hpp"
#include <string>

namespace z0 {

    class WindowHelper {
    public:
        WindowHelper(WindowMode mode, uint32_t w, uint32_t h, const std::string& windowName);

        // accessed by static function framebufferResizeCallback()
        bool windowResized = false;

        bool shouldClose() { return glfwWindowShouldClose(windowHandle); };
        void process() { return glfwPollEvents(); };
        void close();

        int getWidth() const { return width; }
        int getHeight() const { return height; }

#ifdef GLFW_VERSION_MAJOR
        GLFWwindow *getWindowHandle() { return windowHandle; }
#endif
    private:
        WindowMode mode;
        uint32_t width, height;

#ifdef GLFW_VERSION_MAJOR
        GLFWwindow *windowHandle;
#endif
    };

}
