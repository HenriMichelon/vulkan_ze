#pragma once

#include "z0/transform.hpp"
#include "z0/object.hpp"
#include "z0/input_event.hpp"

#include <list>

namespace z0 {

    const glm::vec3 AXIS_X { 1.0, 0.0f, 0.0f };
    const glm::vec3 AXIS_Y { 0.0, 1.0f, 0.0f };
    const glm::vec3 AXIS_Z { 0.0, 0.0f, 1.0f };

    class Node: public Object {
    public:
        using id_t = unsigned int;

        explicit Node(const std::string nodeName = "Node");

        virtual void onReady() {}
        virtual void onProcess(float delta) {}
        virtual void onInput(InputEvent& inputEvent) {}

        id_t getId() const { return id; }
        virtual void printTree(std::ostream&, int tab=0);

        std::string toString() const override { return name.empty() ? Object::toString() : name; };
        std::shared_ptr<Node> duplicate();

        // TODO lookAt

        // parent relative position
        virtual void setPosition(glm::vec3 position);
        glm::vec3 getPosition() const { return localTransform[3]; };
        void translate(glm::vec3 localOffset);

        // world relative position
        virtual void setPositionGlobal(glm::vec3 position);
        glm::vec3 getPositionGlobal() const { return worldTransform[3]; }
        void translateGlobal(glm::vec3 globalOffset);

        // rotations around own center
        glm::vec3 getRotation() const;
        float getRotationX() const { return getRotation().x; }
        float getRotationY() const { return getRotation().y; }
        float getRotationZ() const { return getRotation().z; }
        void rotateDegrees(glm::vec3 orient);
        void rotate(glm::vec3 orientation);
        void rotateX(float angle);
        void rotateY(float angle);
        void rotateZ(float angle);
        void setRotationX(float angle);
        void setRotationY(float angle);
        void setRotationZ(float angle);

        // rotations around parent relative position
        glm::vec3 getRotationGlobal() const;
        float getRotationGlobalX() const { return getRotationGlobal().x; }
        float getRotationGlobalY() const { return getRotationGlobal().y; }
        float getRotationGlobalZ() const { return getRotationGlobal().z; }
        //void rotateGlobal(glm::vec3 orientation);
        void rotateGlobalX(float angle);
        void rotateGlobalY(float angle);
        void rotateGlobalZ(float angle);

        virtual void setScale(glm::vec3 scale);
        glm::vec3 getScale() const;

        virtual void setTransform(glm::mat4 transform) { localTransform = transform; }
        glm::mat4& getTransform() { return localTransform; }
        glm::mat4 getTransformGlobal() const { return worldTransform; }
        virtual void updateTransform();
        virtual void updateTransform(const glm::mat4& parentMatrix);

        bool operator==(const Node& other) const { return id == other.id;}

        Node* getParent() { return parent; }

        void addChild(const std::shared_ptr<Node> child);
        template<typename T>
        void addChild(T& node) {
            std::shared_ptr<Node> child = std::make_shared<T>(std::move(node));
            children.push_back(child);
            child->parent = this;
            child->updateTransform(worldTransform);
        }
        void removeChild(const std::shared_ptr<Node>& child);
        std::list<std::shared_ptr<Node>>& getChildren() { return children; }

        const glm::mat3 transformBasis{1, 0, 0, 0, 1, 0, 0, 0, 1};
        const glm::vec3 vec3Zero{0.0};
        const glm::vec2 vec2Zero{0.0};

    protected:
        std::string name;
        glm::mat4 localTransform {};
        glm::mat4 worldTransform {};
        std::list<std::shared_ptr<Node>> children;
        virtual std::shared_ptr<Node> duplicateInstance();

    private:
        id_t id;
        Node* parent {nullptr};
        static id_t currentId;
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