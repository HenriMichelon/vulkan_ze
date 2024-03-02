#pragma once

#include <vulkan/vulkan.hpp>
#include <memory>
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
        explicit VulkanDevice(WindowHelper &window);
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
        VkSwapchainKHR swapChain;
        std::vector<VkImage> swapChainImages;
        VkFormat swapChainImageFormat;
        VkExtent2D swapChainExtent;
        std::vector<VkImageView> swapChainImageViews;
        std::shared_ptr<VkSwapchainKHR> oldSwapChain;

        void createInstance();
        void createDevice();
        void createSwapChain();
        void createImageViews();

        static bool checkLayerSupport();
        static int rateDeviceSuitability(VkPhysicalDevice vkPhysicalDevice, VkSurfaceKHR surface);
        static bool checkDeviceExtensionSupport(VkPhysicalDevice vkPhysicalDevice);
        static SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice vkPhysicalDevice, VkSurfaceKHR surface);
        static QueueFamilyIndices findQueueFamilies(VkPhysicalDevice vkPhysicalDevice, VkSurfaceKHR surface);
        static VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
        static VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
        static VkExtent2D chooseSwapExtent(WindowHelper& window, const VkSurfaceCapabilitiesKHR& capabilities);
    };

}