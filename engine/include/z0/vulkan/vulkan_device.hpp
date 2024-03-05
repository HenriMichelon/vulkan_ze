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
        [[nodiscard]] VkCommandPool getCommandPool() { return commandPool; }
        [[nodiscard]] VkFormat getDepthFormat() { return depthFormat; }
        [[nodiscard]] VkImage getDepthImage() { return depthImage; }
        [[nodiscard]] VkImageView getDepthImageView() { return depthImageView; }
        [[nodiscard]] VkImage getColorImage() { return colorImage; }
        [[nodiscard]] VkImageView getColorImageView() { return colorImageView; }
        [[nodiscard]] const VkExtent2D& getSwapChainExtent() const { return swapChainExtent;}
        [[nodiscard]] VkFormat getSwapChainImageFormat() const { return swapChainImageFormat; }
        [[nodiscard]] std::vector<VkImageView>& getSwapChainImageViews() { return swapChainImageViews; }
        [[nodiscard]] std::vector<VkImage>& getSwapChainImages() { return swapChainImages; }

        void recreateSwapChain();
        bool framebufferResized = false;

        void createBuffer(
                VkDeviceSize size,
                VkBufferUsageFlags usage,
                VkMemoryPropertyFlags properties,
                VkBuffer &buffer,
                VkDeviceMemory &bufferMemory);
        VkCommandBuffer beginSingleTimeCommands();
        void endSingleTimeCommands(VkCommandBuffer commandBuffer);
        void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
        void createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples,
                         VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
                         VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory,
                         VkImageCreateFlags flags = 0);
        VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);
        void transitionImageLayout(VkImage image, VkFormat format,
                                   VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels = 1);
        void transitionImageLayout(VkCommandBuffer commandBuffer, VkImage image, VkFormat format,
                                   VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels = 1);

        static QueueFamilyIndices findQueueFamilies(VkPhysicalDevice vkPhysicalDevice, VkSurfaceKHR surface);
        uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
        static bool hasStencilComponent(VkFormat format);

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
        void cleanupSwapChain();
        void createImageViews();
        void createCommandPool();

        // Depth buffering
        VkImage depthImage;
        VkFormat depthFormat;
        VkDeviceMemory depthImageMemory;
        VkImageView depthImageView;
        void createDepthResources();

        // MSAA
        VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_4_BIT;
        VkImage colorImage;
        VkDeviceMemory colorImageMemory;
        VkImageView colorImageView;
        //VkSampleCountFlagBits getMaxUsableMSAASampleCount();
        void createColorResources();

        static bool checkLayerSupport();
        static int rateDeviceSuitability(VkPhysicalDevice vkPhysicalDevice, VkSurfaceKHR surface);
        static bool checkDeviceExtensionSupport(VkPhysicalDevice vkPhysicalDevice);
        static SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice vkPhysicalDevice, VkSurfaceKHR surface);
        static VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
        static VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
        static VkExtent2D chooseSwapExtent(WindowHelper& window, const VkSurfaceCapabilitiesKHR& capabilities);
        VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

    public:
        VulkanDevice(const VulkanDevice&) = delete;
        VulkanDevice &operator=(const VulkanDevice&) = delete;
        VulkanDevice(const VulkanDevice&&) = delete;
        VulkanDevice &&operator=(const VulkanDevice&&) = delete;
    };

}