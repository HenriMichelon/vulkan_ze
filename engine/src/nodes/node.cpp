#include "z0/nodes/node.hpp"

namespace z0 {

    Node::id_t Node::currentId = 0;

    Node::Node(): id{currentId++}  {
        localTransform = mat4();
        updateTransform(glm::mat4{1.0f});
    }

    void Node::updateTransform(const glm::mat4& parentMatrix) {
        worldTransform = parentMatrix * localTransform;
        for (auto child : children) {
            child->updateTransform(worldTransform);
        }
    }

    void Node::rotate(glm::vec3 rot) {
        _rotation = rot;
        localTransform = mat4();
        updateTransform(glm::mat4{1.0f});
    }

    void Node::scale(glm::vec3 s) {
        _scale = s;
        localTransform = mat4();
        updateTransform(glm::mat4{1.0f});
    }

    void Node::addChild(const std::shared_ptr<Node>& node) {
        children.push_back(node);
        node->parent = this;
        node->updateTransform(worldTransform);
    }

    void Node::removeChild(const std::shared_ptr<Node>& node) {
        children.remove(node);
        node->parent = nullptr;
    }

    glm::mat4 Node::mat4() const {
        const float c3 = glm::cos(_rotation.z);
        const float s3 = glm::sin(_rotation.z);
        const float c2 = glm::cos(_rotation.x);
        const float s2 = glm::sin(_rotation.x);
        const float c1 = glm::cos(_rotation.y);
        const float s1 = glm::sin(_rotation.y);
        return glm::mat4{
                {
                        _scale.x * (c1 * c3 + s1 * s2 * s3),
                        _scale.x * (c2 * s3),
                        _scale.x * (c1 * s2 * s3 - c3 * s1),
                                                     0.0f,
                },
                {
                        _scale.y * (c3 * s1 * s2 - c1 * s3),
                        _scale.y * (c2 * c3),
                        _scale.y * (c1 * c3 * s2 + s1 * s3),
                                                     0.0f,
                },
                {
                        _scale.z * (c2 * s1),
                        _scale.z * (-s2),
                        _scale.z * (c1 * c2),
                                                     0.0f,
                },
                {position.x, position.y, position.z, 1.0f}};
    };

    glm::mat3 Node::normalMatrix() const {
        const float c3 = glm::cos(_rotation.z);
        const float s3 = glm::sin(_rotation.z);
        const float c2 = glm::cos(_rotation.x);
        const float s2 = glm::sin(_rotation.x);
        const float c1 = glm::cos(_rotation.y);
        const float s1 = glm::sin(_rotation.y);
        const glm::vec3 invScale = 1.0f / _scale;
        return glm::mat3{
                {
                        invScale.x * (c1 * c3 + s1 * s2 * s3),
                        invScale.x * (c2 * s3),
                        invScale.x * (c1 * s2 * s3 - c3 * s1),
                },
                {
                        invScale.y * (c3 * s1 * s2 - c1 * s3),
                        invScale.y * (c2 * c3),
                        invScale.y * (c1 * c3 * s2 + s1 * s3),
                },
                {
                        invScale.z * (c2 * s1),
                        invScale.z * (-s2),
                        invScale.z * (c1 * c2),
                }
        };
    }

}