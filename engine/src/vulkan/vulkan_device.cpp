/*
 * Derived from
 * https://vulkan-tutorial.com/Drawing_a_triangle
 */
#include "z0/vulkan/vulkan_model.hpp"
#include "z0/log.hpp"

#include <map>
#include <set>

namespace z0 {

    // Requested device extensions
    const std::vector<const char*> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        // https://docs.vulkan.org/samples/latest/samples/extensions/dynamic_rendering/README.html
        VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
        // https://docs.vulkan.org/samples/latest/samples/extensions/shader_object/README.html
        VK_EXT_SHADER_OBJECT_EXTENSION_NAME,
    };


    VulkanDevice::VulkanDevice(VulkanInstance& _instance, WindowHelper &_window,
                               bool useMSAA, VkSampleCountFlagBits _samples):
        vulkanInstance{_instance}, window{_window}, samples(_samples)
    {
        // Check for at least one supported Vulkan physical device
        // https://vulkan-tutorial.com/Drawing_a_triangle/Setup/Physical_devices_and_queue_families#page_Selecting-a-physical-device
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(vulkanInstance.getInstance(), &deviceCount, nullptr);
        if (deviceCount == 0) {
            die("Failed to find GPUs with Vulkan support!");
        }

        // Get a VkSurface for drawing in the GLFW window, must be done before picking the better physical device
        // since we need the VkSurface for vkGetPhysicalDeviceSurfaceCapabilitiesKHR
        // https://vulkan-tutorial.com/Drawing_a_triangle/Presentation/Window_surface#page_Window-surface-creation
#ifdef GLFW_VERSION_MAJOR
        if (glfwCreateWindowSurface(vulkanInstance.getInstance(), window.getWindowHandle(), nullptr, &surface) != VK_SUCCESS) {
            die("Failed to create window surface!");
        }
#endif

        // Use the better Vulkan physical device found
        // https://vulkan-tutorial.com/Drawing_a_triangle/Setup/Physical_devices_and_queue_families#page_Base-device-suitability-checks
        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(vulkanInstance.getInstance(), &deviceCount, devices.data());
        // Use an ordered map to automatically sort candidates by increasing score
        std::multimap<int, VkPhysicalDevice> candidates;
        for (const auto& _device : devices) {
            int score = rateDeviceSuitability(_device, surface);
            candidates.insert(std::make_pair(score, _device));
        }
        // Check if the best candidate is suitable at all
        if (candidates.rbegin()->first > 0) {
            // Select the better suitable device and get some useful properties
            physicalDevice = candidates.rbegin()->second;
            if (useMSAA && samples == VK_SAMPLE_COUNT_1_BIT) {
                samples = getMaxUsableMSAASampleCount();
            }
            vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
        } else {
            die("Failed to find a suitable GPU!");
        }
        createDevice();
        createSwapChain();
    }

    VulkanDevice::~VulkanDevice() {
        cleanupSwapChain();
        vkDestroyCommandPool(device, commandPool, nullptr);
        vkDestroyDevice(device, nullptr);
        vkDestroySurfaceKHR(vulkanInstance.getInstance(), surface, nullptr);
    }

    void VulkanDevice::createDevice() {
        // Find a graphical command queue and a presentation command queue
        // https://vulkan-tutorial.com/Drawing_a_triangle/Setup/Physical_devices_and_queue_families#page_Queue-families
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        QueueFamilyIndices indices = findQueueFamilies(physicalDevice, surface);
        {
            float queuePriority = 1.0f;
            std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};
            for (uint32_t queueFamily: uniqueQueueFamilies) {
                const VkDeviceQueueCreateInfo queueCreateInfo{
                    .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                    .queueFamilyIndex = queueFamily,
                    .queueCount = 1,
                    .pQueuePriorities = &queuePriority,
                };
                queueCreateInfos.push_back(queueCreateInfo);
            }
        }

        // Initialize device extensions and create a logical device
        // https://vulkan-tutorial.com/Drawing_a_triangle/Setup/Logical_device_and_queues#page_Specifying-used-device-features
        // https://vulkan-tutorial.com/Drawing_a_triangle/Setup/Logical_device_and_queues#page_Creating-the-logical-device
        {
            // https://docs.vulkan.org/samples/latest/samples/extensions/shader_object/README.html
            VkPhysicalDeviceShaderObjectFeaturesEXT deviceShaderObjectFeatures{
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_OBJECT_FEATURES_EXT,
                .shaderObject  = VK_TRUE,
            };
            // https://lesleylai.info/en/vk-khr-dynamic-rendering/
            const VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamicRenderingFeature{
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR,
                .pNext = &deviceShaderObjectFeatures,
                .dynamicRendering = VK_TRUE,
            };
            const VkPhysicalDeviceFeatures deviceFeatures{
                .samplerAnisotropy = VK_TRUE
            };
            VkDeviceCreateInfo createInfo{
                .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
                .pNext = &dynamicRenderingFeature,
                .queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()),
                .pQueueCreateInfos = queueCreateInfos.data(),
                .enabledLayerCount = static_cast<uint32_t>(requestedLayers.size()),
                .ppEnabledLayerNames = requestedLayers.data(),
                .enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size()),
                .ppEnabledExtensionNames = deviceExtensions.data(),
                .pEnabledFeatures = &deviceFeatures,
            };
            if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
                die("Failed to create logical device!");
            }
        }

        // https://vulkan-tutorial.com/Drawing_a_triangle/Setup/Logical_device_and_queues#page_Retrieving-queue-handles
        vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
        // https://vulkan-tutorial.com/Drawing_a_triangle/Presentation/Window_surface#page_Creating-the-presentation-queue
        vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);

        // Create the device command pool
        // https://vulkan-tutorial.com/Drawing_a_triangle/Drawing/Command_buffers#page_Command-pools
        QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice, surface);
        const VkCommandPoolCreateInfo poolInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            .queueFamilyIndex = queueFamilyIndices.graphicsFamily.value(),
        };
        if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
            die("Failed to create the command pool");
        }
    }

    // https://vulkan-tutorial.com/Drawing_a_triangle/Presentation/Swap_chain
    void VulkanDevice::createSwapChain() {
        const SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice, surface);
        const VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
        const VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
        const VkExtent2D extent = chooseSwapExtent(window, swapChainSupport.capabilities);

        uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
        if (swapChainSupport.capabilities.maxImageCount > 0 &&
            imageCount > swapChainSupport.capabilities.maxImageCount) {
            imageCount = swapChainSupport.capabilities.maxImageCount;
        }

        {
            VkSwapchainCreateInfoKHR createInfo = {
                .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
                .surface = surface,
                .minImageCount = imageCount,
                .imageFormat = surfaceFormat.format,
                .imageColorSpace = surfaceFormat.colorSpace,
                .imageExtent = extent,
                .imageArrayLayers = 1,
                // VK_IMAGE_USAGE_TRANSFER_DST_BIT for Blit or Revolve (see presentToSwapChain())
                .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                .preTransform = swapChainSupport.capabilities.currentTransform,
                .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
                .presentMode = presentMode,
                .clipped = VK_TRUE
            };
            const QueueFamilyIndices indices = findQueueFamilies(physicalDevice, surface);
            const uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};
            if (indices.graphicsFamily != indices.presentFamily) {
                createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
                createInfo.queueFamilyIndexCount = 2;
                createInfo.pQueueFamilyIndices = queueFamilyIndices;
            } else {
                createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
                createInfo.queueFamilyIndexCount = 0;      // Optional
                createInfo.pQueueFamilyIndices = nullptr;  // Optional
            }
            createInfo.oldSwapchain = oldSwapChain == nullptr ? VK_NULL_HANDLE : *oldSwapChain;
            if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
                throw std::runtime_error("failed to create swap chain!");
            }
        }

        vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
        swapChainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());
        swapChainImageFormat = surfaceFormat.format;
        swapChainExtent = extent;

        swapChainImageViews.resize(swapChainImages.size());
        for (uint32_t i = 0; i < swapChainImages.size(); i++) {
            swapChainImageViews[i] = createImageView(swapChainImages[i], swapChainImageFormat,
                                                     VK_IMAGE_ASPECT_COLOR_BIT, 1);
        }

    }

    // https://vulkan-tutorial.com/Drawing_a_triangle/Swap_chain_recreation
    void VulkanDevice::recreateSwapChain() {
        window.windowResized = false;
#ifdef GLFW_VERSION_MAJOR
        int width = 0, height = 0;
        glfwGetFramebufferSize(window.getWindowHandle(), &width, &height);
        while (width == 0 || height == 0) {
            glfwGetFramebufferSize(window.getWindowHandle(), &width, &height);
            glfwWaitEvents();
        }
#endif
        vkDeviceWaitIdle(device);
        cleanupSwapChain();
        createSwapChain();
    }

    // https://vulkan-tutorial.com/Drawing_a_triangle/Swap_chain_recreation#page_Recreating-the-swap-chain
    void VulkanDevice::cleanupSwapChain() {
        for (auto & swapChainImageView : swapChainImageViews) {
            vkDestroyImageView(device, swapChainImageView, nullptr);
        }
        vkDestroySwapchainKHR(device, swapChain, nullptr);
    }

    // https://vulkan-tutorial.com/Drawing_a_triangle/Presentation/Swap_chain#page_Choosing-the-right-settings-for-the-swap-chain
    VkSurfaceFormatKHR VulkanDevice::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats) {
        for (const auto& availableFormat : availableFormats) {
            // Using sRGB no-linear color space
            // https://learnopengl.com/Advanced-Lighting/Gamma-Correction
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return availableFormat;
            }
        }
        return availableFormats[0];
    }

    // https://vulkan-tutorial.com/Drawing_a_triangle/Presentation/Swap_chain#page_Presentation-mode
    VkPresentModeKHR VulkanDevice::chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes) {
        for (const auto& availablePresentMode : availablePresentModes) {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                return availablePresentMode;
            }
        }
        return VK_PRESENT_MODE_FIFO_KHR;
    }

    // https://vulkan-tutorial.com/Drawing_a_triangle/Presentation/Swap_chain#page_Swap-extent
    VkExtent2D VulkanDevice::chooseSwapExtent(WindowHelper& window, const VkSurfaceCapabilitiesKHR &capabilities) {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            return capabilities.currentExtent;
        } else {
            VkExtent2D actualExtent = {
            static_cast<uint32_t>(window.getWidth()),
            static_cast<uint32_t>(window.getHeight())
            };;
            actualExtent.width = std::max(
                    capabilities.minImageExtent.width,
                    std::min(capabilities.maxImageExtent.width, actualExtent.width));
            actualExtent.height = std::max(
                    capabilities.minImageExtent.height,
                    std::min(capabilities.maxImageExtent.height, actualExtent.height));

            return actualExtent;
        }
    }

    uint32_t VulkanDevice::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);
        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
            if ((typeFilter & (1 << i)) &&
                (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }
        die("failed to find suitable memory type!");
        return 0;
    }

    // https://vulkan-tutorial.com/Texture_mapping/Images#page_Layout-transitions
    VkCommandBuffer VulkanDevice::beginSingleTimeCommands() {
        const VkCommandBufferAllocateInfo allocInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = commandPool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1
        };
        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

        const VkCommandBufferBeginInfo beginInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
        };
        vkBeginCommandBuffer(commandBuffer, &beginInfo);
        return commandBuffer;
    }

    // https://vulkan-tutorial.com/Texture_mapping/Images#page_Layout-transitions
    void VulkanDevice::endSingleTimeCommands(VkCommandBuffer commandBuffer) {
        vkEndCommandBuffer(commandBuffer);
        const VkSubmitInfo submitInfo{
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .commandBufferCount = 1,
            .pCommandBuffers = &commandBuffer
        };
        vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(graphicsQueue);
        vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
    }

    // https://vulkan-tutorial.com/Drawing_a_triangle/Setup/Physical_devices_and_queue_families#page_Queue-families
    QueueFamilyIndices  VulkanDevice::findQueueFamilies(VkPhysicalDevice vkPhysicalDevice, VkSurfaceKHR surface) {
        QueueFamilyIndices indices;
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice, &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice, &queueFamilyCount, queueFamilies.data());
        int i = 0;
        for (const auto& queueFamily : queueFamilies) {
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                indices.graphicsFamily = i;
            }
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(vkPhysicalDevice, i, surface, &presentSupport);
            if (presentSupport) {
                indices.presentFamily = i;
            }
            if (indices.isComplete()) {
                break;
            }
            i++;
        }
        return indices;
    }

    // https://vulkan-tutorial.com/Drawing_a_triangle/Presentation/Swap_chain#page_Querying-details-of-swap-chain-support
    SwapChainSupportDetails VulkanDevice::querySwapChainSupport(VkPhysicalDevice vkPhysicalDevice, VkSurfaceKHR surface) {
        SwapChainSupportDetails details;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vkPhysicalDevice, surface, &details.capabilities);
        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(vkPhysicalDevice, surface, &formatCount, nullptr);
        if (formatCount != 0) {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(vkPhysicalDevice, surface, &formatCount, details.formats.data());
        }
        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(vkPhysicalDevice, surface, &presentModeCount, nullptr);
        if (presentModeCount != 0) {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(vkPhysicalDevice, surface, &presentModeCount, details.presentModes.data());
        }
        return details;
    }

    // https://vulkan-tutorial.com/Drawing_a_triangle/Presentation/Swap_chain#page_Checking-for-swap-chain-support
    bool VulkanDevice::checkDeviceExtensionSupport(VkPhysicalDevice vkPhysicalDevice) {
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(vkPhysicalDevice, nullptr, &extensionCount, nullptr);
        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(vkPhysicalDevice, nullptr, &extensionCount, availableExtensions.data());
        std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());
        for (const auto& extension : availableExtensions) {
            requiredExtensions.erase(extension.extensionName);
        }
        return requiredExtensions.empty();
    }

    // https://vulkan-tutorial.com/Drawing_a_triangle/Setup/Physical_devices_and_queue_families#page_Base-device-suitability-checks
    int VulkanDevice::rateDeviceSuitability(VkPhysicalDevice vkPhysicalDevice, VkSurfaceKHR surface) {
        VkPhysicalDeviceProperties _deviceProperties;
        vkGetPhysicalDeviceProperties(vkPhysicalDevice, &_deviceProperties);
        VkPhysicalDeviceFeatures deviceFeatures;
        vkGetPhysicalDeviceFeatures(vkPhysicalDevice, &deviceFeatures);

        int score = 0;
        // Discrete GPUs have a significant performance advantage
        if (_deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            score += 1000;
        }
        // Maximum possible size of textures affects graphics quality
        score += _deviceProperties.limits.maxImageDimension2D;
        // MainLoop can't function without geometry shaders
        if (!deviceFeatures.geometryShader) {
            return 0;
        }
        bool extensionsSupported = checkDeviceExtensionSupport(vkPhysicalDevice);
        bool swapChainAdequate = false;
        if (extensionsSupported) {
            SwapChainSupportDetails swapChainSupport = querySwapChainSupport(vkPhysicalDevice, surface);
            swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        }
        QueueFamilyIndices indices = findQueueFamilies(vkPhysicalDevice, surface);
        if ((!extensionsSupported) || (!indices.isComplete()) || (!swapChainAdequate)) {
            return 0;
        }
        return score;
    }

    // https://vulkan-tutorial.com/Depth_buffering#page_Depth-image-and-view
    VkFormat VulkanDevice::findImageTilingSupportedFormat(const std::vector<VkFormat> &candidates, VkImageTiling tiling,
                                                          VkFormatFeatureFlags features) {
        for (VkFormat format : candidates) {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);
            if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
                return format;
            } else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
                return format;
            }
        }
        die("failed to find supported format!");
        return candidates.at(0);
    }

    // https://vulkan-tutorial.com/Multisampling#page_Getting-available-sample-count
    VkSampleCountFlagBits VulkanDevice::getMaxUsableMSAASampleCount() {
        VkPhysicalDeviceProperties physicalDeviceProperties;
        vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);
        VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;
        if (counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
        if (counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
        if (counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
        if (counts & VK_SAMPLE_COUNT_8_BIT) { return VK_SAMPLE_COUNT_8_BIT; }
        if (counts & VK_SAMPLE_COUNT_4_BIT) { return VK_SAMPLE_COUNT_4_BIT; }
        if (counts & VK_SAMPLE_COUNT_2_BIT) { return VK_SAMPLE_COUNT_2_BIT; }
        return VK_SAMPLE_COUNT_1_BIT;
    }

    // https://vulkan-tutorial.com/Drawing_a_triangle/Presentation/Image_views
    VkImageView VulkanDevice::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels) {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = format;
        viewInfo.subresourceRange.aspectMask = aspectFlags;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = mipLevels;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        VkImageView imageView;
        if (vkCreateImageView(device, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
            die("failed to create texture image view!");
        }
        return imageView;
    }

    // https://vulkan-tutorial.com/Texture_mapping/Images#page_Texture-Image
    void VulkanDevice::createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples,
                                   VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
                                   VkMemoryPropertyFlags properties, VkImage &image,
                                   VkDeviceMemory &imageMemory, VkImageCreateFlags flags) {
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = width;
        imageInfo.extent.height = height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = mipLevels;
        imageInfo.arrayLayers = 1;
        imageInfo.format = format;
        imageInfo.tiling = tiling;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = usage;
        imageInfo.samples = numSamples;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.flags = flags;

        if (vkCreateImage(device, &imageInfo, nullptr, &image) != VK_SUCCESS) {
            throw std::runtime_error("failed to create image!");
        }

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(device, image, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
            die("failed to allocate image memory!");
        }

        vkBindImageMemory(device, image, imageMemory, 0);
    }

    // https://vulkan-tutorial.com/Depth_buffering#page_Depth-image-and-view
    bool VulkanDevice::hasStencilComponent(VkFormat format) {
        return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
    }

    // https://vulkan-tutorial.com/Texture_mapping/Images#page_Layout-transitions
    void VulkanDevice::transitionImageLayout(VkImage image, VkFormat format,
                                             VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels) {
        VkCommandBuffer commandBuffer = beginSingleTimeCommands();
        transitionImageLayout(commandBuffer, image, format, oldLayout, newLayout, mipLevels);
        endSingleTimeCommands(commandBuffer);
    }

    // https://vulkan-tutorial.com/Texture_mapping/Images#page_Layout-transitions
    // https://vulkan-tutorial.com/Generating_Mipmaps#page_Generating-Mipmaps
    void VulkanDevice::transitionImageLayout(VkCommandBuffer commandBuffer, VkImage image, VkFormat format,
                                             VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels) {
        VkImageMemoryBarrier imageMemoryBarrier{
                .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                .srcAccessMask = 0,
                .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                .oldLayout = oldLayout,
                .newLayout = newLayout,
                .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .image = image,
                .subresourceRange = {
                        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                        .baseMipLevel = 0,
                        .levelCount = mipLevels,
                        .baseArrayLayer = 0,
                        .layerCount = 1,
                },
        };

        VkPipelineStageFlags sourceStage;
        VkPipelineStageFlags destinationStage;

        // https://vulkan-tutorial.com/Depth_buffering#page_Explicitly-transitioning-the-depth-image
        if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
            imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
            if (hasStencilComponent(format)) {
                imageMemoryBarrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
            }
        } else if (newLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL) {
                imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
                if (hasStencilComponent(format)) {
                    imageMemoryBarrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
                }
        } else {
            imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        }

        if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
            imageMemoryBarrier.srcAccessMask = 0;
            imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        } else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) {
            imageMemoryBarrier.srcAccessMask = 0;
            imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        } else if (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) {
            imageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            imageMemoryBarrier.dstAccessMask = 0;
            sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            destinationStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) {
            imageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            imageMemoryBarrier.dstAccessMask = 0;
            sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            destinationStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        } else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
            imageMemoryBarrier.srcAccessMask = 0;
            imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_UNDEFINED) {
            imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            imageMemoryBarrier.dstAccessMask = 0;
            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        } else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL) {
            imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
            imageMemoryBarrier.srcAccessMask = 0;
            imageMemoryBarrier.dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT;
            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        } else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
            imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;// | VK_IMAGE_ASPECT_STENCIL_BIT;
            imageMemoryBarrier.srcAccessMask = 0;
            imageMemoryBarrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT; // | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        } else {
            die("unsupported layout transition!");
        }

        vkCmdPipelineBarrier(
                commandBuffer,
                sourceStage,
                destinationStage,
                0,
                0,
                nullptr,
                0,
                nullptr,
                1, // imageMemoryBarrierCount
                &imageMemoryBarrier // pImageMemoryBarriers
        );
    }

}