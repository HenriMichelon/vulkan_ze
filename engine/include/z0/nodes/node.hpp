#pragma once

#include "z0/transform.hpp"
#include "z0/object.hpp"

namespace z0 {

    class Node: public Object {
    public:
        using id_t = unsigned int;

        Node(const std::string nodeName = "");

        virtual void onReady() {}
        virtual void onProcess(float delta) {}

        void addChild(const std::shared_ptr<Node>& node);
        void removeChild(const std::shared_ptr<Node>& node);
        std::list<std::shared_ptr<Node>>& getChildren() { return children; }
        Node* getParent() { return parent; }
        void updateTransform(const glm::mat4& parentMatrix);
        virtual void printTree(std::ostream&, int tab=0);
        std::string toString() const override { return name.empty() ? Object::toString() : name; };

        void rotate(glm::vec3 rotation);
        void rotate_degree(glm::vec3 rotation);
        void scale(glm::vec3 scale);

        bool operator==(const Node& other) const { return id == other.id;}

    //protected:
        glm::vec3 position {};
        glm::mat4 localTransform {};
        glm::mat4 worldTransform {};

    protected:
        std::string name;
        std::list<std::shared_ptr<Node>> children;

    private:
        id_t id;
        Node* parent {nullptr};
        glm::vec3 _rotation {};
        glm::vec3 _scale {1.0f, 1.0f, 1.0f };

        static id_t currentId;

        // Matrix corresponds to Translate * Ry * Rx * Rz * Scale
        // Rotations correspond to Tait-bryan angles of Y(1), X(2), Z(3)
        // https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
        glm::mat4 mat4() const;
        glm::mat3 normalMatrix() const;
    };
}