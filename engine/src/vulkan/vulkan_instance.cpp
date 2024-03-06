#include "z0/vulkan/vulkan_instance.hpp"
#include "z0/vulkan/window_helper.hpp" // needed for GLFW support
#include "z0/log.hpp"

#include <cstring>

namespace z0 {

    VulkanInstance::VulkanInstance() {
        if (volkInitialize() != VK_SUCCESS) {
            die("Failed to initialize Volk");
        }
#ifdef GLFW_VERSION_MAJOR
        if (!glfwInit() || !glfwVulkanSupported()) {
            die("Failed to initialize GLFW");
        }
#endif

        // Check if all the requested Vulkan layers are supported by the Vulkan instance
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
                die("A requested Vulkan layer is not supported");
            }
        }

        uint32_t extensionCount = 0;
        const char **extensions = nullptr;
        // Get the Vulkan extensions requested by GLFW
#ifdef GLFW_VERSION_MAJOR
        extensions = glfwGetRequiredInstanceExtensions(&extensionCount);
#endif

        const VkApplicationInfo applicationInfo{
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .apiVersion = VK_API_VERSION_1_3
        };
        const VkInstanceCreateInfo createInfo = {
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
        volkLoadInstance(instance);
    }

    VulkanInstance::~VulkanInstance() {
        vkDestroyInstance(instance, nullptr);
    }

}