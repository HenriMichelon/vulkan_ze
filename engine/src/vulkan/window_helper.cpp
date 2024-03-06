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
        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* vidmod = glfwGetVideoMode(monitor);
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // disable OpenGL API support
        const bool isFullScreen = mode == WINDOW_MODE_FULLSCREEN || mode == WINDOW_MODE_FULLSCREEN_EXCLUSIVE;
        glfwWindowHint(GLFW_RESIZABLE, mode == WINDOW_MODE_WINDOWED || mode == WINDOW_MODE_WINDOWED_MAXIMIZED);
        glfwWindowHint(GLFW_MAXIMIZED, mode == WINDOW_MODE_WINDOWED_MAXIMIZED);
        windowHandle = glfwCreateWindow(
            isFullScreen ? vidmod->width : width,
            isFullScreen ? vidmod->height : height,
            windowName.c_str(),
            isFullScreen ? monitor : nullptr,
            nullptr);
        if (mode == WINDOW_MODE_WINDOWED) {
            // Center the window on the screen
            glfwSetWindowPos(windowHandle, (vidmod->width - width) / 2, (vidmod->height - height) / 2);
        } else if (mode == WINDOW_MODE_WINDOWED_FULLSCREEN) {
            // switch in "window fullscreen" mode
            glfwSetWindowMonitor(windowHandle, monitor, 0, 0,
                                 vidmod->width, vidmod->height, vidmod->refreshRate);
        }

#ifdef _WIN64
        // Clean window background
        HWND hWnd = glfwGetWin32Window(windowHandle);
        PAINTSTRUCT ps2;
        HDC hdc = BeginPaint(hWnd, &ps2);
        HBRUSH brush = CreateSolidBrush(RGB(WINDOW_CLEAR_COLOR[0], WINDOW_CLEAR_COLOR[1], WINDOW_CLEAR_COLOR[2]));
        RECT rect = { };
        GetClientRect(hWnd, &rect);
        FillRect(hdc, &rect, brush);
        DeleteObject(brush);
        EndPaint(hWnd, &ps2);
#endif

        // Set callback for window resize/minimize
        glfwSetWindowUserPointer(windowHandle, this);
        glfwSetFramebufferSizeCallback(windowHandle, framebufferResizeCallback);
    }

    void WindowHelper::close() {
        glfwDestroyWindow(windowHandle);
        glfwTerminate();
    }
}