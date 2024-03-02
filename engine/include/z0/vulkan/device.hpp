#pragma once

#include <vulkan/vulkan.hpp>
#include "window.hpp"

namespace z0 {

    class Z0Device {
    public:
        Z0Device(VkInstance instance, Z0Window &window, std::vector<const char*> requestedLayers);
        ~Z0Device();

        VkDevice device() { return _device; }
        VkQueue graphicsQueue() { return _graphicsQueue; }

        Z0Device(const Z0Device&) = delete;
        Z0Device &operator=(const Z0Device&) = delete;
        Z0Device(const Z0Device&&) = delete;
        Z0Device &&operator=(const Z0Device&&) = delete;

    private:
        VkInstance _instance;
        VkSurfaceKHR _surface;
        VkDevice _device;
        VkQueue _graphicsQueue;
        VkQueue _presentQueue;
        void createDevice(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, std::vector<const char*> requestedLayers);
    };

}