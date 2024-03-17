#pragma once

#include "z0/vulkan/vulkan_instance.hpp"
#include "z0/vulkan/window_helper.hpp"
#include "z0/vulkan/vulkan_renderer.hpp"

#include <memory>
#include <optional>
#include <vector>

namespace z0 {

    const int MAX_FRAMES_IN_FLIGHT = 2;

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
        VulkanDevice(VulkanInstance& instance, WindowHelper &window, bool autoMSAA = false, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT);
        ~VulkanDevice();

        VkDevice getDevice() { return device; }
        VkPhysicalDevice getPhysicalDevice() { return physicalDevice; }
        VkSampleCountFlagBits getSamples() { return samples; }
        const VkExtent2D& getSwapChainExtent() const { return swapChainExtent;}
        VkFormat getSwapChainImageFormat() const { return swapChainImageFormat; }
        VkPhysicalDeviceProperties getDeviceProperties() const { return deviceProperties; }
        float getAspectRatio() const {return static_cast<float>(swapChainExtent.width) / static_cast<float>(swapChainExtent.height);}

        void drawFrame();
        void wait();
        void registerRenderer(const std::shared_ptr<VulkanRenderer>& renderer);

        VkCommandBuffer beginSingleTimeCommands();
        void endSingleTimeCommands(VkCommandBuffer commandBuffer);

        void createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples,
                         VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
                         VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory,
                         VkImageCreateFlags flags = 0, uint32_t layers = 1);
        VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags,
                                    uint32_t mipLevels = 1, VkImageViewType type = VK_IMAGE_VIEW_TYPE_2D);

        void transitionImageLayout(VkImage image,
                                   VkImageLayout oldLayout, VkImageLayout newLayout,
                                   VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask,
                                   VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask,
                                   VkImageAspectFlags aspectMask, uint32_t mipLevels = 1);
        void transitionImageLayout(VkCommandBuffer commandBuffer, VkImage image,
                                   VkImageLayout oldLayout, VkImageLayout newLayout,
                                   VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask,
                                   VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask,
                                   VkImageAspectFlags aspectMask, uint32_t mipLevels = 1);

        static QueueFamilyIndices findQueueFamilies(VkPhysicalDevice vkPhysicalDevice, VkSurfaceKHR surface);
        uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
        // Returns if a given format support LINEAR filtering
        VkBool32 formatIsFilterable(VkFormat format, VkImageTiling tiling);
        // Find a suitable IMAGE_TILING format (for the Depth buffering image)
        VkFormat findImageTilingSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);


    private:
        VulkanInstance& vulkanInstance;
        std::vector<std::shared_ptr<VulkanRenderer>> renderers;

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

        // Drawing a frame
        uint32_t currentFrame = 0;
        std::vector<VkCommandBuffer> commandBuffers;
        std::vector<VkSemaphore> imageAvailableSemaphores;
        std::vector<VkSemaphore> renderFinishedSemaphores;
        std::vector<VkFence> inFlightFences;
        void setInitialState(VkCommandBuffer commandBuffer);

        // Swap chain management
        VkSwapchainKHR swapChain;
        std::vector<VkImage> swapChainImages;
        VkFormat swapChainImageFormat;
        VkExtent2D swapChainExtent;
        std::vector<VkImageView> swapChainImageViews;
        std::shared_ptr<VkSwapchainKHR> oldSwapChain;
        void createSwapChain();
        void cleanupSwapChain();
        void recreateSwapChain();

        // MSAA
        VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;
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

    public:
        VulkanDevice(const VulkanDevice&) = delete;
        VulkanDevice &operator=(const VulkanDevice&) = delete;
        VulkanDevice(const VulkanDevice&&) = delete;
        VulkanDevice &&operator=(const VulkanDevice&&) = delete;
    };

}