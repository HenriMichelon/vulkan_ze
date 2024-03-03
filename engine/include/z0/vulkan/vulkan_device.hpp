#pragma once

#include <volk.h>

#include "z0/vulkan/window_helper.hpp"

#include <memory>
#include <optional>
#include <vector>

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

        [[nodiscard]] VkDevice getDevice() { return device; }
        [[nodiscard]] VkSurfaceKHR getSurface() { return surface; }
        [[nodiscard]] VkSwapchainKHR getSwapChain() { return swapChain; }
        [[nodiscard]] VkPhysicalDevice getPhysicalDevice() { return physicalDevice; }
        [[nodiscard]] VkQueue getGraphicsQueue() { return graphicsQueue; }
        [[nodiscard]] VkQueue getPresentQueue() { return presentQueue; }
        [[nodiscard]] const VkExtent2D& getSwapChainExtent() const { return swapChainExtent;}
        [[nodiscard]] VkFormat getSwapChainImageFormat() const { return swapChainImageFormat; }
        [[nodiscard]] std::vector<VkImageView>& getSwapChainImageViews() { return swapChainImageViews; }
        [[nodiscard]] std::vector<VkImage>& getSwapChainImages() { return swapChainImages; }

        void createBuffer(
                VkDeviceSize size,
                VkBufferUsageFlags usage,
                VkMemoryPropertyFlags properties,
                VkBuffer &buffer,
                VkDeviceMemory &bufferMemory);
        VkCommandBuffer beginSingleTimeCommands();
        void endSingleTimeCommands(VkCommandBuffer commandBuffer);
        void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

        static QueueFamilyIndices findQueueFamilies(VkPhysicalDevice vkPhysicalDevice, VkSurfaceKHR surface);

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
        VkCommandPool commandPool;

        void createInstance();
        void createDevice();
        void createSwapChain();
        void createImageViews();
        void createCommandPool();

        static bool checkLayerSupport();
        static int rateDeviceSuitability(VkPhysicalDevice vkPhysicalDevice, VkSurfaceKHR surface);
        static bool checkDeviceExtensionSupport(VkPhysicalDevice vkPhysicalDevice);
        static SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice vkPhysicalDevice, VkSurfaceKHR surface);
        static VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
        static VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
        static VkExtent2D chooseSwapExtent(WindowHelper& window, const VkSurfaceCapabilitiesKHR& capabilities);
        static uint32_t findMemoryType(VkPhysicalDevice vkPhysicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);

    public:
        VulkanDevice(const VulkanDevice&) = delete;
        VulkanDevice &operator=(const VulkanDevice&) = delete;
        VulkanDevice(const VulkanDevice&&) = delete;
        VulkanDevice &&operator=(const VulkanDevice&&) = delete;
    };

}