#pragma once

#include "z0/vulkan/window_helper.hpp"

#include "viewport.hpp"

namespace z0 {

    class Application: public Object {
    public:
        Application(int w, int h, const std::string& name, const std::string appdir);

        const std::string& getApplicationDirectory() const { return appdir; }

        static Application& getApp();

    private:
        Viewport viewport;
        const std::string appdir;
    };

}