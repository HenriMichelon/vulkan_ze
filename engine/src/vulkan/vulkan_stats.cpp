#include "z0/vulkan/vulkan_stats.hpp"

#include <iostream>

namespace z0 {

#ifdef VULKAN_STATS

    std::unique_ptr<VulkanStats> VulkanStats::instance = std::make_unique<VulkanStats>();

    void VulkanStats::display() const {
        std::cout << buffersCount << " buffers" << std::endl;
        std::cout << descriptorSetsCount << " descriptor sets" << std::endl;
        std::cout << imagesCount << " images" << std::endl;
        std::cout << averageFps << " avg FPS" << std::endl;
    }
#endif

}