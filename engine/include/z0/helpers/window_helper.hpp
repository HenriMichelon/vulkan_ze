#pragma once

#include "volk.h"
#include "imgui.h"

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#ifdef _WIN64
  #define GLFW_EXPOSE_NATIVE_WIN32
  #include "GLFW/glfw3native.h"
#endif

#include <iostream>
#include <list>



#include "z0/window.hpp"
#include <string>

namespace z0 {

    class WindowHelper {
    public:
        WindowHelper(WindowMode mode, int w, int h, const std::string& windowName);

        bool shouldClose();
        void process();
        void close();

        int getWidth() const { return _width; }
        int getHeight() const { return _height; }

#ifdef GLFW_VERSION_MAJOR
        GLFWwindow* getWindowHandle() const { return windowHandle; }
#endif
    private:
        WindowMode mode;
#ifdef GLFW_VERSION_MAJOR
        GLFWwindow *windowHandle;
#endif

    public:
        // accessed by static functions
        bool _windowResized = false;
        int _width, _height;
        double _mouseLastX, _mouseLastY;
    };

}
