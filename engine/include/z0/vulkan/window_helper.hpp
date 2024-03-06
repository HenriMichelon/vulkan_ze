#pragma once

#include <volk.h>
#ifdef _WIN64
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#define GLFW_EXPOSE_NATIVE_WIN32
#include "GLFW/glfw3native.h"
#else
#error OS Not supported
#endif

#include <string>

namespace z0 {

    class WindowHelper {
    public:
        WindowHelper(int w, int h, const std::string& windowName);

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
        int width, height;

#ifdef GLFW_VERSION_MAJOR
        GLFWwindow *windowHandle;
#endif
    };

}