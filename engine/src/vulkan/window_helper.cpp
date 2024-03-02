#include "z0/vulkan/window_helper.hpp"

#include <stdexcept>

namespace z0 {
    WindowHelper::WindowHelper(int w, int h, const std::string& windowName): width{w}, height{h} {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
        windowHandle = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);
        glfwSetWindowUserPointer(windowHandle, this);
        const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
        glfwSetWindowPos(windowHandle, (mode->width - width)/2, (mode->height - height)/2);
    }

    void WindowHelper::close() {
        glfwDestroyWindow(windowHandle);
        glfwTerminate();
    }
}