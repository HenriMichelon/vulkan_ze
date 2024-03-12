#include "z0/nodes/node.hpp"

namespace z0 {

    Node::id_t Node::currentId = 0;

    Node::Node(const std::string _name): id{currentId++}, name{_name}   {
        localTransform = mat4();
        normalLocalTransform = normalMatrix();
        updateTransform(glm::mat4{1.0f});
    }

    void Node::updateTransform(const glm::mat4& parentMatrix) {
        normalLocalTransform = normalMatrix();
        worldTransform = parentMatrix * localTransform;
        normalWorldTransform = parentMatrix * normalLocalTransform;
        for (const auto& child : children) {
            child->updateTransform(worldTransform);
        }
    }

    void Node::setPosition(glm::vec3 pos) {
        _position = pos;
        localTransform = mat4();
        updateTransform(glm::mat4{1.0f});
    }

    void Node::setRotation(glm::vec3 orient) {
        _orientation = orient;
        _orientation.x += glm::pi<float>();
        localTransform = mat4();
        updateTransform(glm::mat4{1.0f});
    }
    void Node::setRotationX(float angle) {
        _orientation.x = angle;
        localTransform = mat4();
        setRotation(_orientation);
    }

    void Node::setRotationY(float angle) {
        _orientation.y = angle;
        localTransform = mat4();
        setRotation(_orientation);
    }

    void Node::setRotationZ(float angle) {
        _orientation.z = angle;
        localTransform = mat4();
        setRotation(_orientation);
    }

    void Node::setScale(glm::vec3 s) {
        _scale = s;
        localTransform = mat4();
        updateTransform(glm::mat4{1.0f});
    }

    void Node::setRotationDegrees(glm::vec3 orient) {
        setRotation({
                            glm::radians(orient.x),
                            glm::radians(orient.y),
                            glm::radians(orient.z)});
    }

    void Node::rotateX(float angle) {
        localTransform = glm::rotate(glm::mat4{1.0f}, angle, glm::vec3{ 1.0, .0, .0 } ) * mat4();
        updateTransform(glm::mat4{1.0f});
    }

    void Node::rotateY(float angle) {
        localTransform = glm::rotate(glm::mat4{1.0f}, angle, glm::vec3{ .0, 1.0, .0 } ) * mat4();
        updateTransform(glm::mat4{1.0f});
    }

    void Node::rotateZ(float angle) {
        localTransform = glm::rotate(glm::mat4{1.0f}, angle, glm::vec3{ .0, 0.0, 1.0 } ) * mat4();
        updateTransform(glm::mat4{1.0f});
    }

    void Node::addChild(const std::shared_ptr<Node> child) {
        children.push_back(child);
        child->parent = this;
        child->updateTransform(worldTransform);
    }

    /*void Node::addChildPtr(std::shared_ptr<Node> child) {
        children.push_back(child);
        child->parent = this;
        child->updateTransform(worldTransform);
    }*/

    void Node::removeChild(const std::shared_ptr<Node>& node) {
        children.remove(node);
        node->parent = nullptr;
    }

    std::shared_ptr<Node> Node::duplicate() {
        std::shared_ptr<Node> dup = duplicateInstance();
        dup->children.clear();
        for(auto&child : children) {
            dup->addChild(child->duplicate());
        }
        return dup;
    }

    std::shared_ptr<Node> Node::duplicateInstance() {
        return std::make_shared<Node>(*this);
    }

    void Node::decomposeLocalMatrix() {
    }

    glm::mat4 Node::mat4() const {
        const float c3 = glm::cos(_orientation.z);
        const float s3 = glm::sin(_orientation.z);
        const float c2 = glm::cos(_orientation.x);
        const float s2 = glm::sin(_orientation.x);
        const float c1 = glm::cos(_orientation.y);
        const float s1 = glm::sin(_orientation.y);
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
                {_position.x, _position.y, _position.z, 1.0f}};
    };


glm::mat3 Node::normalMatrix() const {
        const float c3 = glm::cos(_orientation.z);
        const float s3 = glm::sin(_orientation.z);
        const float c2 = glm::cos(_orientation.x);
        const float s2 = glm::sin(_orientation.x);
        const float c1 = glm::cos(_orientation.y);
        const float s1 = glm::sin(_orientation.y);
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

    void Node::printTree(std::ostream& out, int tab) {
        for (int i = 0; i < (tab*2); i++) {
            out << " ";
        }
        out << toString() << std::endl;
        for (auto child: children) child->printTree(out, tab+1);
    }

}