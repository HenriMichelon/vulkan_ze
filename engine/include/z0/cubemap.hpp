#pragma once

#include "z0/vulkan/vulkan_cubemap.hpp"
#include "z0/resource.hpp"

namespace z0 {

    class Cubemap: public Resource {
    public:
        Cubemap(std::string name = ""): Resource(name) {}
        explicit Cubemap(const std::filesystem::path& filename, const std::string& fileext);

        bool isValid() override { return vulkanCubemap != nullptr; }
        uint32_t getWidth() const { return vulkanCubemap->getWidth(); };
        uint32_t getHeight() const { return vulkanCubemap->getHeight(); };
        glm::vec2 getSize() const { return glm::vec2{getWidth(), getHeight()}; };

        void loadFromFile(const std::filesystem::path& filename, const std::string& fileext);

    private:
        std::shared_ptr<VulkanCubemap> vulkanCubemap {nullptr};

    public:
        std::shared_ptr<VulkanCubemap>& _getCubemap() { return vulkanCubemap; }
    };

}