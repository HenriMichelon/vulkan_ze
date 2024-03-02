#include "z0/vulkan/main_loop.hpp"
#include "z0/log.hpp"

#include <iostream>
#include <vector>
#include <cstring>

#define ENGINE_NAME "ZeroZero"
#define ENGINE_VERSION VK_MAKE_VERSION(0, 0, 1)

namespace z0 {

    const std::vector<const char*> requestedLayers = {
#ifndef NDEBUG
        "VK_LAYER_KHRONOS_validation"
#endif
    };

    Z0MainLoop::Z0MainLoop(Application& app, int w, int h, const std::string& name): application{app}, window{w, h, name} {
        log(name, "starting");
        createInstance(name);
        device = std::make_unique<Z0Device>(instance, window, requestedLayers);
        window.run(*this);
    }

    Z0MainLoop::~Z0MainLoop() {
        device.reset();
        vkDestroyInstance(instance, nullptr);
        window.close();
    }

    void Z0MainLoop::run() {
    }

    void Z0MainLoop::createInstance(const std::string& name) {
        if (!checkLayerSupport()) {
            die("Some requested layers are not supported");
        }

        VkApplicationInfo applicationInfo = {
            VK_STRUCTURE_TYPE_APPLICATION_INFO,
            nullptr,
            name.c_str(),
            VK_MAKE_VERSION(1, 0, 0),
            ENGINE_NAME,
            ENGINE_VERSION,
            VK_API_VERSION_1_0
        };
        uint32_t extensionCount = 0;
        const char** extensions = nullptr;
#ifdef GLFW_VERSION_MAJOR
        extensions = glfwGetRequiredInstanceExtensions(&extensionCount);
#endif
        VkInstanceCreateInfo createInfo  = {
            VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            nullptr,
            0,
            &applicationInfo,
            static_cast<uint32_t>(requestedLayers.size()),
            requestedLayers.data(),
            extensionCount,
            extensions
        };
        check(vkCreateInstance(&createInfo, nullptr, &instance),
            "Failed to create Vulkan instance");

    }

    bool Z0MainLoop::checkLayerSupport() {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        for (const char* layerName : requestedLayers) {
            bool layerFound = false;
            for (const auto& layerProperties : availableLayers) {
                if (strcmp(layerName, layerProperties.layerName) == 0) {
                    layerFound = true;
                    break;
                }
            }
            if (!layerFound) {
                return false;
            }
        }

        return true;
    }

}