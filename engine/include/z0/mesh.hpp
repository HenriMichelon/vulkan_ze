#pragma once

#include "z0/viewport.hpp"

#include "z0/vulkan/vulkan_model.hpp"
#include "texture.hpp"
#include "z0/material.hpp"

namespace z0 {

    class Mesh: public Resource {
    public:
        Mesh() = default;
        Mesh(std::filesystem::path filename);

        uint32_t getSurfaceCount() const { return 1; };
        std::shared_ptr<StandardMaterial> getSurfaceMaterial(uint32_t surfaceIndex) const { return materials[surfaceIndex]; }
        void setSurfaceMaterial(uint32_t surfaceIndex, const std::shared_ptr<StandardMaterial>& material) { materials[surfaceIndex] = material; }

        bool isValid() override { return vulkanModel != nullptr; }
        void loadFromFile(std::filesystem::path filepath);

    private:
        std::string name;
        std::vector<std::shared_ptr<StandardMaterial>> materials{};
        std::vector<Vertex> vertices{};
        std::vector<uint32_t> indices{};
        std::shared_ptr<VulkanModel> vulkanModel{nullptr};

    public:
        VulkanModel& _getModel() { return *vulkanModel; }
    };

}