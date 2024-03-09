#pragma once

#include "z0/vulkan/vulkan_model.hpp"
#include "z0/material.hpp"

namespace z0 {

    struct MeshSurface {
        uint32_t firstVertexIndex;
        uint32_t indexCount;
        std::shared_ptr<Material> material;
        MeshSurface(uint32_t first, uint32_t count): firstVertexIndex{first}, indexCount{count} {};
    };

    class Mesh: public Resource {
    public:
        explicit Mesh(const std::string& meshName): name{meshName} {};

        const std::string& getName() const { return name; }
        std::vector<std::shared_ptr<MeshSurface>>& getSurfaces() { return surfaces; };
        std::shared_ptr<Material>& getSurfaceMaterial(uint32_t surfaceIndex);
        void setSurfaceMaterial(uint32_t surfaceIndex, std::shared_ptr<Material>& material);
        std::vector<Vertex>& getVertices() { return vertices; }
        std::vector<uint32_t>& getIndices() { return indices; }
        bool isValid() override { return _model != nullptr; }

    private:
        std::string name {};
        std::vector<Vertex> vertices{};
        std::vector<uint32_t> indices{};
        std::vector<std::shared_ptr<MeshSurface>> surfaces{};

        std::shared_ptr<VulkanModel> _model;
        std::unordered_set<std::shared_ptr<Material>> _materials{};

    public:
        std::unordered_set<std::shared_ptr<Material>>& _getMaterials() { return _materials; };
        std::shared_ptr<VulkanModel>& _getModel() { return _model; };
        void _buildModel();
    };

}