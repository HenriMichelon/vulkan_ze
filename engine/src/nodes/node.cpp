#include "z0/nodes/node.hpp"

namespace z0 {

    Node::id_t Node::currentId = 0;

    Node::Node(const std::string _name): id{currentId++}, name{_name}   {
        localTransform = glm::mat4 {1.0};
        updateTransform(glm::mat4{1.0f});
    }

    void Node::updateTransform() {
        auto parentMatrix = parent == nullptr ? glm::mat4{1.0f} : parent->worldTransform;
        worldTransform = parentMatrix * localTransform;
        for (const auto& child : children) {
            child->updateTransform(worldTransform);
        }
    }

    void Node::updateTransform(const glm::mat4& parentMatrix) {
        worldTransform = parentMatrix * localTransform;
        for (const auto& child : children) {
            child->updateTransform(worldTransform);
        }
    }

    void Node::translateGlobal(glm::vec3 globalOffset) {
        setPosition(getPositionGlobal() + globalOffset);
    }

    void Node::translate(glm::vec3 localOffset) {
        glm::quat currentOrientation = glm::toQuat(glm::mat3(localTransform));
        glm::vec3 worldTranslation = currentOrientation * localOffset;
        setPosition(getPosition() + worldTranslation);
    }

    void Node::setPosition(glm::vec3 pos) {
        localTransform[3] = glm::vec4(pos, 1.0f);
        updateTransform(glm::mat4{1.0f});
    }

    void Node::setPositionGlobal(glm::vec3 pos) {
        if (parent == nullptr) {
            setPosition(pos);
            return;
        }
        auto inverseParentTransform = glm::inverse(parent->worldTransform);
        auto newLocalPositionHomogeneous = inverseParentTransform * glm::vec4(pos, 1.0);
        auto newLocalPosition = glm::vec3(newLocalPositionHomogeneous);
        localTransform[3] = glm::vec4(newLocalPosition, 1.0f);
        updateTransform(glm::mat4{1.0f});
    }

    glm::vec3 Node::getRotation() const {
        return glm::eulerAngles(glm::toQuat(glm::mat3(localTransform)));
    };

    glm::vec3 Node::getRotationGlobal() const {
        return glm::eulerAngles(glm::toQuat(glm::mat3(worldTransform)));
    };

    void Node::setRotationX(float angle) {
        rotateX(angle - getRotationX());
    }

    void Node::setRotationY(float angle) {
        rotateX(angle - getRotationY());
    }

    void Node::setRotationZ(float angle) {
        rotateX(angle - getRotationZ());
    }

    void Node::rotate(glm::vec3 orientation) {
        localTransform = glm::rotate(localTransform, orientation.x, AXIS_X);
        localTransform = glm::rotate(localTransform, orientation.y, AXIS_Y);
        localTransform = glm::rotate(localTransform, orientation.z, AXIS_Z);
        updateTransform();
    }

    void Node::rotateX(float angle) {
        localTransform = glm::rotate(localTransform, angle, AXIS_X);
        updateTransform();
    }

    void Node::rotateY(float angle) {
        localTransform = glm::rotate(localTransform, angle, AXIS_Y);
        updateTransform();
    }

    void Node::rotateZ(float angle) {
        localTransform = glm::rotate(localTransform, angle, AXIS_Z);
        updateTransform();
    }

    /*void Node::rotateGlobal(glm::vec3 orient) {
        localTransform = glm::rotate(localTransform, orient.z, glm::vec3(0.0f, 0.0f, 1.0f)); // Rotate around Z axis
        localTransform = glm::rotate(localTransform, orient.y, glm::vec3(0.0f, 1.0f, 0.0f)); // Rotate around Y axis
        localTransform = glm::rotate(localTransform, orient.x, glm::vec3(1.0f, 0.0f, 0.0f)); // Rotate around X axis
        updateTransform(glm::mat4{1.0f});
    }*/

    void Node::rotateGlobalX(float angle) {
        glm::mat4 rotation = glm::rotate(glm::mat4{1.0f}, angle, AXIS_X);
        localTransform = rotation * localTransform;
        updateTransform();
    }

    void Node::rotateGlobalY(float angle) {
        glm::mat4 rotation = glm::rotate(glm::mat4{1.0f}, angle, AXIS_Y);
        localTransform = rotation * localTransform;
        updateTransform();
    }

    void Node::rotateGlobalZ(float angle) {
        glm::mat4 rotation = glm::rotate(glm::mat4{1.0f}, angle, AXIS_Z);
        localTransform = rotation * localTransform;
        updateTransform();
    }

    void Node::setScale(glm::vec3 scale) {
        localTransform = glm::scale(localTransform, scale);
        updateTransform();
    }

    glm::vec3 Node::getScale() const {
        glm::vec3 scale;
        glm::quat rotation;
        glm::vec3 translation;
        glm::vec3 skew;
        glm::vec4 perspective;
        glm::decompose(localTransform, scale, rotation, translation, skew, perspective);
        return scale;
    }

    void Node::rotateDegrees(glm::vec3 orient) {
        rotate({
                            glm::radians(orient.x),
                            glm::radians(orient.y),
                            glm::radians(orient.z)});
    }

    void Node::addChild(const std::shared_ptr<Node> child) {
        children.push_back(child);
        child->parent = this;
        child->updateTransform(worldTransform);
    }

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
        dup->id = currentId++;
        return dup;
    }

    std::shared_ptr<Node> Node::duplicateInstance() {
        return std::make_shared<Node>(*this);
    }

    void Node::printTree(std::ostream& out, int tab) {
        for (int i = 0; i < (tab*2); i++) {
            out << " ";
        }
        out << toString() << std::endl;
        for (auto child: children) child->printTree(out, tab+1);
    }

    DistanceSortedNode::DistanceSortedNode(Node &_node, const Node &origin):
        node(_node), distance(glm::distance(origin.getPositionGlobal(), _node.getPositionGlobal())) {
    }

}