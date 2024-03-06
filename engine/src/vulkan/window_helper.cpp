#include "z0/vulkan/window_helper.hpp"

#include <stdexcept>

namespace z0 {

    // Called by GLFW when the window is resized/minimized
    static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
        auto device = reinterpret_cast<WindowHelper*>(glfwGetWindowUserPointer(window));
        device->windowResized = true;
    }

    // Create the GLW Window on the primary monitor and center the window
    WindowHelper::WindowHelper(int w, int h, const std::string& windowName): width{w}, height{h} {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
        windowHandle = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);
        glfwSetWindowUserPointer(windowHandle, this);
        glfwSetFramebufferSizeCallback(windowHandle, framebufferResizeCallback);
        const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
        glfwSetWindowPos(windowHandle, (mode->width - width)/2, (mode->height - height)/2);
    }

    void WindowHelper::close() {
        glfwDestroyWindow(windowHandle);
        glfwTerminate();
    }
}