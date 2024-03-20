#pragma once

#include "z0/transform.hpp"
#include "z0/object.hpp"

namespace z0 {

    const glm::vec3 VECTOR_AXIS_X = glm::vec3{1.0f, 0.0f, 0.0f};
    const glm::vec3 VECTOR_AXIS_Y = glm::vec3{0.0f, 1.0f, 0.0f};
    const glm::vec3 VECTOR_AXIS_Z = glm::vec3{0.0f, 0.0f, 1.0f};

    class Node: public Object {
    public:
        using id_t = unsigned int;

        explicit Node(const std::string nodeName = "Node");

        virtual void onReady() {}
        virtual void onProcess(float delta) {}

        id_t getId() const { return id; }
        void removeChild(const std::shared_ptr<Node>& child);
        std::list<std::shared_ptr<Node>>& getChildren() { return children; }
        Node* getParent() { return parent; }
        void updateTransform(const glm::mat4& parentMatrix);
        virtual void printTree(std::ostream&, int tab=0);
        std::string toString() const override { return name.empty() ? Object::toString() : name; };
        std::shared_ptr<Node> duplicate();

        void setPosition(glm::vec3 position);
        glm::vec3 getPosition() const { return _position; };
        glm::vec3 getGlobalPosition() const { return glm::vec3(worldTransform[3]); }

        void setRotation(glm::vec3 orientation);
        void setRotationDegrees(glm::vec3 orient);
        void setRotationX(float angle);
        void setRotationY(float angle);
        void setRotationZ(float angle);
        glm::vec3 getRotation() const { return _orientation; };
        float getRotationX() const { return _orientation.x; }
        float getRotationY() const { return _orientation.y; }
        float getRotationZ() const { return _orientation.z; }

        void setScale(glm::vec3 scale);
        glm::vec3 getScale() const { return _scale; }

        // rotation around the position
        void rotateX(float angle);
        void rotateY(float angle);
        void rotateZ(float angle);

        glm::mat4 getGlobalTransform() const { return worldTransform; }
        glm::mat4 getGlobalNormalTransform() const { return normalWorldTransform; }

        bool operator==(const Node& other) const { return id == other.id;}

        void addChild(const std::shared_ptr<Node> child);
        template<typename T>
        void addChild(T& node) {
            std::shared_ptr<Node> child = std::make_shared<T>(std::move(node));
            children.push_back(child);
            child->parent = this;
            child->updateTransform(worldTransform);
        }

    protected:
        std::string name;
        glm::vec3 _position {};
        std::list<std::shared_ptr<Node>> children;
        virtual std::shared_ptr<Node> duplicateInstance();

    private:
        glm::vec3 _orientation {};
        glm::vec3 _scale {1.0f, 1.0f, 1.0f };
        glm::mat4 normalLocalTransform {};
        glm::mat4 worldTransform {};
        glm::mat4 normalWorldTransform {};

        id_t id;
        Node* parent {nullptr};

        static id_t currentId;
        //void addChildPtr(std::shared_ptr<Node> child);

        // Matrix corresponds to Translate * Ry * Rx * Rz * Scale
        // Rotations correspond to Tait-bryan angles of Y(1), X(2), Z(3)
        // https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
        glm::mat4 mat4() const;
        glm::mat3 normalMatrix() const;
        void decomposeLocalMatrix();

    public:
        glm::mat4 localTransform {};
    };


    class DistanceSortedNode {
    public:
        DistanceSortedNode(Node& node, const Node& origin);

        Node& getNode() const { return node; }

        bool operator<(const DistanceSortedNode& other) const {
            return distance < other.distance;
        }

    private:
        Node& node;
        float distance;
    };
}