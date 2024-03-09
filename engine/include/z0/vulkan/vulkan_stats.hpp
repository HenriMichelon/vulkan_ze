#pragma once

#include <cstdint>
#include <memory>

namespace z0 {

#ifdef VULKAN_STATS
    class VulkanStats {
    public:
        uint32_t buffersCount;
        uint32_t descriptorSetsCount;
        uint32_t imagesCount;

        void display() const;

        static VulkanStats& get() { return *instance; }
    private:
        static std::unique_ptr<VulkanStats> instance;
    };
#endif

}