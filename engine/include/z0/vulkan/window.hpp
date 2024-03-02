#pragma once

#include "os.hpp"
#include "z0/run_loop.hpp"

#include <string>

namespace z0 {

    class Z0Window {
    public:
        Z0Window(int w, int h, const std::string& windowName);

        void run(RunLoop &mainLoop);
        void close();

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