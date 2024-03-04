// https://vulkan-tutorial.com/Drawing_a_triangle/Setup/Base_code
// https://vulkan-tutorial.com/Drawing_a_triangle/Presentation/Window_surface
#include "z0/vulkan/vulkan_device.hpp"
#include "z0/vulkan/vulkan_model.hpp"
#include "z0/log.hpp"

#include <map>
#include <set>

namespace z0 {

    const std::vector<const char*> requestedLayers = {
#ifndef NDEBUG
        "VK_LAYER_KHRONOS_validation"
#endif
    };

    const std::vector<const char*> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        // https://docs.vulkan.org/samples/latest/samples/extensions/dynamic_rendering/README.html
        VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
        // https://docs.vulkan.org/samples/latest/samples/extensions/shader_object/README.html
        VK_EXT_SHADER_OBJECT_EXTENSION_NAME
    };

#ifdef GLFW_VERSION_MAJOR
    static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
        auto device = reinterpret_cast<VulkanDevice*>(glfwGetWindowUserPointer(window));
        device->framebufferResized = true;
    }
#endif

    VulkanDevice::VulkanDevice(WindowHelper &w): window{w} {
        if (volkInitialize() != VK_SUCCESS) {
            die("Failed to initialize Volk");
        }
        createInstance();
        volkLoadInstance(instance);
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
        if (deviceCount == 0) {
            die("Failed to find GPUs with Vulkan support!");
        }
#ifdef GLFW_VERSION_MAJOR
        if (glfwCreateWindowSurface(instance, window.getWindowHandle(), nullptr, &surface) != VK_SUCCESS) {
            die("Failed to create window surface!");
        }
        glfwSetWindowUserPointer(window.getWindowHandle(), this);
        glfwSetFramebufferSizeCallback(window.getWindowHandle(), framebufferResizeCallback);
#endif
        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
        // Use an ordered map to automatically sort candidates by increasing score
        std::multimap<int, VkPhysicalDevice> candidates;
        for (const auto& device : devices) {
            int score = rateDeviceSuitability(device, surface);
            candidates.insert(std::make_pair(score, device));
        }
        // Check if the best candidate is suitable at all
        if (candidates.rbegin()->first > 0) {
            physicalDevice = candidates.rbegin()->second;
        } else {
            die("Failed to find a suitable GPU!");
        }
        createDevice();
        createSwapChain();
        createImageViews();
        createCommandPool();
    }

    VulkanDevice::~VulkanDevice() {
        cleanupSwapChain();
        vkDestroyCommandPool(device, commandPool, nullptr);
        vkDestroyDevice(device, nullptr);
        vkDestroySurfaceKHR(instance, surface, nullptr);
        vkDestroyInstance(instance, nullptr);
    }

    void VulkanDevice::createDevice() {
        QueueFamilyIndices indices = findQueueFamilies(physicalDevice, surface);

        float queuePriority = 1.0f;
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};
        for (uint32_t queueFamily : uniqueQueueFamilies) {
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }

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

        vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
        vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
    }

    void VulkanDevice::createInstance() {
        if (!checkLayerSupport()) {
            die("Some requested layers are not supported");
        }

        const VkApplicationInfo applicationInfo{
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .apiVersion = VK_API_VERSION_1_3
        };

        uint32_t extensionCount = 0;
        const char** extensions = nullptr;
#ifdef GLFW_VERSION_MAJOR
        extensions = glfwGetRequiredInstanceExtensions(&extensionCount);
#endif
        const VkInstanceCreateInfo createInfo  = {
            VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            nullptr,
            0,
            &applicationInfo,
            static_cast<uint32_t>(requestedLayers.size()),
            requestedLayers.data(),
            extensionCount,
            extensions
        };
        if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
            die("Failed to create Vulkan instance");
        }
    }

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
                .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                              VK_IMAGE_USAGE_TRANSFER_DST_BIT, // TODO Check if VK_IMAGE_USAGE_TRANSFER_DST_BIT is useful
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
    }

    void VulkanDevice::recreateSwapChain() {
        framebufferResized = false;
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
        createImageViews();
    }

    void VulkanDevice::cleanupSwapChain() {
        for (auto & swapChainImageView : swapChainImageViews) {
            vkDestroyImageView(device, swapChainImageView, nullptr);
        }
        vkDestroySwapchainKHR(device, swapChain, nullptr);
    }

    void VulkanDevice::createImageViews() {
        swapChainImageViews.resize(swapChainImages.size());
        for (size_t i = 0; i < swapChainImages.size(); i++) {
            const VkImageViewCreateInfo viewInfo{
                .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                .image = swapChainImages[i],
                .viewType = VK_IMAGE_VIEW_TYPE_2D,
                .format = swapChainImageFormat,
                .components = {
                VK_COMPONENT_SWIZZLE_IDENTITY,
                VK_COMPONENT_SWIZZLE_IDENTITY,
                VK_COMPONENT_SWIZZLE_IDENTITY,
                VK_COMPONENT_SWIZZLE_IDENTITY
                },
                .subresourceRange = {
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .baseMipLevel = 0,
                    .levelCount = 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1
                }
            };
            if (vkCreateImageView(device, &viewInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS) {
                die("failed to create texture image view!");
            }
        }
    }

    void VulkanDevice::createCommandPool() {
        QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice, surface);
        const VkCommandPoolCreateInfo poolInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            .queueFamilyIndex = queueFamilyIndices.graphicsFamily.value(),
        };
        if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
            die("failed to create command pool!");
        }
    }

    bool VulkanDevice::checkLayerSupport() {
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

    VkSurfaceFormatKHR VulkanDevice::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats) {
        for (const auto& availableFormat : availableFormats) {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return availableFormat;
            }
        }
        return availableFormats[0];
    }

    VkPresentModeKHR VulkanDevice::chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes) {
        for (const auto& availablePresentMode : availablePresentModes) {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                return availablePresentMode;
            }
        }
        return VK_PRESENT_MODE_FIFO_KHR;
    }

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

    uint32_t VulkanDevice::findMemoryType(VkPhysicalDevice vkPhysicalDevice, uint32_t typeFilter,
                                          VkMemoryPropertyFlags properties) {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(vkPhysicalDevice, &memProperties);
        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
            if ((typeFilter & (1 << i)) &&
                (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }
        die("failed to find suitable memory type!");
        return 0;
    }

    void VulkanDevice::createBuffer(
            VkDeviceSize size,
            VkBufferUsageFlags usage,
            VkMemoryPropertyFlags properties,
            VkBuffer &buffer,
            VkDeviceMemory &bufferMemory) {
        const VkBufferCreateInfo bufferInfo{
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = size,
            .usage = usage,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE
        };
        if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
            die("failed to create vertex buffer!");
        }
        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(device, buffer, &memRequirements);
        VkMemoryAllocateInfo allocInfo{
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .allocationSize = memRequirements.size,
            .memoryTypeIndex = findMemoryType(physicalDevice, memRequirements.memoryTypeBits, properties)
        };
        if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
            die("failed to allocate vertex buffer memory!");
        }
        vkBindBufferMemory(device, buffer, bufferMemory, 0);
    }

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

    void VulkanDevice::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
        VkCommandBuffer commandBuffer = beginSingleTimeCommands();
        VkBufferCopy copyRegion{};
        copyRegion.size = size;
        vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
        endSingleTimeCommands(commandBuffer);
    }

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

    int VulkanDevice::rateDeviceSuitability(VkPhysicalDevice vkPhysicalDevice, VkSurfaceKHR surface) {
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(vkPhysicalDevice, &deviceProperties);
        VkPhysicalDeviceFeatures deviceFeatures;
        vkGetPhysicalDeviceFeatures(vkPhysicalDevice, &deviceFeatures);

        int score = 0;
        // Discrete GPUs have a significant performance advantage
        if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            score += 1000;
        }
        // Maximum possible size of textures affects graphics quality
        score += deviceProperties.limits.maxImageDimension2D;
        // Application can't function without geometry shaders
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

    VkImageView VulkanDevice::createImageView(VkImage image, VkFormat format) {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        VkImageView imageView;
        if (vkCreateImageView(device, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
            die("failed to create texture image view!");
        }
        return imageView;
    }
}