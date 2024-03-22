#include "z0/helpers/window_helper.hpp"

#include <stdexcept>

namespace z0 {

    // Called by GLFW when the window is resized/minimized
    static void glfwFramebufferResizeCallback(GLFWwindow* window, int width, int height) {
        auto windowHelper = reinterpret_cast<WindowHelper*>(glfwGetWindowUserPointer(window));
        windowHelper->_windowResized = true;
        windowHelper->_width = width;
        windowHelper->_height = height;
    }

    static void glfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
        auto windowHelper = reinterpret_cast<WindowHelper*>(glfwGetWindowUserPointer(window));
        windowHelper->_inputQueue.push_back(std::make_shared<InputEventKey>(
                (Key)key,
                (action == GLFW_PRESS) || (action == GLFW_REPEAT),
                action == GLFW_REPEAT,
                mods
                ));
    }

    static void glfwMouseMoveCallback(GLFWwindow* window, double xpos, double ypos) {
        auto windowHelper = reinterpret_cast<WindowHelper*>(glfwGetWindowUserPointer(window));
        windowHelper->_inputQueue.push_back(std::make_shared<InputEventMouseMotion>(
                static_cast<float>(xpos),
                static_cast<float>(ypos),
                static_cast<float>(xpos - windowHelper->_mouseLastX),
                static_cast<float>(ypos - windowHelper->_mouseLastY)
                ));
        windowHelper->_mouseLastX = xpos;
        windowHelper->_mouseLastY = ypos;
    }

    void WindowHelper::process() {
        glfwPollEvents();
    };

    bool WindowHelper::shouldClose() {
        return glfwWindowShouldClose(windowHandle);
    };

    std::shared_ptr<InputEvent> WindowHelper::consumeEvent() {
        auto event = _inputQueue.front();
        _inputQueue.pop_front();
        return event;
    }

    WindowHelper::WindowHelper(WindowMode _mode, int w, int h, const std::string& windowName):
        mode{_mode}, _width{w}, _height{h}
    {
        glfwInit();

        // Create the GLW Window on the primary monitor
        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* vidmod = glfwGetVideoMode(monitor);
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // disable OpenGL API support
        const bool isFullScreen = mode == WINDOW_MODE_FULLSCREEN; // || mode == WINDOW_MODE_FULLSCREEN_EXCLUSIVE;
        glfwWindowHint(GLFW_RESIZABLE, mode == WINDOW_MODE_WINDOWED || mode == WINDOW_MODE_WINDOWED_MAXIMIZED);
        glfwWindowHint(GLFW_MAXIMIZED, mode == WINDOW_MODE_WINDOWED_MAXIMIZED);
        windowHandle = glfwCreateWindow(
            isFullScreen ? vidmod->width : _width,
            isFullScreen ? vidmod->height : _height,
            windowName.c_str(),
            isFullScreen ? monitor : nullptr,
            nullptr);
        if (mode == WINDOW_MODE_WINDOWED) {
            // Center the window on the screen
            glfwSetWindowPos(windowHandle, (vidmod->width - _width) / 2, (vidmod->height - _height) / 2);
        } else if (mode == WINDOW_MODE_WINDOWED_FULLSCREEN) {
            // switch in "window fullscreen" mode
            glfwSetWindowMonitor(windowHandle, monitor, 0, 0,
                                 vidmod->width, vidmod->height, vidmod->refreshRate);
        }
        glfwGetWindowSize(windowHandle, &_width, &_height);

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
        glfwSetFramebufferSizeCallback(windowHandle, glfwFramebufferResizeCallback);
        glfwSetKeyCallback(windowHandle, glfwKeyCallback);
        glfwGetCursorPos(windowHandle, &_mouseLastX, &_mouseLastY);
        glfwSetCursorPosCallback(windowHandle, glfwMouseMoveCallback);
    }

    void WindowHelper::close() {
        glfwSetWindowShouldClose(windowHandle, GL_TRUE);
    }
}