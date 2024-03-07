#pragma once

#include "z0/vulkan/vulkan_instance.hpp"
#include "z0/vulkan/window_helper.hpp"

#include <memory>
#include <optional>
#include <vector>

namespace z0 {

    // used for findQueueFamilies()
    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;
        bool isComplete() const {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };

    // used for querySwapChainSupport()
    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    class VulkanDevice {
    public:
        // if useMSAA == true && samples == VK_SAMPLE_COUNT_1_BIT then use the maximum sample count
        VulkanDevice(VulkanInstance& instance, WindowHelper &window, bool useMSAA = false, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT);
        ~VulkanDevice();

        VkDevice getDevice() { return device; }
        VkPhysicalDevice getPhysicalDevice() { return physicalDevice; }
        VkSwapchainKHR getSwapChain() { return swapChain; }
        VkQueue getGraphicsQueue() { return graphicsQueue; }
        VkQueue getPresentQueue() { return presentQueue; }
        VkCommandPool getCommandPool() { return commandPool; }
        VkFormat getDepthFormat() { return depthFormat; }
        VkImage getDepthImage() { return depthImage; }
        VkImageView getDepthImageView() { return depthImageView; }
        VkImage getColorImage() { return colorImage; }
        VkImageView getColorImageView() { return colorImageView; }
        VkSampleCountFlagBits getSamples() { return samples; }
        const VkExtent2D& getSwapChainExtent() const { return swapChainExtent;}
        float getSwapChainAspectRatio() const;
        VkFormat getSwapChainImageFormat() const { return swapChainImageFormat; }
        std::vector<VkImage>& getSwapChainImages() { return swapChainImages; }
        VkPhysicalDeviceProperties getDeviceProperties() const { return deviceProperties; }
        WindowHelper& getWindowHelper() { return window; }

        // Recreation of the swap chain in case of window resizing/minimizing
        void recreateSwapChain();
        // Since we render in a memory image we need to manually present the image in the swap chain
        void presentToSwapChain(VkCommandBuffer commandBuffer, uint32_t imageIndex);

        VkCommandBuffer beginSingleTimeCommands();
        void endSingleTimeCommands(VkCommandBuffer commandBuffer);

        // Image management helpers
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
        VulkanInstance& vulkanInstance;

        // Physical & logical device management
        WindowHelper &window;
        VkDevice device;
        VkPhysicalDevice physicalDevice;
        VkSurfaceKHR surface;
        VkQueue graphicsQueue;
        VkQueue presentQueue;
        VkCommandPool commandPool;
        VkPhysicalDeviceProperties deviceProperties;
        void createDevice();

        // Swap chain management
        VkSwapchainKHR swapChain;
        std::vector<VkImage> swapChainImages;
        VkFormat swapChainImageFormat;
        VkExtent2D swapChainExtent;
        std::vector<VkImageView> swapChainImageViews;
        std::shared_ptr<VkSwapchainKHR> oldSwapChain;
        void createSwapChain();
        void cleanupSwapChain();

        // Depth buffering
        VkImage depthImage;
        VkFormat depthFormat;
        VkDeviceMemory depthImageMemory;
        VkImageView depthImageView;
        void createDepthResources();

        // MSAA
        VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;
        VkImage colorImage;
        VkDeviceMemory colorImageMemory;
        VkImageView colorImageView;
        VkImageBlit colorImageBlit{};
        VkImageResolve colorImageResolve{};
        void createColorResources();
        VkSampleCountFlagBits getMaxUsableMSAASampleCount();

        // Check if all the requested Vulkan extensions are supported by a device
        static bool checkDeviceExtensionSupport(VkPhysicalDevice vkPhysicalDevice);
        // Rate physical device by properties
        static int rateDeviceSuitability(VkPhysicalDevice vkPhysicalDevice, VkSurfaceKHR surface);
        // Get the swap chain capabilities
        static SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice vkPhysicalDevice, VkSurfaceKHR surface);
        // Get the swap chain format, default for sRGB/NON-LINEAR
        static VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
        // Get the swap chain present mode, default to MAILBOX, if not avaible FIFO (V-SYNC)
        static VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
        // Get the swap chain images sizes
        static VkExtent2D chooseSwapExtent(WindowHelper& window, const VkSurfaceCapabilitiesKHR& capabilities);
        // Find a suitable IMAGE_TILING format (for the Depth buffering image)
        VkFormat findImageTilingSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

    public:
        VulkanDevice(const VulkanDevice&) = delete;
        VulkanDevice &operator=(const VulkanDevice&) = delete;
        VulkanDevice(const VulkanDevice&&) = delete;
        VulkanDevice &&operator=(const VulkanDevice&&) = delete;
    };

}