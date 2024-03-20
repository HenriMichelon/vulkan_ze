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
        WindowHelper(WindowMode mode, int w, int h, const std::string& windowName);

        bool shouldClose() { return glfwWindowShouldClose(windowHandle); };
        void process() { return glfwPollEvents(); };
        void close();

        int getWidth() const { return _width; }
        int getHeight() const { return _height; }

#ifdef GLFW_VERSION_MAJOR
        GLFWwindow *getWindowHandle() { return windowHandle; }
#endif
    private:
        WindowMode mode;

#ifdef GLFW_VERSION_MAJOR
        GLFWwindow *windowHandle;
#endif

    public:
        // accessed by static function framebufferResizeCallback()
        bool _windowResized = false;
        int _width, _height;
    };

}
