#pragma once

#include "z0/resources/mesh.hpp"

#include <unordered_set>

namespace z0 {

    class MultiMesh: public Resource {
    public:
        explicit MultiMesh(const std::string& meshName): Resource{meshName} {};
        MultiMesh(const std::shared_ptr<Mesh> mesh, uint32_t count, const std::string& meshName);

        bool isValid() override { return getInstanceCount() > 0; }
        void setInstanceCount(uint32_t  count);
        uint32_t getInstanceCount() const;
        void setInstanceTransform(uint32_t instance, glm::mat4 transform);

    private:
        std::shared_ptr<Mesh> mesh;
        std::vector<glm::mat4> transforms{0};
    };

}