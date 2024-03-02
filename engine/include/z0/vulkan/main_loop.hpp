#pragma once

#include <memory>
#include "window.hpp"
#include "device.hpp"
#include "z0/application.hpp"

namespace z0 {

    class Z0MainLoop : RunLoop {
    public:
        Z0MainLoop(Application& app, int w, int h, const std::string& name);
        ~Z0MainLoop();

        Z0MainLoop(const Z0MainLoop&) = delete;
        Z0MainLoop &operator=(const Z0MainLoop&) = delete;
        Z0MainLoop(const Z0MainLoop&&) = delete;
        Z0MainLoop &&operator=(const Z0MainLoop&&) = delete;

    private:
        void run() override;
        void createInstance(const std::string& name);
        bool checkLayerSupport();

        Z0Window window;
        std::unique_ptr<Z0Device> device;
        VkInstance instance;
        Application& application;

    };

}