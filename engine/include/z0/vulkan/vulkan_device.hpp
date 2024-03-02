#pragma once

#include <vulkan/vulkan.hpp>
#include "window_helper.hpp"

namespace z0 {

    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;
        bool isComplete() const {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };
    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    class VulkanDevice {
    public:
        VulkanDevice(WindowHelper &window);
        ~VulkanDevice();

        VkDevice getDevice() { return device; }

        VulkanDevice(const VulkanDevice&) = delete;
        VulkanDevice &operator=(const VulkanDevice&) = delete;
        VulkanDevice(const VulkanDevice&&) = delete;
        VulkanDevice &&operator=(const VulkanDevice&&) = delete;

    private:
        WindowHelper &window;
        VkInstance instance;
        VkDevice device;
        VkPhysicalDevice physicalDevice;
        VkSurfaceKHR surface;
        VkQueue graphicsQueue;
        VkQueue presentQueue;

        static bool checkLayerSupport();
        void createInstance();
        void createDevice();
        int rateDeviceSuitability();
        bool checkDeviceExtensionSupport();
        SwapChainSupportDetails querySwapChainSupport();
        QueueFamilyIndices findQueueFamilies();
    };

}