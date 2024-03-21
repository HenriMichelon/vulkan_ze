#pragma once

#include "z0/transform.hpp"
#include "z0/object.hpp"
#include "z0/input.hpp"

namespace z0 {

    class Node: public Object {
    public:
        using id_t = unsigned int;

        explicit Node(const std::string nodeName = "Node");

        virtual void onReady() {}
        virtual void onProcess(float delta) {}
        virtual void onInput(InputEvent& inputEvent) {}

        id_t getId() const { return id; }
        void removeChild(const std::shared_ptr<Node>& child);
        std::list<std::shared_ptr<Node>>& getChildren() { return children; }
        Node* getParent() { return parent; }
        void updateTransform(const glm::mat4& parentMatrix);
        virtual void printTree(std::ostream&, int tab=0);
        std::string toString() const override { return name.empty() ? Object::toString() : name; };
        std::shared_ptr<Node> duplicate();

        virtual void setPosition(glm::vec3 position);
        glm::vec3 getPosition() const { return _position; };
        glm::vec3 getPositionGlobal() const { return glm::vec3(worldTransform[3]); }

        void setRotationDegrees(glm::vec3 orient);
        virtual void setRotation(glm::vec3 orientation);
        virtual void setRotationX(float angle);
        virtual void setRotationY(float angle);
        virtual void setRotationZ(float angle);
        virtual void setRotationGlobalX(float angle);
        virtual void setRotationGlobalY(float angle);
        virtual void setRotationGlobalZ(float angle);
        glm::vec3 getRotation() const { return _orientation; };
        float getRotationX() const { return _orientation.x; }
        float getRotationY() const { return _orientation.y; }
        float getRotationZ() const { return _orientation.z; }

        virtual void rotateX(float angle);
        virtual void rotateY(float angle);
        virtual void rotateZ(float angle);

        virtual void translate(glm::vec3 localOffset);
        virtual void translateGlobal(glm::vec3 globalOffset);

        virtual void setScale(glm::vec3 scale);
        glm::vec3 getScale() const { return _scale; }

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
        glm::vec3 _orientation {};
        std::list<std::shared_ptr<Node>> children;
        virtual std::shared_ptr<Node> duplicateInstance();

    private:
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