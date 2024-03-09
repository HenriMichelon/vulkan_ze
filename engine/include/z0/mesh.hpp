#pragma once

#include "z0/vulkan/vulkan_model.hpp"
#include "z0/material.hpp"

namespace z0 {

    struct MeshSurface {
        uint32_t firstVertexIndex;
        uint32_t indexCount;
        uint32_t materialIndex;

        MeshSurface(uint32_t first, uint32_t count): firstVertexIndex{first}, indexCount{count} {};
    };

    class Mesh: public Resource {
    public:
        Mesh() = default;
        Mesh(const std::filesystem::path& filename);

        std::vector<std::shared_ptr<MeshSurface>>& getSurfaces() { return surfaces; };
        std::vector<std::shared_ptr<StandardMaterial>>& getMaterials() { return materials; }

        std::shared_ptr<StandardMaterial>& getSurfaceMaterial(uint32_t surfaceIndex);
        void setSurfaceMaterial(uint32_t surfaceIndex, std::shared_ptr<StandardMaterial>& material);

        bool isValid() override { return !surfaces.empty(); }
        void loadFromFile(const std::filesystem::path& filepath);

    private:
        std::string name {};
        std::vector<Vertex> vertices{};
        std::vector<uint32_t> indices{};
        std::shared_ptr<VulkanModel> model;
        std::vector<std::shared_ptr<MeshSurface>> surfaces{};
        std::vector<std::shared_ptr<StandardMaterial>> materials{};

    public:
        std::shared_ptr<VulkanModel> _getModel() const { return model; };
    };

}