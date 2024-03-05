#pragma once

#include "z0/vulkan/vulkan_model.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include <memory>
#include <unordered_map>

namespace z0 {
    struct TransformComponent {
        glm::vec3 translation{};
        glm::vec3 scale{1.0f, 1.0f, 1.0f };
        glm::vec3 rotation{};

        // Matrix corresponds to Translate * Ry * Rx * Rz * Scale
        // Rotations correspond to Tait-bryan angles of Y(1), X(2), Z(3)
        // https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
        glm::mat4 mat4();
        glm::mat3 normalMatrix();
    };

    /*struct PointLightComponent {
        float lightIntensity = 1.0f;
    };*/

    class Node {
    public:
        using id_t = unsigned int;
        using Map = std::unordered_map<id_t, Node>;

        static Node create() {
            static id_t currentId = 0;
            return Node{currentId++};
        }

        /*static Node makePointLight(
                float intensity = 10.0f,
                float radius = 0.1f,
                glm::vec3 color = glm::vec3(1.0f)
                );*/

        id_t getId() { return id; }

        Node(const Node &) = delete;
        Node& operator=(const Node &) = delete;
        Node(Node &&) = default;
        Node &operator=(Node &&) = delete;

        glm::vec3 color{};
        TransformComponent transform{};

        // optional pointer components
        std::shared_ptr<VulkanModel> model{};
        //std::unique_ptr<PointLightComponent> pointLight = nullptr;

    private:
        id_t id;

        explicit Node(id_t objId): id{objId} {};

    };
}