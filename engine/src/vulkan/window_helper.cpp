#include "z0/vulkan/window_helper.hpp"

#include <stdexcept>

namespace z0 {

    // Called by GLFW when the window is resized/minimized
    static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
        auto device = reinterpret_cast<WindowHelper*>(glfwGetWindowUserPointer(window));
        device->windowResized = true;
    }

    WindowHelper::WindowHelper(WindowMode _mode, int w, int h, const std::string& windowName):
        mode{_mode}, width{w}, height{h}
    {
        // Initialize GLFW
        glfwInit();

        // Create the GLW Window on the primary monitor
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        const GLFWvidmode* vidmod = glfwGetVideoMode(glfwGetPrimaryMonitor());
        if (mode != WINDOW_MODE_WINDOWED) {
            // ignore the requested window size in full screen modes
            width = vidmod->width;
            height = vidmod->height;
        }
        glfwWindowHint(GLFW_RESIZABLE, mode == WINDOW_MODE_WINDOWED);
        windowHandle = glfwCreateWindow(
                width,
                height,
                windowName.c_str(),
                mode == WINDOW_MODE_WINDOWED ? nullptr : glfwGetPrimaryMonitor(),
                nullptr);

        if (mode == WINDOW_MODE_WINDOWED) {
            // Center the window on the screen
            glfwSetWindowPos(windowHandle, (vidmod->width - width) / 2, (vidmod->height - height) / 2);
        }

        // Set callback for window resize/minimize
        glfwSetWindowUserPointer(windowHandle, this);
        glfwSetFramebufferSizeCallback(windowHandle, framebufferResizeCallback);
    }

    void WindowHelper::close() {
        glfwDestroyWindow(windowHandle);
        glfwTerminate();
    }
}