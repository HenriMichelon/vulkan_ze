#pragma once

#include <volk.h>
#include <vector>

namespace z0 {
    const std::vector<const char*> requestedLayers = {
#ifndef NDEBUG
        "VK_LAYER_KHRONOS_validation"
#endif
    };

    class VulkanInstance {
    public:
        VulkanInstance();
        ~VulkanInstance();

        VkInstance getInstance() const { return instance; }

    private:
        VkInstance instance;

    public:
        VulkanInstance(const VulkanInstance&) = delete;
        VulkanInstance &operator=(const VulkanInstance&) = delete;
        VulkanInstance(const VulkanInstance&&) = delete;
        VulkanInstance &&operator=(const VulkanInstance&&) = delete;
    };

}